#pragma once

#include "../data/databasemanager.h"

#include <QObject>
#include <QString>

class AppState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString databasePath READ databasePath NOTIFY stateChanged)
    Q_PROPERTY(QString planningSummary READ planningSummary NOTIFY stateChanged)
    Q_PROPERTY(QString defaultProfileName READ defaultProfileName NOTIFY stateChanged)
    Q_PROPERTY(QString budgetSummary READ budgetSummary NOTIFY stateChanged)
    Q_PROPERTY(int activeDishCount READ activeDishCount NOTIFY stateChanged)
    Q_PROPERTY(int mealLogCount READ mealLogCount NOTIFY stateChanged)

public:
    explicit AppState(const DatabaseManager &databaseManager,
                      QObject *parent = nullptr);

    QString databasePath() const;
    QString planningSummary() const;
    QString defaultProfileName() const;
    QString budgetSummary() const;
    int activeDishCount() const;
    int mealLogCount() const;

    Q_INVOKABLE void reload();

signals:
    void stateChanged();

private:
    const DatabaseManager &m_databaseManager;
    QString m_planningSummary;
    QString m_defaultProfileName;
    QString m_budgetSummary;
    int m_activeDishCount = 0;
    int m_mealLogCount = 0;
};
