#pragma once

#include "../data/databasemanager.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class AppSettings;

class RecommendationEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString summary READ summary NOTIFY recommendationsChanged)
    Q_PROPERTY(QVariantList candidates READ candidates NOTIFY recommendationsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool apiConfigured READ apiConfigured NOTIFY supplementChanged)
    Q_PROPERTY(QString supplementSummary READ supplementSummary NOTIFY supplementChanged)
    Q_PROPERTY(QString supplementStatus READ supplementStatus NOTIFY supplementChanged)
    Q_PROPERTY(QString supplementState READ supplementState NOTIFY supplementChanged)
    Q_PROPERTY(bool supplementFallbackActive READ supplementFallbackActive NOTIFY supplementChanged)
    Q_PROPERTY(QVariantList supplementWeights READ supplementWeights NOTIFY supplementChanged)
    Q_PROPERTY(QString feedbackParseStatus READ feedbackParseStatus NOTIFY feedbackParseChanged)
    Q_PROPERTY(QString feedbackParseState READ feedbackParseState NOTIFY feedbackParseChanged)
    Q_PROPERTY(bool feedbackParseBusy READ feedbackParseBusy NOTIFY feedbackParseChanged)
    Q_PROPERTY(bool feedbackParseFallbackActive READ feedbackParseFallbackActive NOTIFY feedbackParseChanged)
    Q_PROPERTY(QVariantMap parsedFeedback READ parsedFeedback NOTIFY feedbackParseChanged)
    Q_PROPERTY(QString dishParseStatus READ dishParseStatus NOTIFY dishParseChanged)
    Q_PROPERTY(QString dishParseState READ dishParseState NOTIFY dishParseChanged)
    Q_PROPERTY(bool dishParseBusy READ dishParseBusy NOTIFY dishParseChanged)
    Q_PROPERTY(bool dishParseFallbackActive READ dishParseFallbackActive NOTIFY dishParseChanged)
    Q_PROPERTY(QVariantMap parsedDish READ parsedDish NOTIFY dishParseChanged)
    Q_PROPERTY(QVariantList activeWeightConfig READ activeWeightConfig NOTIFY weightConfigChanged)
    Q_PROPERTY(QString llmConnectionTestStatus READ llmConnectionTestStatus NOTIFY llmConnectionTestChanged)
    Q_PROPERTY(QString llmConnectionTestState READ llmConnectionTestState NOTIFY llmConnectionTestChanged)
    Q_PROPERTY(bool llmConnectionTestBusy READ llmConnectionTestBusy NOTIFY llmConnectionTestChanged)

