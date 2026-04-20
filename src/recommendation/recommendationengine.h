#pragma once

#include "../data/databasemanager.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class RecommendationEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString summary READ summary NOTIFY recommendationsChanged)
    Q_PROPERTY(QVariantList candidates READ candidates NOTIFY recommendationsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool apiConfigured READ apiConfigured CONSTANT)
    Q_PROPERTY(QString supplementSummary READ supplementSummary NOTIFY supplementChanged)
    Q_PROPERTY(QString supplementStatus READ supplementStatus NOTIFY supplementChanged)
    Q_PROPERTY(QVariantList supplementWeights READ supplementWeights NOTIFY supplementChanged)
    Q_PROPERTY(QVariantList activeWeightConfig READ activeWeightConfig NOTIFY weightConfigChanged)

public:
    struct SupplementAdjustment
    {
        bool hasParsed = false;
        QString sourceText;
        QString summary;
        double hungerIntent = 0.0;
        double carbIntent = 0.0;
        double drinkIntent = 0.0;
        double budgetFlexIntent = 0.0;
        bool skipClassConstraint = false;
        QString postMealSleepPlan = QStringLiteral("unknown");
        int plannedNapMinutes = 0;
        double sleepNeedLevel = 0.0;
        double sleepPlanConfidence = 0.0;
        double proteinIntent = 0.0;
        double colaIntent = 0.0;
        double flavorIntent = 0.0;
        bool relaxedTimePreference = false;
    };

    explicit RecommendationEngine(const DatabaseManager &databaseManager,
                                  QObject *parent = nullptr);

    QString summary() const;
    QVariantList candidates() const;
    bool busy() const;
    bool apiConfigured() const;
    QString supplementSummary() const;
    QString supplementStatus() const;
    QVariantList supplementWeights() const;
    QVariantList activeWeightConfig() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void runDecision();
    Q_INVOKABLE QString previewRecommendation() const;
    Q_INVOKABLE void parseSupplement(const QString &text);
    Q_INVOKABLE void clearSupplement();
    Q_INVOKABLE void setWeightOverrides(const QVariantMap &overrides);
    Q_INVOKABLE void clearWeightOverrides();

signals:
    void recommendationsChanged();
    void supplementChanged();
    void busyChanged();
    void weightConfigChanged();

private:
    void setBusy(bool busy);
    void setInitialState();
    void applyParsedSupplement(const SupplementAdjustment &adjustment);

    const DatabaseManager &m_databaseManager;
    QNetworkAccessManager m_networkAccessManager;
    QString m_summary;
    QVariantList m_candidates;
    bool m_busy = false;
    QString m_supplementSummary;
    QString m_supplementStatus;
    QVariantList m_supplementWeights;
    QVariantList m_activeWeightConfig;
    QVariantMap m_weightOverrides;
    SupplementAdjustment m_adjustment;
};
