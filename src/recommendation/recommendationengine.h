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
    Q_PROPERTY(QVariantList activeWeightConfig READ activeWeightConfig NOTIFY weightConfigChanged)

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
    QVariantList activeWeightConfig() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void runDecision();
    Q_INVOKABLE QString previewRecommendation() const;
    Q_INVOKABLE void parseSupplement(const QString &text);
    Q_INVOKABLE void clearSupplement();
    Q_INVOKABLE void setWeightOverrides(const QVariantMap &overrides);
    Q_INVOKABLE void clearWeightOverrides();
    Q_INVOKABLE void refreshSupplementConfigState();

    static SupplementAdjustment neutralSupplementAdjustment();
    static SupplementParseOutcome evaluateSupplementResponse(const QString &sourceText,
                                                            const QByteArray &responseBody,
                                                            const QString &networkError = QString(),
                                                            bool timedOut = false);

signals:
    void recommendationsChanged();
    void supplementChanged();
    void busyChanged();
    void weightConfigChanged();

private:
    void setBusy(bool busy);
    void setInitialState();
    void applySupplementOutcome(const SupplementParseOutcome &outcome);

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
    QVariantList m_activeWeightConfig;
    QVariantMap m_weightOverrides;
    SupplementAdjustment m_adjustment;
};
