#include "../../src/core/appconfig.h"
#include "../../src/core/foodmanager.h"
#include "../../src/core/meallogmanager.h"
#include "../../src/core/schedulemanager.h"
#include "../../src/data/databasemanager.h"
#include "../../src/recommendation/recommendationengine.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QHash>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQueue>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QVariantList>

#include <iostream>
#include <functional>

namespace
{
struct ScheduleSeed {
    int weekday;
    int periodStart;
    int periodEnd;
    const char *courseName;
    const char *location;
    const char *campusZone;
    const char *intensity;
    const char *notes;
};

struct MerchantSeed {
    const char *name;
    const char *campusArea;
    const char *priceLevel;
    bool dineIn;
    bool takeaway;
    bool delivery;
    int deliveryEta;
    int distance;
    int queueTime;
};

struct DishSeed {
    const char *name;
    const char *merchantName;
    const char *category;
    double price;
    const char *diningMode;
    int eatTime;
    int effort;
    const char *carb;
    const char *fat;
    const char *protein;
    const char *satiety;
    const char *burden;
    const char *sleepiness;
    bool combo;
};

struct FeedbackSeed {
    int fullness;
    int sleepiness;
    int comfort;
    int focus;
    int taste;
    int repeat;
    bool wouldEatAgain;
    const char *text;
};

struct MealSeed {
    const char *scenarioKey;
    const char *recommendationNow;
    const char *mealType;
    const char *eatenAt;
    int weekday;
    bool hasClassAfterMeal;
    int minutesUntilNextClass;
    const char *locationType;
    const char *diningMode;
    double totalPrice;
    int totalEatTimeMinutes;
    int hunger;
    int energy;
    const char *mood;
    const char *notes;
    const char *dishName;
    FeedbackSeed feedback;
};

struct ValidationCase {
    QString name;
    bool ok;
    QString detail;
};

void addResult(QList<ValidationCase> *results, const QString &name, bool ok,
               const QString &detail)
{
    results->append({name, ok, detail});
}

QVariantMap findByName(const QVariantList &items, const QString &name)
{
    for (const QVariant &itemVariant : items) {
        const QVariantMap item = itemVariant.toMap();
        if (item.value(QStringLiteral("name")).toString() == name) {
            return item;
        }
    }
    return {};
}

QVariantMap findMealByTimestamp(const QVariantList &meals, const QString &eatenAt)
{
    for (const QVariant &mealVariant : meals) {
        const QVariantMap meal = mealVariant.toMap();
        if (meal.value(QStringLiteral("eatenAt")).toString() == eatenAt) {
            return meal;
        }
    }
    return {};
}

QString joinCandidateNames(const QVariantList &candidates)
{
    QStringList names;
    for (const QVariant &candidateVariant : candidates) {
        const QString dishName =
            candidateVariant.toMap().value(QStringLiteral("dishName")).toString();
        if (!dishName.isEmpty()) {
            names.append(dishName);
        }
    }
    return names.join(QStringLiteral(" | "));
}

QString firstName(const QVariantList &items)
{
    if (items.isEmpty()) {
        return {};
    }

    const QVariantMap firstItem = items.first().toMap();
    const QString dishName =
        firstItem.value(QStringLiteral("dishName")).toString().trimmed();
    if (!dishName.isEmpty()) {
        return dishName;
    }

    return firstItem.value(QStringLiteral("name")).toString().trimmed();
}

bool top3Contains(const QVariantList &candidates, const QStringList &dishNames)
{
    for (const QVariant &candidateVariant : candidates) {
        if (dishNames.contains(
                candidateVariant.toMap().value(QStringLiteral("dishName")).toString())) {
            return true;
        }
    }
    return false;
}

bool top3Excludes(const QVariantList &candidates, const QStringList &dishNames)
{
    for (const QVariant &candidateVariant : candidates) {
        if (dishNames.contains(
                candidateVariant.toMap().value(QStringLiteral("dishName")).toString())) {
            return false;
        }
    }
    return true;
}

bool execSql(const QString &connectionName, const QString &sql,
             const QVariantList &bindValues = {})
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(sql);
    for (const QVariant &value : bindValues) {
        query.addBindValue(value);
    }
    return query.exec();
}

int countRows(const QString &connectionName, const QString &table)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM %1").arg(table));
    return query.exec() && query.next() ? query.value(0).toInt() : -1;
}

bool querySingleInt(const QString &connectionName, const QString &sql,
                    const QVariantList &bindValues, int *value)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(sql);
    for (const QVariant &bindValue : bindValues) {
        query.addBindValue(bindValue);
    }
    if (!query.exec() || !query.next()) {
        return false;
    }
    *value = query.value(0).toInt();
    return true;
}

QString insightNames(const QVariantList &insights)
{
    QStringList names;
    for (const QVariant &insightVariant : insights) {
        const QString key =
            insightVariant.toMap().value(QStringLiteral("key")).toString();
        if (!key.isEmpty()) {
            names.append(key);
        }
    }
    return names.join(QStringLiteral(", "));
}

QVariantList runScenario(RecommendationEngine *engine, const QString &overrideNow)
{
    qputenv("MEALADVISOR_FIXED_NOW", overrideNow.toUtf8());
    engine->runDecision();
    qunsetenv("MEALADVISOR_FIXED_NOW");
    return engine->candidates();
}

bool upsertMerchant(FoodManager *foodManager, const MerchantSeed &seed,
                    int *merchantId, QString *error)
{
    const QString name = QString::fromUtf8(seed.name);
    foodManager->reload();
    const QVariantMap existing = findByName(foodManager->merchants(), name);
    const bool ok = existing.isEmpty()
                        ? foodManager->addMerchant(
                              name, QString::fromUtf8(seed.campusArea),
                              QString::fromUtf8(seed.priceLevel), seed.dineIn,
                              seed.takeaway, seed.delivery, seed.deliveryEta,
                              seed.distance, seed.queueTime, QString())
                        : foodManager->updateMerchant(
                              existing.value(QStringLiteral("id")).toInt(), name,
                              QString::fromUtf8(seed.campusArea),
                              QString::fromUtf8(seed.priceLevel), seed.dineIn,
                              seed.takeaway, seed.delivery, seed.deliveryEta,
                              seed.distance, seed.queueTime, QString());
    if (!ok) {
        *error = foodManager->lastError();
        return false;
    }
    foodManager->reload();
    *merchantId = findByName(foodManager->merchants(), name)
                      .value(QStringLiteral("id"))
                      .toInt();
    return *merchantId > 0;
}

bool upsertDish(FoodManager *foodManager, const DishSeed &seed, int merchantId,
                int *dishId, QString *error)
{
    const QString name = QString::fromUtf8(seed.name);
    foodManager->reload();
    const QVariantMap existing = findByName(foodManager->dishes(), name);
    const bool ok = existing.isEmpty()
                        ? foodManager->addDish(
                              name, merchantId, QString::fromUtf8(seed.category),
                              seed.price, QString::fromUtf8(seed.diningMode),
                              seed.eatTime, seed.effort, QString::fromUtf8(seed.carb),
                              QString::fromUtf8(seed.fat),
                              QString::fromUtf8(seed.protein), QStringLiteral("medium"),
                              QStringLiteral("medium"),
                              QString::fromUtf8(seed.satiety),
                              QString::fromUtf8(seed.burden),
                              QString::fromUtf8(seed.sleepiness),
                              QStringLiteral("high"), QStringLiteral("low"), seed.combo,
                              false, 1.0, QString())
                        : foodManager->updateDish(
                              existing.value(QStringLiteral("id")).toInt(), name,
                              merchantId, QString::fromUtf8(seed.category), seed.price,
                              QString::fromUtf8(seed.diningMode), seed.eatTime,
                              seed.effort, QString::fromUtf8(seed.carb),
                              QString::fromUtf8(seed.fat),
                              QString::fromUtf8(seed.protein), QStringLiteral("medium"),
                              QStringLiteral("medium"),
                              QString::fromUtf8(seed.satiety),
                              QString::fromUtf8(seed.burden),
                              QString::fromUtf8(seed.sleepiness),
                              QStringLiteral("high"), QStringLiteral("low"), seed.combo,
                              false, 1.0, QString());
    if (!ok) {
        *error = foodManager->lastError();
        return false;
    }
    foodManager->reload();
    *dishId =
        findByName(foodManager->dishes(), name).value(QStringLiteral("id")).toInt();
    return *dishId > 0;
}

