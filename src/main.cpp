#include <QGuiApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSslSocket>

#include "core/appconfig.h"
#include "core/appstate.h"
#include "core/foodmanager.h"
#include "core/meallogmanager.h"
#include "core/schedulemanager.h"
#include "data/databasemanager.h"
#include "recommendation/recommendationengine.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    qputenv("ANDROID_OPENSSL_SUFFIX", "_3");
#endif

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("MealAdvisor"));
    QCoreApplication::setApplicationName(QStringLiteral("MealAdvisor"));

    qInfo() << "Device supports SSL:" << QSslSocket::supportsSsl()
            << QSslSocket::sslLibraryBuildVersionString()
            << QSslSocket::sslLibraryVersionString();

    QQmlApplicationEngine engine;
    DatabaseManager databaseManager;

    if (!databaseManager.initialize()) {
        qFatal("Database initialization failed: %s",
               qPrintable(databaseManager.lastError()));
    }

    AppState appState(databaseManager);
    FoodManager foodManager(databaseManager);
    MealLogManager mealLogManager(databaseManager);
    ScheduleManager scheduleManager(databaseManager);
    AppSettings appSettings;
    RecommendationEngine recommendationEngine(databaseManager, &appSettings);

    engine.rootContext()->setContextProperty("appConfig", &appSettings);
    engine.rootContext()->setContextProperty("appState", &appState);
    engine.rootContext()->setContextProperty("databaseManager", &databaseManager);
    engine.rootContext()->setContextProperty("foodManager", &foodManager);
    engine.rootContext()->setContextProperty("mealLogManager", &mealLogManager);
    engine.rootContext()->setContextProperty("scheduleManager", &scheduleManager);
    engine.rootContext()->setContextProperty("recommendationEngine",
                                             &recommendationEngine);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MealAdvisor", "Main");

    return app.exec();
}