public:
    struct SupplementAdjustment
    {
        bool hasParsed = false;
        bool fallbackUsed = false;
        QString sourceText;
        QString summary;
        double hungerIntent = 1.0;
        double carbIntent = 1.0;
        double drinkIntent = 1.0;
        double budgetFlexIntent = 1.0;
        bool skipClassConstraint = false;
        double classConstraintWeight = 1.0;
        QString postMealSleepPlan = QStringLiteral("unknown");
        int plannedNapMinutes = 0;
        double sleepNeedLevel = 1.0;
        double sleepPlanConfidence = 0.0;
        double proteinIntent = 1.0;
        double colaIntent = 1.0;
        double flavorIntent = 1.0;
        double relaxedTimePreference = 1.0;
    };

    struct SupplementParseOutcome
    {
        SupplementAdjustment adjustment;
        QString state;
        QString status;
        bool accepted = false;
        bool fallbackUsed = false;
    };

    struct FeedbackParseResult
    {
        int fullnessLevel = 3;
        int sleepinessLevel = 3;
        int comfortLevel = 3;
        int focusImpactLevel = 3;
        int tasteRating = 3;
        int repeatWillingness = 3;
        bool wouldEatAgain = true;
        QString freeTextFeedback;
    };

    struct FeedbackParseOutcome
    {
        FeedbackParseResult result;
        QString state;
        QString status;
        bool accepted = false;
        bool fallbackUsed = false;
    };

    struct DishParseResult
    {
        QString name;
        QString category;
        double price = 0.0;
        QString defaultDiningMode = QStringLiteral("dine_in");
        int eatTimeMinutes = 15;
        int acquireEffortScore = 1;
        QString carbLevel = QStringLiteral("medium");
        QString fatLevel = QStringLiteral("medium");
        QString proteinLevel = QStringLiteral("medium");
        QString vitaminLevel = QStringLiteral("medium");
        QString fiberLevel = QStringLiteral("medium");
        QString satietyLevel = QStringLiteral("medium");
        QString digestiveBurdenLevel = QStringLiteral("medium");
        QString sleepinessRiskLevel = QStringLiteral("medium");
        QString flavorLevel = QStringLiteral("medium");
        QString odorLevel = QStringLiteral("low");
        bool isCombo = false;
        bool isBeverage = false;
        double mealImpactWeight = 1.0;
        QString notes;
    };

    struct DishParseOutcome
    {
        DishParseResult result;
        QString state;
        QString status;
        bool accepted = false;
        bool fallbackUsed = false;
    };

    explicit RecommendationEngine(const DatabaseManager &databaseManager,
                                  AppSettings *appSettings = nullptr,
                                  QObject *parent = nullptr);

    QString summary() const;
    QVariantList candidates() const;
    bool busy() const;
    bool apiConfigured() const;
    QString supplementSummary() const;
    QString supplementStatus() const;
    QString supplementState() const;
    bool supplementFallbackActive() const;
    QVariantList supplementWeights() const;
    QString feedbackParseStatus() const;
    QString feedbackParseState() const;
    bool feedbackParseBusy() const;
    bool feedbackParseFallbackActive() const;
    QVariantMap parsedFeedback() const;
    QString dishParseStatus() const;
    QString dishParseState() const;
    bool dishParseBusy() const;
    bool dishParseFallbackActive() const;
    QVariantMap parsedDish() const;
    QVariantList activeWeightConfig() const;
    QString llmConnectionTestStatus() const;
    QString llmConnectionTestState() const;
    bool llmConnectionTestBusy() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void runDecision();
    Q_INVOKABLE QString previewRecommendation() const;
    Q_INVOKABLE void parseSupplement(const QString &text);
    Q_INVOKABLE void parseFeedback(const QString &text, const QString &mealSummary);
    Q_INVOKABLE void parseDishInput(const QString &text, const QString &merchantName);
    Q_INVOKABLE void clearSupplement();
    Q_INVOKABLE void clearFeedbackParse();
    Q_INVOKABLE void clearDishParse();
    Q_INVOKABLE void setWeightOverrides(const QVariantMap &overrides);
    Q_INVOKABLE void clearWeightOverrides();
    Q_INVOKABLE void refreshSupplementConfigState();
    Q_INVOKABLE void testLlmConnection(const QString &apiKey,
                                       const QString &apiUrl,
                                       const QString &model);

    static SupplementAdjustment neutralSupplementAdjustment();
    static FeedbackParseResult neutralFeedbackParseResult();
    static DishParseResult neutralDishParseResult();
    static SupplementParseOutcome evaluateSupplementResponse(const QString &sourceText,
                                                            const QByteArray &responseBody,
                                                            const QString &networkError = QString(),
                                                            bool timedOut = false);
    static FeedbackParseOutcome evaluateFeedbackResponse(const QString &sourceText,
                                                        const QByteArray &responseBody,
                                                        const QString &networkError = QString(),
                                                        bool timedOut = false);
    static DishParseOutcome evaluateDishParseResponse(const QString &sourceText,
                                                      const QByteArray &responseBody,
                                                      const QString &networkError = QString(),
                                                      bool timedOut = false);

signals:
    void recommendationsChanged();
    void supplementChanged();
    void feedbackParseChanged();
    void dishParseChanged();
    void busyChanged();
    void weightConfigChanged();
    void llmConnectionTestChanged();

private:
    void setBusy(bool busy);
    void setFeedbackParseBusy(bool busy);
    void setDishParseBusy(bool busy);
    void setInitialState();
    void applySupplementOutcome(const SupplementParseOutcome &outcome);
    void applyFeedbackOutcome(const FeedbackParseOutcome &outcome);
    void applyDishParseOutcome(const DishParseOutcome &outcome);

    const DatabaseManager &m_databaseManager;
    AppSettings *m_appSettings = nullptr;
    QNetworkAccessManager m_networkAccessManager;
    QString m_summary;
    QVariantList m_candidates;
    bool m_busy = false;
    QString m_supplementSummary;
    QString m_supplementStatus;
    QString m_supplementState;
    bool m_supplementFallbackActive = false;
    QVariantList m_supplementWeights;
    bool m_feedbackParseBusy = false;
    QString m_feedbackParseStatus;
    QString m_feedbackParseState;
    bool m_feedbackParseFallbackActive = false;
    QVariantMap m_parsedFeedback;
    bool m_dishParseBusy = false;
    QString m_dishParseStatus;
    QString m_dishParseState;
    bool m_dishParseFallbackActive = false;
    QVariantMap m_parsedDish;
    QVariantList m_activeWeightConfig;
    bool m_llmConnectionTestBusy = false;
    QString m_llmConnectionTestStatus;
    QString m_llmConnectionTestState;
    QVariantMap m_weightOverrides;
    SupplementAdjustment m_adjustment;
};