void cleanupScenarioData(const QString &connectionName)
{
    const QStringList mealTimes = {
        QStringLiteral("2026-04-21T12:03:00"),
        QStringLiteral("2026-04-21T12:10:00"),
        QStringLiteral("2026-04-21T18:02:00"),
        QStringLiteral("2026-04-22T12:28:00"),
        QStringLiteral("2026-04-22T12:30:00"),
        QStringLiteral("2026-04-22T12:38:00"),
        QStringLiteral("2026-04-22T17:50:00"),
        QStringLiteral("2026-04-22T17:55:00"),
        QStringLiteral("2026-04-23T12:08:00"),
        QStringLiteral("2026-04-23T18:05:00"),
        QStringLiteral("2026-04-24T12:32:00")
    };
    for (const QString &eatenAt : mealTimes) {
        execSql(connectionName,
                QStringLiteral("DELETE FROM meal_feedback WHERE meal_log_id IN "
                               "(SELECT id FROM meal_logs WHERE eaten_at = ?)"),
                {eatenAt});
        execSql(connectionName,
                QStringLiteral("DELETE FROM meal_log_dishes WHERE meal_log_id IN "
                               "(SELECT id FROM meal_logs WHERE eaten_at = ?)"),
                {eatenAt});
    }
    execSql(connectionName, QStringLiteral("DELETE FROM recommendation_records"));
    for (const QString &eatenAt : mealTimes) {
        execSql(connectionName,
                QStringLiteral("DELETE FROM meal_logs WHERE eaten_at = ?"),
                {eatenAt});
    }
    execSql(connectionName, QStringLiteral("DELETE FROM dishes"));
    execSql(connectionName, QStringLiteral("DELETE FROM merchants"));
    execSql(connectionName,
            QStringLiteral("DELETE FROM schedule_entries WHERE weekday BETWEEN 2 AND 5"));
}

bool saveMeal(MealLogManager *mealLogManager, int dishId, const MealSeed &meal,
              QString *error)
{
    mealLogManager->clearSelection();
    if (!mealLogManager->addSelectedDish(dishId, 1.0, QString())) {
        *error = mealLogManager->lastError();
        return false;
    }
    const bool ok = mealLogManager->saveMealLog(
        QString::fromUtf8(meal.mealType), QString::fromUtf8(meal.eatenAt),
        meal.weekday, meal.hasClassAfterMeal, meal.minutesUntilNextClass,
        QString::fromUtf8(meal.locationType), QString::fromUtf8(meal.diningMode),
        meal.totalPrice, meal.totalEatTimeMinutes, meal.hunger, meal.energy,
        QString::fromUtf8(meal.mood), QString::fromUtf8(meal.notes));
    if (!ok) {
        *error = mealLogManager->lastError();
    }
    return ok;
}

bool saveFeedbackForMeal(MealLogManager *mealLogManager, const QString &connectionName,
                         const QString &eatenAt, const FeedbackSeed &feedback,
                         QString *error)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(QStringLiteral(
        "SELECT id FROM meal_logs WHERE eaten_at = ? ORDER BY id DESC LIMIT 1"));
    query.addBindValue(eatenAt);
    const int mealId = (query.exec() && query.next()) ? query.value(0).toInt() : 0;
    if (mealId <= 0) {
        *error = QStringLiteral("Meal not found for feedback: %1").arg(eatenAt);
        return false;
    }
    const bool ok = mealLogManager->saveMealFeedback(
        mealId, feedback.fullness, feedback.sleepiness, feedback.comfort,
        feedback.focus, feedback.wouldEatAgain, feedback.taste, feedback.repeat,
        QString::fromUtf8(feedback.text));
    if (!ok) {
        *error = mealLogManager->lastError();
    }
    return ok;
}

void appendLog(const QString &text)
{
    QFile file(QStringLiteral("validation-output.txt"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << text << '\n';
}

struct MockHttpResponse {
    int statusCode = 200;
    QByteArray body;
    QByteArray contentType = "application/json";
    int delayMs = 0;
};

class MockOpenAiServer final : public QTcpServer
{
public:
    bool start(QString *error = nullptr)
    {
        if (listen(QHostAddress::LocalHost, 0)) {
            return true;
        }

        if (error != nullptr) {
            *error = errorString();
        }
        return false;
    }

    QString chatCompletionsUrl() const
    {
        return QStringLiteral("http://127.0.0.1:%1/v1/chat/completions")
            .arg(serverPort());
    }

    void enqueueResponse(const MockHttpResponse &response)
    {
        m_responses.enqueue(response);
    }

    void clearLastRequestBody()
    {
        m_lastRequestBody.clear();
    }

    QByteArray lastRequestBody() const
    {
        return m_lastRequestBody;
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override
    {
        QTcpSocket *socket = new QTcpSocket(this);
        if (!socket->setSocketDescriptor(socketDescriptor)) {
            socket->deleteLater();
            return;
        }

        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            if (socket->property("mealadvisorHandled").toBool()) {
                socket->readAll();
                return;
            }

            QByteArray &buffer = m_requestBuffers[socket];
            buffer += socket->readAll();
            maybeHandleRequest(socket);
        });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_requestBuffers.remove(socket);
            socket->deleteLater();
        });
    }

private:
    void maybeHandleRequest(QTcpSocket *socket)
    {
        QByteArray &buffer = m_requestBuffers[socket];
        const int headerEnd = buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            return;
        }

        const QByteArray headerBytes = buffer.left(headerEnd);
        int contentLength = 0;
        const QList<QByteArray> headerLines = headerBytes.split('\n');
        for (const QByteArray &rawLine : headerLines) {
            const QByteArray line = rawLine.trimmed();
            if (line.left(sizeof("Content-Length:") - 1)
                    .compare("Content-Length:", Qt::CaseInsensitive) == 0) {
                contentLength = line.mid(sizeof("Content-Length:") - 1).trimmed().toInt();
                break;
            }
        }

        const int bodyStart = headerEnd + 4;
        if (buffer.size() - bodyStart < contentLength) {
            return;
        }

        socket->setProperty("mealadvisorHandled", true);
        m_lastRequestBody = buffer.mid(bodyStart, contentLength);
        const MockHttpResponse response = m_responses.isEmpty()
                                              ? MockHttpResponse{500,
                                                                 QByteArray("{\"error\":\"missing mock response\"}"),
                                                                 "application/json",
                                                                 0}
                                              : m_responses.dequeue();
        m_requestBuffers.remove(socket);

        QTimer::singleShot(response.delayMs, socket, [socket, response]() {
            if (socket->state() == QAbstractSocket::UnconnectedState) {
                return;
            }

            QByteArray statusText = "OK";
            if (response.statusCode == 401) {
                statusText = "Unauthorized";
            } else if (response.statusCode >= 400) {
                statusText = "Error";
            }

            const QByteArray reply =
                "HTTP/1.1 " + QByteArray::number(response.statusCode) + " " + statusText +
                "\r\nContent-Type: " + response.contentType +
                "\r\nContent-Length: " + QByteArray::number(response.body.size()) +
                "\r\nConnection: close\r\n\r\n" + response.body;
            socket->write(reply);
            socket->flush();
            socket->disconnectFromHost();
        });
    }

    QQueue<MockHttpResponse> m_responses;
    QHash<QTcpSocket *, QByteArray> m_requestBuffers;
    QByteArray m_lastRequestBody;
};

