#pragma once

#include <QObject>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);

    bool initialize();
    QString lastError() const;
    QString databasePath() const;
    QString connectionName() const;

private:
    bool openConnection();
    bool applySchema();
    bool applyMigrations();
    bool seedClassPeriods();
    bool seedPlanningPolicies();
    bool seedRecommendationProfiles();

    QString m_connectionName;
    QString m_databasePath;
    QString m_lastError;
};