bool waitForCondition(const std::function<bool()> &predicate, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        if (predicate()) {
            return true;
        }
        QThread::msleep(10);
    }

    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    return predicate();
}

struct ParseScenarioResult {
    bool completed = false;
    QString state;
    QString status;
    QString summary;
    bool fallbackActive = false;
    QByteArray requestBody;
};

ParseScenarioResult runParseScenario(RecommendationEngine *engine,
                                     MockOpenAiServer *server,
                                     const QString &text,
                                     const MockHttpResponse &response,
                                     int timeoutMs = 18000)
{
    server->clearLastRequestBody();
    server->enqueueResponse(response);

    engine->clearSupplement();
    engine->parseSupplement(text);

    ParseScenarioResult result;
    result.completed =
        waitForCondition([engine]() { return !engine->busy(); }, timeoutMs);
    result.state = engine->supplementState();
    result.status = engine->supplementStatus();
    result.summary = engine->supplementSummary();
    result.fallbackActive = engine->supplementFallbackActive();
    result.requestBody = server->lastRequestBody();
    return result;
}

QJsonObject parseJsonObject(const QByteArray &payload)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    return document.object();
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("MealAdvisor"));
    QCoreApplication::setApplicationName(QStringLiteral("MealAdvisor"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QDir().mkpath(QStringLiteral("validation-settings"));
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       QDir::current().absoluteFilePath(QStringLiteral("validation-settings")));

    const QStringList args = app.arguments();
    if (args.size() >= 3 && args.at(1) == QStringLiteral("--evaluate-response-file")) {
        QFile responseFile(args.at(2));
        if (!responseFile.open(QIODevice::ReadOnly)) {
            std::cerr << "Failed to read response file: "
                      << qPrintable(args.at(2)) << "\n";
            return 1;
        }

        const QString sourceText = args.size() >= 4 ? args.at(3) : QString();
        const auto outcome = RecommendationEngine::evaluateSupplementResponse(
            sourceText, responseFile.readAll());

        QJsonObject resultObject{
            {QStringLiteral("accepted"), outcome.accepted},
            {QStringLiteral("state"), outcome.state},
            {QStringLiteral("status"), outcome.status},
            {QStringLiteral("fallbackUsed"), outcome.fallbackUsed},
            {QStringLiteral("result"),
             QJsonObject{
                 {QStringLiteral("hungerIntent"), outcome.adjustment.hungerIntent},
                 {QStringLiteral("carbIntent"), outcome.adjustment.carbIntent},
                 {QStringLiteral("drinkIntent"), outcome.adjustment.drinkIntent},
                 {QStringLiteral("budgetFlexIntent"),
                  outcome.adjustment.budgetFlexIntent},
                 {QStringLiteral("classConstraintWeight"),
                  outcome.adjustment.classConstraintWeight},
                 {QStringLiteral("postMealSleepPlan"),
                  outcome.adjustment.postMealSleepPlan},
                 {QStringLiteral("plannedNapMinutes"),
                  outcome.adjustment.plannedNapMinutes},
                 {QStringLiteral("sleepNeedLevel"),
                  outcome.adjustment.sleepNeedLevel},
                 {QStringLiteral("sleepPlanConfidence"),
                  outcome.adjustment.sleepPlanConfidence},
                 {QStringLiteral("proteinIntent"), outcome.adjustment.proteinIntent},
                 {QStringLiteral("colaIntent"), outcome.adjustment.colaIntent},
                 {QStringLiteral("flavorIntent"), outcome.adjustment.flavorIntent},
                 {QStringLiteral("relaxedTimePreference"),
                  outcome.adjustment.relaxedTimePreference}}}
        };
        std::cout << qPrintable(QString::fromUtf8(
                         QJsonDocument(resultObject).toJson(QJsonDocument::Compact)))
                  << "\n";
        return outcome.accepted ? 0 : 2;
    }

    QFile::remove(QStringLiteral("validation-output.txt"));
    appendLog(QStringLiteral("Validation runner started"));

    DatabaseManager databaseManager;
    if (!databaseManager.initialize()) {
        appendLog(QStringLiteral("Database init failed: %1")
                      .arg(databaseManager.lastError()));
        std::cerr << "Database initialization failed: "
                  << qPrintable(databaseManager.lastError()) << "\n";
        return 1;
    }
    appendLog(QStringLiteral("Database init ok: %1")
                  .arg(databaseManager.databasePath()));

    ScheduleManager scheduleManager(databaseManager);
    FoodManager foodManager(databaseManager);
    MealLogManager mealLogManager(databaseManager);
    AppSettings appSettings;
    RecommendationEngine recommendationEngine(databaseManager, &appSettings);

    QList<ValidationCase> results;
    QStringList importedMerchants;
    QStringList importedDishes;
    QStringList importedMeals;

    const QList<ScheduleSeed> schedules = {
        {2, 1, 4, "高等数学", "A1-203", "主教学楼", "high", "11:40 下课"},
        {2, 7, 8, "数据结构", "B2-301", "信息楼", "medium", "14:15 上课"},
        {3, 1, 5, "大学英语", "C1-404", "外语楼", "medium", "12:25 下课"},
        {3, 11, 12, "数据库原理", "实验楼305", "实验楼", "high", "19:00 晚课"},
        {4, 1, 4, "概率统计", "A3-201", "主教学楼", "high", "11:40 下课"},
        {4, 9, 10, "软件工程", "B1-205", "信息楼", "medium", "16:00-17:25 连堂"},
        {5, 1, 5, "操作系统", "A2-208", "主教学楼", "high", "12:25 下课"},
        {5, 7, 7, "人工智能导论", "B3-102", "信息楼", "medium", "14:15 上课"}
    };

    const QList<MerchantSeed> merchants = {
        {"麦当劳（北门店）", "校外北门", "mid", true, true, true, 28, 12, 8},
        {"肯德基（南门店）", "校外南门", "mid", true, true, true, 30, 10, 10},
        {"味千拉面（商业街店）", "校外商业街", "mid", true, true, false, 0, 9, 6},
        {"潮汕牛肉火锅（商业街二楼）", "校外商业街", "high", true, false, false, 0, 15, 18},
        {"第一食堂炒饭窗口", "校内一食堂", "budget", true, true, false, 0, 4, 7},
        {"第二食堂拉面窗口", "校内二食堂", "budget", true, true, false, 0, 6, 5}
    };

    const QList<DishSeed> dishes = {
        {"麦辣鸡腿堡套餐", "麦当劳（北门店）", "套餐", 39, "delivery", 20, 2, "high", "medium", "medium", "medium", "medium", "medium", true},
        {"双层吉士牛堡套餐", "麦当劳（北门店）", "套餐", 42, "delivery", 22, 2, "high", "high", "medium", "high", "high", "high", true},
        {"吮指原味鸡腿堡套餐", "肯德基（南门店）", "套餐", 36, "dine_in", 24, 2, "high", "high", "medium", "high", "medium", "medium", true},
        {"老北京鸡肉卷套餐", "肯德基（南门店）", "套餐", 34, "delivery", 18, 2, "medium", "medium", "medium", "medium", "low", "low", true},
        {"味千豚骨叉烧拉面", "味千拉面（商业街店）", "拉面", 32, "dine_in", 26, 2, "high", "medium", "medium", "high", "medium", "medium", false},
        {"牛肉火锅单人套餐", "潮汕牛肉火锅（商业街二楼）", "火锅", 78, "dine_in", 55, 3, "low", "medium", "high", "high", "medium", "low", true},
        {"招牌火腿鸡蛋炒饭", "第一食堂炒饭窗口", "炒饭", 16, "dine_in", 14, 1, "high", "medium", "low", "medium", "medium", "high", false},
        {"牛肉炒饭", "第一食堂炒饭窗口", "炒饭", 18, "takeaway", 13, 1, "high", "medium", "medium", "high", "medium", "medium", false},
        {"清汤牛肉拉面", "第二食堂拉面窗口", "拉面", 18, "dine_in", 16, 1, "medium", "low", "medium", "medium", "low", "low", false},
        {"番茄鸡蛋拉面", "第二食堂拉面窗口", "拉面", 15, "dine_in", 15, 1, "medium", "low", "low", "medium", "low", "low", false}
    };

    const QList<MealSeed> meals = {
        {"tue_lunch_1140_1415_a", "2026-04-21T12:02:00", "lunch", "2026-04-21T12:03:00", 2, true, 132, "campus", "dine_in", 16, 14, 4, 3, "赶时间", "周二课间快吃", "招牌火腿鸡蛋炒饭", {3, 5, 2, 2, 3, 2, false, "吃完有点困，下午上课前不算稳"}},
        {"tue_lunch_1140_1415_b", "2026-04-21T12:09:00", "lunch", "2026-04-21T12:10:00", 2, true, 125, "campus", "takeaway", 18, 13, 4, 3, "赶时间", "周二打包去上课", "牛肉炒饭", {4, 4, 3, 2, 3, 2, false, "能吃饱，但还是偏困"}},
        {"tue_dinner_relaxed", "2026-04-21T18:01:00", "dinner", "2026-04-21T18:02:00", 2, false, 0, "off_campus", "delivery", 42, 22, 4, 3, "想吃重口", "周二晚上没课", "双层吉士牛堡套餐", {4, 3, 2, 2, 5, 4, true, "重口满足，但吃完负担偏高"}},
        {"wed_lunch_1225_relaxed_a", "2026-04-22T12:27:00", "lunch", "2026-04-22T12:28:00", 3, false, 25, "campus", "delivery", 34, 18, 3, 3, "想吃轻一点", "周三下午没课", "老北京鸡肉卷套餐", {3, 1, 4, 4, 4, 4, true, "这份更轻也更稳"}},
        {"wed_lunch_1225_relaxed_b", "2026-04-22T12:37:00", "lunch", "2026-04-22T12:38:00", 3, false, 0, "off_campus", "dine_in", 32, 26, 3, 3, "想吃面", "周三午后松一点", "味千豚骨叉烧拉面", {4, 3, 3, 3, 3, 3, true, "口味还行，属于中等偏好"}},
        {"wed_dinner_1900_a", "2026-04-22T17:49:00", "dinner", "2026-04-22T17:50:00", 3, true, 70, "campus", "dine_in", 15, 15, 4, 3, "晚课前求稳", "周三晚课前", "番茄鸡蛋拉面", {3, 1, 5, 5, 4, 5, true, "晚课前最稳的一次"}},
        {"wed_dinner_1900_b", "2026-04-22T17:54:00", "dinner", "2026-04-22T17:55:00", 3, true, 65, "dorm", "delivery", 39, 20, 4, 3, "想快点解决", "周三晚课前外卖", "麦辣鸡腿堡套餐", {4, 3, 3, 3, 3, 3, true, "到得快，但稳定性一般"}},
        {"thu_lunch_1140_1600", "2026-04-23T12:07:00", "lunch", "2026-04-23T12:08:00", 4, true, 232, "off_campus", "dine_in", 36, 24, 4, 3, "下午还有连堂", "周四午餐", "吮指原味鸡腿堡套餐", {4, 3, 3, 3, 3, 3, true, "一般偏可接受，不算特别轻"}},
        {"thu_dinner_high_budget", "2026-04-23T18:04:00", "dinner", "2026-04-23T18:05:00", 4, false, 0, "off_campus", "dine_in", 78, 55, 4, 3, "想吃好一点", "周四晚上没课", "牛肉火锅单人套餐", {5, 1, 5, 4, 5, 5, true, "高预算堂食里这顿最满意"}},
        {"fri_lunch_1225_1415", "2026-04-24T12:31:00", "lunch", "2026-04-24T12:32:00", 5, true, 103, "campus", "dine_in", 18, 16, 4, 3, "求稳", "周五午课前", "清汤牛肉拉面", {4, 1, 5, 5, 4, 5, true, "稳，课前吃很合适"}}
    };

    cleanupScenarioData(databaseManager.connectionName());
    appendLog(QStringLiteral("Scenario cleanup done"));

    for (const ScheduleSeed &schedule : schedules) {
        if (!scheduleManager.addCustomEntry(schedule.weekday, schedule.periodStart,
                                            schedule.periodEnd, QString::fromUtf8(schedule.courseName),
                                            QString::fromUtf8(schedule.location),
                                            QString::fromUtf8(schedule.campusZone),
                                            QString::fromUtf8(schedule.intensity),
                                            QString::fromUtf8(schedule.notes))) {
            std::cerr << "Failed to seed schedule: "
                      << qPrintable(scheduleManager.lastError()) << "\n";
            return 1;
        }
    }
    appendLog(QStringLiteral("Schedule seeded"));

    QHash<QString, int> merchantIds;
    for (const MerchantSeed &merchant : merchants) {
        int merchantId = 0;
        QString error;
        if (!upsertMerchant(&foodManager, merchant, &merchantId, &error)) {
            appendLog(QStringLiteral("Merchant seed failed: %1 | %2")
                          .arg(QString::fromUtf8(merchant.name), error));
            std::cerr << "Failed to seed merchant " << merchant.name << ": "
                      << qPrintable(error) << "\n";
            return 1;
        }
        merchantIds.insert(QString::fromUtf8(merchant.name), merchantId);
        importedMerchants.append(QString::fromUtf8(merchant.name));
    }
    appendLog(QStringLiteral("Merchants seeded"));

    QHash<QString, int> dishIds;
    for (const DishSeed &dish : dishes) {
        int dishId = 0;
        QString error;
        if (!upsertDish(&foodManager, dish,
                        merchantIds.value(QString::fromUtf8(dish.merchantName)),
                        &dishId, &error)) {
            appendLog(QStringLiteral("Dish seed failed: %1 | %2")
                          .arg(QString::fromUtf8(dish.name), error));
            std::cerr << "Failed to seed dish " << dish.name << ": "
                      << qPrintable(error) << "\n";
            return 1;
        }
        dishIds.insert(QString::fromUtf8(dish.name), dishId);
        importedDishes.append(QString::fromUtf8(dish.name));
    }
    appendLog(QStringLiteral("Dishes seeded"));
    mealLogManager.reload();

    for (const MealSeed &meal : meals) {
        runScenario(&recommendationEngine, QString::fromUtf8(meal.recommendationNow));
        QString error;
        if (!saveMeal(&mealLogManager, dishIds.value(QString::fromUtf8(meal.dishName)),
                      meal, &error)) {
            appendLog(QStringLiteral("Meal seed failed: %1 | %2")
                          .arg(QString::fromUtf8(meal.eatenAt), error));
            std::cerr << "Failed to seed meal " << meal.eatenAt << ": "
                      << qPrintable(error) << "\n";
            return 1;
        }
        importedMeals.append(
            QStringLiteral("%1 | %2")
                .arg(QString::fromUtf8(meal.eatenAt), QString::fromUtf8(meal.dishName)));
        if (!saveFeedbackForMeal(&mealLogManager, databaseManager.connectionName(),
                                 QString::fromUtf8(meal.eatenAt), meal.feedback,
                                 &error)) {
            appendLog(QStringLiteral("Feedback seed failed: %1 | %2")
                          .arg(QString::fromUtf8(meal.eatenAt), error));
            std::cerr << "Failed to seed feedback for " << meal.eatenAt << ": "
                      << qPrintable(error) << "\n";
            return 1;
        }
    }
    appendLog(QStringLiteral("Meals and feedback seeded"));

    foodManager.reload();
    mealLogManager.reload();
    recommendationEngine.reload();

    mealLogManager.setDishSearch(QStringLiteral("番茄"));
    const QVariantList tomatoMatches = mealLogManager.filteredAvailableDishes();
    addResult(&results, QStringLiteral("Meals search ranks exact keyword first"),
              !tomatoMatches.isEmpty() &&
                  firstName(tomatoMatches) == QStringLiteral("番茄鸡蛋拉面"),
              QStringLiteral("搜索“番茄”首项：%1").arg(firstName(tomatoMatches)));

    mealLogManager.clearSelection();
    const bool addFirstOk =
        !tomatoMatches.isEmpty() &&
        mealLogManager.addSelectedDish(
            tomatoMatches.first().toMap().value(QStringLiteral("id")).toInt(), 1.0,
            QStringLiteral("enter-add smoke"));
    addResult(&results, QStringLiteral("Enter-to-add first result stays usable"),
              addFirstOk && mealLogManager.selectedDishes().size() == 1,
              QStringLiteral("首项加入后已选数量：%1")
                  .arg(mealLogManager.selectedDishes().size()));
    mealLogManager.clearSelection();
    mealLogManager.setDishSearch(QString());

    mealLogManager.clearSelection();
    mealLogManager.addSelectedDish(dishIds.value(QStringLiteral("番茄鸡蛋拉面")), 1.0,
                                   QString());
    const bool invalidDatetimeBlocked = !mealLogManager.saveMealLog(
        QStringLiteral("lunch"), QStringLiteral("2026/04/22 12:30"), 3, false, 0,
        QStringLiteral("campus"), QStringLiteral("dine_in"), 15.0, 15, 3, 3,
        QStringLiteral("测试"), QStringLiteral("invalid datetime"));
    addResult(&results, QStringLiteral("Meal guardrail blocks invalid datetime"),
              invalidDatetimeBlocked &&
                  mealLogManager.lastError().contains(QStringLiteral("有效的用餐时间")),
              mealLogManager.lastError());

    mealLogManager.clearSelection();
    mealLogManager.addSelectedDish(dishIds.value(QStringLiteral("番茄鸡蛋拉面")), 1.0,
                                   QString());
    const bool invalidMinutesBlocked = !mealLogManager.saveMealLog(
        QStringLiteral("lunch"), QStringLiteral("2026-04-22T12:30:00"), 3, true, 0,
        QStringLiteral("campus"), QStringLiteral("dine_in"), 15.0, 15, 3, 3,
        QStringLiteral("测试"), QStringLiteral("invalid minutes"));
    addResult(&results,
              QStringLiteral("Meal guardrail blocks class-after-meal with nonpositive minutes"),
              invalidMinutesBlocked &&
                  mealLogManager.lastError().contains(QStringLiteral("必须大于 0")),
              mealLogManager.lastError());
    mealLogManager.clearSelection();

    int normalizedMinutes = -1;
    const bool normalizedMinutesOk = querySingleInt(
        databaseManager.connectionName(),
        QStringLiteral("SELECT minutes_until_next_class FROM meal_logs WHERE eaten_at = ?"),
        {QStringLiteral("2026-04-22T12:28:00")}, &normalizedMinutes);
    addResult(&results, QStringLiteral("No-class meal saves normalize minutes back to 0"),
              normalizedMinutesOk && normalizedMinutes == 0,
              QStringLiteral("2026-04-22T12:28:00 保存后 minutes=%1")
                  .arg(normalizedMinutes));

    const bool invalidMerchantRejected =
        !foodManager.addMerchant(QStringLiteral("坏商家"), QStringLiteral("测试"),
                                 QStringLiteral("luxury"), true, false, false, 0, 1,
                                 1, QString());
    addResult(&results, QStringLiteral("Merchant validation rejects invalid price level"),
              invalidMerchantRejected &&
                  foodManager.lastError().contains(QStringLiteral("budget、mid 或 high")),
              foodManager.lastError());

    const bool invalidDishRejected = !foodManager.addDish(
        QStringLiteral("坏菜"), merchantIds.value(QStringLiteral("第一食堂炒饭窗口")),
        QStringLiteral("测试"), -1.0, QStringLiteral("delivery"), 10, 1,
        QStringLiteral("medium"), QStringLiteral("medium"), QStringLiteral("medium"),
        QStringLiteral("medium"), QStringLiteral("medium"), QStringLiteral("medium"),
        QStringLiteral("medium"), QStringLiteral("medium"),
        QStringLiteral("medium"), QStringLiteral("medium"), false, false, 1.0,
        QString());
    addResult(&results, QStringLiteral("Dish validation rejects invalid numeric range"),
              invalidDishRejected &&
                  foodManager.lastError().contains(QStringLiteral("超出支持范围")),
              foodManager.lastError());

    foodManager.setDishSearch(QStringLiteral("拉面"));
    addResult(&results, QStringLiteral("Food search returns ramen group deterministically"),
              foodManager.filteredDishes().size() >= 3,
              QStringLiteral("搜索“拉面”结果：%1")
                  .arg(joinCandidateNames(foodManager.filteredDishes())));
    foodManager.setDishSearch(QString());

    const QVariantList tueLunch =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-21T12:02:00"));
    addResult(&results, QStringLiteral("11:40下课 + 14:15上课 场景能压住重餐"),
              top3Contains(tueLunch, {QStringLiteral("清汤牛肉拉面"),
                                      QStringLiteral("番茄鸡蛋拉面"),
                                      QStringLiteral("老北京鸡肉卷套餐")}) &&
                  top3Excludes(tueLunch, {QStringLiteral("牛肉火锅单人套餐"),
                                          QStringLiteral("双层吉士牛堡套餐")}),
              QStringLiteral("top3: %1").arg(joinCandidateNames(tueLunch)));

    const QVariantList wedLunch =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-22T12:27:00"));
    addResult(&results, QStringLiteral("12:25下课 + 下午没课 午餐更偏均衡而非硬压速度"),
              top3Contains(wedLunch, {QStringLiteral("老北京鸡肉卷套餐"),
                                      QStringLiteral("味千豚骨叉烧拉面"),
                                      QStringLiteral("吮指原味鸡腿堡套餐")}),
              QStringLiteral("top3: %1").arg(joinCandidateNames(wedLunch)));

    const QVariantList thuLunch =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-23T12:07:00"));
    addResult(&results, QStringLiteral("11:40下课 + 16:00-17:25连堂 午餐不应按120分钟内有课处理"),
              top3Contains(thuLunch, {QStringLiteral("吮指原味鸡腿堡套餐"),
                                      QStringLiteral("味千豚骨叉烧拉面"),
                                      QStringLiteral("老北京鸡肉卷套餐")}),
              QStringLiteral("top3: %1").arg(joinCandidateNames(thuLunch)));

    const QVariantList wedDinner =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-22T17:49:00"));
    addResult(&results, QStringLiteral("19:00晚课前晚餐 更应偏向稳和快"),
              top3Contains(wedDinner, {QStringLiteral("番茄鸡蛋拉面"),
                                       QStringLiteral("清汤牛肉拉面"),
                                       QStringLiteral("麦辣鸡腿堡套餐"),
                                       QStringLiteral("老北京鸡肉卷套餐")}) &&
                  top3Excludes(wedDinner, {QStringLiteral("牛肉火锅单人套餐")}),
              QStringLiteral("top3: %1").arg(joinCandidateNames(wedDinner)));

    const QVariantList thuDinner =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-23T18:04:00"));
    addResult(&results, QStringLiteral("晚上没课时的高预算堂食 可把火锅抬进前列"),
              top3Contains(thuDinner, {QStringLiteral("牛肉火锅单人套餐")}),
              QStringLiteral("top3: %1").arg(joinCandidateNames(thuDinner)));

    const QVariantList tieRunA =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-23T12:07:00"));
    const QVariantList tieRunB =
        runScenario(&recommendationEngine, QStringLiteral("2026-04-23T12:07:00"));
    addResult(&results, QStringLiteral("Recommendation tie-break stays stable across identical reruns"),
              joinCandidateNames(tieRunA) == joinCandidateNames(tieRunB),
              QStringLiteral("runA: %1 || runB: %2")
                  .arg(joinCandidateNames(tieRunA), joinCandidateNames(tieRunB)));

    const int recommendationRecordCount =
        countRows(databaseManager.connectionName(), QStringLiteral("recommendation_records"));
    addResult(&results, QStringLiteral("Recommendation runs persisted to recommendation_records"),
              recommendationRecordCount >= meals.size(),
              QStringLiteral("recommendation_records=%1").arg(recommendationRecordCount));

    int linkedMealCount = 0;
    querySingleInt(databaseManager.connectionName(),
                   QStringLiteral("SELECT COUNT(*) FROM recommendation_records WHERE selected_meal_log_id IS NOT NULL"),
                   {}, &linkedMealCount);
    int linkedMismatchCount = 0;
    querySingleInt(databaseManager.connectionName(),
                   QStringLiteral(
                       "SELECT COUNT(*) "
                       "FROM recommendation_records rr "
                       "LEFT JOIN meal_feedback mf ON mf.meal_log_id = rr.selected_meal_log_id "
                       "WHERE rr.selected_meal_log_id IS NOT NULL "
                       "AND (mf.recommendation_record_id IS NULL OR mf.recommendation_record_id != rr.id)"),
                   {}, &linkedMismatchCount);
    addResult(&results, QStringLiteral("Meal save links back to recommendation_records"),
              linkedMealCount >= 3 && linkedMismatchCount == 0,
              QStringLiteral("linked recommendation selections=%1, mismatches=%2")
                  .arg(linkedMealCount)
                  .arg(linkedMismatchCount));

    int feedbackLinkedCount = 0;
    querySingleInt(databaseManager.connectionName(),
                   QStringLiteral("SELECT COUNT(*) FROM meal_feedback WHERE recommendation_record_id IS NOT NULL"),
                   {}, &feedbackLinkedCount);
    addResult(&results, QStringLiteral("Feedback keeps recommendation linkage"),
              feedbackLinkedCount == linkedMealCount && feedbackLinkedCount >= 3,
              QStringLiteral("feedback linked to recommendation=%1, selected meals=%2")
                  .arg(feedbackLinkedCount)
                  .arg(linkedMealCount));

    mealLogManager.reload();
    const QString insightKeyList = insightNames(mealLogManager.feedbackInsights());
    addResult(&results,
              QStringLiteral("Feedback insights surface recommendation/feedback chain summaries"),
              insightKeyList.contains(QStringLiteral("feedback_coverage")) &&
                  insightKeyList.contains(QStringLiteral("recommendation_hits")),
              QStringLiteral("insights: %1").arg(insightKeyList));
    addResult(&results,
              QStringLiteral("Feedback insights surface sleepiness-watch / stable-favorites / low-repeat signals"),
              insightKeyList.contains(QStringLiteral("sleepiness_watch")) &&
                  insightKeyList.contains(QStringLiteral("stable_favorites")) &&
                  insightKeyList.contains(QStringLiteral("low_repeat")),
              QStringLiteral("insights: %1").arg(insightKeyList));

    appSettings.clearLlmSettings();
    qunsetenv("MEALADVISOR_LLM_API_KEY");
    qunsetenv("MEALADVISOR_LLM_API_URL");
    qunsetenv("MEALADVISOR_LLM_MODEL");
    qunsetenv("OPENAI_API_KEY");
    recommendationEngine.clearSupplement();
    recommendationEngine.parseSupplement(QStringLiteral("想喝可乐"));
    addResult(&results,
              QStringLiteral("Supplement parser reports unconfigured state and fallback"),
              recommendationEngine.supplementState() == QStringLiteral("unconfigured") &&
                  recommendationEngine.supplementFallbackActive() &&
                  recommendationEngine.supplementStatus().contains(QStringLiteral("已回退")),
              QStringLiteral("%1 | %2")
                  .arg(recommendationEngine.supplementState(),
                       recommendationEngine.supplementStatus()));

    appSettings.saveLlmSettings(QStringLiteral("test-key"),
                                QStringLiteral("http://127.0.0.1:18080/v1/chat/completions"),
                                QStringLiteral("deepseek-chat"));

    const auto buildChatPayload = [](const QJsonObject &contractObject) {
        const QString content =
            QString::fromUtf8(QJsonDocument(contractObject).toJson(QJsonDocument::Compact));
        return QJsonDocument(QJsonObject{
                                 {QStringLiteral("choices"),
                                  QJsonArray{
                                      QJsonObject{
                                          {QStringLiteral("message"),
                                           QJsonObject{
                                               {QStringLiteral("role"),
                                                QStringLiteral("assistant")},
                                               {QStringLiteral("content"), content}}}}}}})
            .toJson(QJsonDocument::Compact);
    };

    const QJsonObject validContract{
        {QStringLiteral("version"), QStringLiteral("supplement_parser_v1")},
        {QStringLiteral("result"),
         QJsonObject{
             {QStringLiteral("hungerIntent"), 1.1},
             {QStringLiteral("carbIntent"), 0.95},
             {QStringLiteral("drinkIntent"), 1.2},
             {QStringLiteral("budgetFlexIntent"), 1.1},
             {QStringLiteral("classConstraintWeight"), 1.25},
             {QStringLiteral("postMealSleepPlan"), QStringLiteral("stay_awake")},
             {QStringLiteral("plannedNapMinutes"), 0},
             {QStringLiteral("sleepNeedLevel"), 1.6},
             {QStringLiteral("sleepPlanConfidence"), 0.75},
             {QStringLiteral("proteinIntent"), 1.1},
             {QStringLiteral("colaIntent"), 1.2},
             {QStringLiteral("flavorIntent"), 1.0},
             {QStringLiteral("relaxedTimePreference"), 0.8}}}
    };

    const auto validOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"), buildChatPayload(validContract));
    addResult(&results,
              QStringLiteral("Supplement parser accepts valid structured result"),
              validOutcome.accepted &&
                  validOutcome.state == QStringLiteral("success") &&
                  !validOutcome.fallbackUsed &&
                  qFuzzyCompare(validOutcome.adjustment.classConstraintWeight, 1.25),
              QStringLiteral("%1 | %2")
                  .arg(validOutcome.state, validOutcome.status));

    const auto malformedOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"),
            QByteArray(
                "{\"choices\":[{\"message\":{\"content\":\"not json\"}}]}"));
    addResult(&results,
              QStringLiteral("Supplement parser rejects non-JSON model output"),
              !malformedOutcome.accepted &&
                  malformedOutcome.state == QStringLiteral("invalid_response") &&
                  malformedOutcome.fallbackUsed,
              QStringLiteral("%1 | %2")
                  .arg(malformedOutcome.state, malformedOutcome.status));

    const QJsonObject invalidContract{
        {QStringLiteral("version"), QStringLiteral("supplement_parser_v1")},
        {QStringLiteral("result"),
         QJsonObject{
             {QStringLiteral("hungerIntent"), 1.1},
             {QStringLiteral("carbIntent"), 0.95}}}
    };
    const auto invalidStructureOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"), buildChatPayload(invalidContract));
    addResult(&results,
              QStringLiteral("Supplement parser rejects invalid result structure"),
              !invalidStructureOutcome.accepted &&
                  invalidStructureOutcome.state == QStringLiteral("invalid_response") &&
                  invalidStructureOutcome.fallbackUsed,
              QStringLiteral("%1 | %2")
                  .arg(invalidStructureOutcome.state,
                       invalidStructureOutcome.status));

    const QJsonObject invalidValueContract{
        {QStringLiteral("version"), QStringLiteral("supplement_parser_v1")},
        {QStringLiteral("result"),
         QJsonObject{
             {QStringLiteral("hungerIntent"), 1.1},
             {QStringLiteral("carbIntent"), 0.95},
             {QStringLiteral("drinkIntent"), 1.2},
             {QStringLiteral("budgetFlexIntent"), 1.1},
             {QStringLiteral("classConstraintWeight"), 1.7},
             {QStringLiteral("postMealSleepPlan"), QStringLiteral("stay_awake")},
             {QStringLiteral("plannedNapMinutes"), 25},
             {QStringLiteral("sleepNeedLevel"), 1.6},
             {QStringLiteral("sleepPlanConfidence"), 0.75},
             {QStringLiteral("proteinIntent"), 1.1},
             {QStringLiteral("colaIntent"), 1.2},
             {QStringLiteral("flavorIntent"), 1.0},
             {QStringLiteral("relaxedTimePreference"), 0.8}}}
    };
    const auto invalidValueOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"), buildChatPayload(invalidValueContract));
    addResult(&results,
              QStringLiteral("Supplement parser rejects invalid fixed-set values"),
              !invalidValueOutcome.accepted &&
                  invalidValueOutcome.state == QStringLiteral("invalid_response") &&
                  invalidValueOutcome.fallbackUsed,
              QStringLiteral("%1 | %2")
                  .arg(invalidValueOutcome.state,
                       invalidValueOutcome.status));

    const auto networkFailureOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"), QByteArray(),
            QStringLiteral("Connection refused"), false);
    addResult(&results,
              QStringLiteral("Supplement parser falls back on network failure"),
              !networkFailureOutcome.accepted &&
                  networkFailureOutcome.state == QStringLiteral("network_error") &&
                  networkFailureOutcome.fallbackUsed,
              QStringLiteral("%1 | %2")
                  .arg(networkFailureOutcome.state,
                       networkFailureOutcome.status));

    const auto timeoutOutcome =
        RecommendationEngine::evaluateSupplementResponse(
            QStringLiteral("想喝可乐"), QByteArray(),
            QStringLiteral("Operation canceled"), true);
    addResult(&results,
              QStringLiteral("Supplement parser falls back on timeout"),
              !timeoutOutcome.accepted &&
                  timeoutOutcome.state == QStringLiteral("network_error") &&
                  timeoutOutcome.fallbackUsed &&
                  timeoutOutcome.status.contains(QStringLiteral("超时")),
              QStringLiteral("%1 | %2")
                  .arg(timeoutOutcome.state, timeoutOutcome.status));

    appSettings.clearLlmSettings();
    qputenv("MEALADVISOR_LLM_API_KEY", QByteArray("env-key"));
    qputenv("MEALADVISOR_LLM_API_URL",
            QByteArray("http://127.0.0.1:28080/v1/chat/completions"));
    qputenv("MEALADVISOR_LLM_MODEL", QByteArray("env-model"));
    addResult(&results,
              QStringLiteral("AppConfig falls back to env vars when in-app values are empty"),
              AppConfig::llmApiConfigured() &&
                  AppConfig::llmApiKey() == QStringLiteral("env-key") &&
                  AppConfig::llmApiUrl() ==
                      QStringLiteral("http://127.0.0.1:28080/v1/chat/completions") &&
                  AppConfig::llmModel() == QStringLiteral("env-model"),
              QStringLiteral("%1 | %2 | %3")
                  .arg(AppConfig::llmApiKey(), AppConfig::llmApiUrl(),
                       AppConfig::llmModel()));

    appSettings.saveLlmSettings(QStringLiteral("local-key"), QString(),
                                QStringLiteral("local-model"));
    addResult(&results,
              QStringLiteral("In-app config overrides env key/model while blank fields still fall back to env"),
              AppConfig::llmApiConfigured() &&
                  AppConfig::llmApiKey() == QStringLiteral("local-key") &&
                  AppConfig::llmApiUrl() ==
                      QStringLiteral("http://127.0.0.1:28080/v1/chat/completions") &&
                  AppConfig::llmModel() == QStringLiteral("local-model"),
              QStringLiteral("%1 | %2 | %3")
                  .arg(AppConfig::llmApiKey(), AppConfig::llmApiUrl(),
                       AppConfig::llmModel()));

    appSettings.clearLlmSettings();
    qunsetenv("MEALADVISOR_LLM_API_KEY");
    qunsetenv("MEALADVISOR_LLM_API_URL");
    qunsetenv("MEALADVISOR_LLM_MODEL");

    QJsonObject extraFieldContract = validContract;
    QJsonObject extraFieldResult =
        extraFieldContract.value(QStringLiteral("result")).toObject();
    extraFieldResult.insert(QStringLiteral("unexpectedFlag"), true);
    extraFieldContract.insert(QStringLiteral("result"), extraFieldResult);

    MockOpenAiServer mockServer;
    QString mockServerError;
    const bool mockStarted = mockServer.start(&mockServerError);
    addResult(&results,
              QStringLiteral("Local supplement mock server starts"),
              mockStarted,
              mockStarted ? mockServer.chatCompletionsUrl() : mockServerError);

    if (mockStarted) {
        appSettings.saveLlmSettings(QStringLiteral("test-key"),
                                    mockServer.chatCompletionsUrl(),
                                    QStringLiteral("deepseek-chat"));

        const ParseScenarioResult validParse = runParseScenario(
            &recommendationEngine, &mockServer,
            QStringLiteral("Need to stay awake for afternoon class and maybe drink cola."),
            MockHttpResponse{200, buildChatPayload(validContract), "application/json", 0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path accepts valid structured result"),
                  validParse.completed &&
                      validParse.state == QStringLiteral("success") &&
                      !validParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(validParse.state, validParse.status));

        const QJsonObject capturedRequest = parseJsonObject(validParse.requestBody);
        const QJsonArray capturedMessages =
            capturedRequest.value(QStringLiteral("messages")).toArray();
        const QString userMessage =
            capturedMessages.size() >= 2
                ? capturedMessages.at(1).toObject().value(QStringLiteral("content")).toString()
                : QString();
        const bool requestShapeOk =
            capturedRequest.value(QStringLiteral("model")).toString() ==
                QStringLiteral("deepseek-chat") &&
            qFuzzyCompare(capturedRequest.value(QStringLiteral("temperature")).toDouble() + 1.0,
                          1.0) &&
            capturedRequest.value(QStringLiteral("response_format"))
                    .toObject()
                    .value(QStringLiteral("type"))
                    .toString() == QStringLiteral("json_object") &&
            capturedMessages.size() == 2 &&
            capturedMessages.at(0).toObject().value(QStringLiteral("role")).toString() ==
                QStringLiteral("system") &&
            capturedMessages.at(1).toObject().value(QStringLiteral("role")).toString() ==
                QStringLiteral("user") &&
            userMessage.contains(QStringLiteral("Current context:")) &&
            userMessage.contains(QStringLiteral("User supplement text:"));
        addResult(&results,
                  QStringLiteral("Supplement request uses chat-completions payload with strict JSON response_format"),
                  requestShapeOk,
                  QString::fromUtf8(validParse.requestBody));

        QJsonObject budgetRelaxContract = validContract;
        QJsonObject budgetRelaxResult;
        budgetRelaxResult.insert(QStringLiteral("hungerIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("carbIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("drinkIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("budgetFlexIntent"), 1.1);
        budgetRelaxResult.insert(QStringLiteral("classConstraintWeight"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("postMealSleepPlan"),
                                 QStringLiteral("unknown"));
        budgetRelaxResult.insert(QStringLiteral("plannedNapMinutes"), 0);
        budgetRelaxResult.insert(QStringLiteral("sleepNeedLevel"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("sleepPlanConfidence"), 0.0);
        budgetRelaxResult.insert(QStringLiteral("proteinIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("colaIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("flavorIntent"), 1.0);
        budgetRelaxResult.insert(QStringLiteral("relaxedTimePreference"), 1.0);
        budgetRelaxContract.insert(QStringLiteral("result"), budgetRelaxResult);

        const ParseScenarioResult budgetRelaxParse = runParseScenario(
            &recommendationEngine, &mockServer,
            QStringLiteral("今晚没课，预算可以放宽一点。"),
            MockHttpResponse{200, buildChatPayload(budgetRelaxContract),
                             "application/json", 0});
        const QVariantList budgetRelaxDinner = runScenario(
            &recommendationEngine, QStringLiteral("2026-04-23T18:04:00"));
        addResult(&results,
                  QStringLiteral("Budget-flex supplement lifts high-budget relaxed dinner candidates"),
                  budgetRelaxParse.completed &&
                      budgetRelaxParse.state == QStringLiteral("success") &&
                      top3Contains(budgetRelaxDinner,
                                   {QStringLiteral("牛肉火锅单人套餐")}),
                  QStringLiteral("%1 | top3: %2")
                      .arg(budgetRelaxParse.state,
                           joinCandidateNames(budgetRelaxDinner)));

        const ParseScenarioResult malformedParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{200,
                             QByteArray("{\"choices\":[{\"message\":{\"content\":\"not json\"}}]}"),
                             "application/json",
                             0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path rejects non-JSON model output"),
                  malformedParse.completed &&
                      malformedParse.state == QStringLiteral("invalid_response") &&
                      malformedParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(malformedParse.state, malformedParse.status));

        const ParseScenarioResult invalidStructureParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{200, buildChatPayload(invalidContract), "application/json", 0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path rejects missing fields"),
                  invalidStructureParse.completed &&
                      invalidStructureParse.state == QStringLiteral("invalid_response") &&
                      invalidStructureParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(invalidStructureParse.state, invalidStructureParse.status));

        const ParseScenarioResult extraFieldParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{200, buildChatPayload(extraFieldContract), "application/json", 0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path rejects extra fields"),
                  extraFieldParse.completed &&
                      extraFieldParse.state == QStringLiteral("invalid_response") &&
                      extraFieldParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(extraFieldParse.state, extraFieldParse.status));

        const ParseScenarioResult invalidValueParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{200, buildChatPayload(invalidValueContract), "application/json", 0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path rejects invalid fixed-set values"),
                  invalidValueParse.completed &&
                      invalidValueParse.state == QStringLiteral("invalid_response") &&
                      invalidValueParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(invalidValueParse.state, invalidValueParse.status));

        const ParseScenarioResult unauthorizedParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{401,
                             QByteArray("{\"error\":{\"message\":\"bad api key\"}}"),
                             "application/json",
                             0});
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path falls back on 401 or endpoint failure"),
                  unauthorizedParse.completed &&
                      unauthorizedParse.state == QStringLiteral("network_error") &&
                      unauthorizedParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(unauthorizedParse.state, unauthorizedParse.status));

        const ParseScenarioResult timeoutParse = runParseScenario(
            &recommendationEngine, &mockServer, QStringLiteral("鎯冲枬鍙箰"),
            MockHttpResponse{200, buildChatPayload(validContract), "application/json", 16000},
            19000);
        addResult(&results,
                  QStringLiteral("Supplement parser real parse path falls back on timeout"),
                  timeoutParse.completed &&
                      timeoutParse.state == QStringLiteral("network_error") &&
                      timeoutParse.fallbackActive,
                  QStringLiteral("%1 | %2")
                      .arg(timeoutParse.state, timeoutParse.status));
    }

    int passed = 0;
    QStringList issueSummaries;
    for (const ValidationCase &result : results) {
        if (result.ok) {
            ++passed;
        } else {
            issueSummaries.append(result.name + QStringLiteral(" -> ") + result.detail);
        }
    }

    std::cout << "MealAdvisor local validation\n";
    std::cout << "Database: " << qPrintable(databaseManager.databasePath()) << "\n";
    std::cout << "Imported schedule entries: " << schedules.size() << "\n";
    std::cout << "Imported merchants (" << importedMerchants.size() << "): "
              << qPrintable(importedMerchants.join(QStringLiteral(", "))) << "\n";
    std::cout << "Imported dishes (" << importedDishes.size() << "): "
              << qPrintable(importedDishes.join(QStringLiteral(", "))) << "\n";
    std::cout << "Imported meals (" << importedMeals.size() << "):\n";
    for (const QString &meal : importedMeals) {
        std::cout << "  - " << qPrintable(meal) << "\n";
    }
    std::cout << "\nValidation results (" << passed << "/" << results.size()
              << " passed)\n";
    for (const ValidationCase &result : results) {
        std::cout << (result.ok ? "[PASS] " : "[FAIL] ")
                  << qPrintable(result.name) << "\n";
        std::cout << "       " << qPrintable(result.detail) << "\n";
    }
    if (!issueSummaries.isEmpty()) {
        std::cout << "\nOpen validation issues:\n";
        for (const QString &issue : issueSummaries) {
            std::cout << "  - " << qPrintable(issue) << "\n";
        }
    }

    appendLog(QStringLiteral("Validation finished: %1/%2 passed")
                  .arg(passed)
                  .arg(results.size()));

    return 0;
}
