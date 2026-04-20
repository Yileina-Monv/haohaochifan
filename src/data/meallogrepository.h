#pragma once

#include "../core/domainmodels.h"

#include <QString>

class MealLogRepository
{
public:
    explicit MealLogRepository(QString connectionName);

    MealLog loadMealLog(int mealLogId) const;
    QList<MealLogDishItem> loadMealLogDishItems(int mealLogId) const;
    bool addMealLog(const MealLog &mealLog,
                    const QList<MealLogDishItem> &dishItems,
                    int *mealLogId = nullptr,
                    QString *errorMessage = nullptr) const;
    bool updateMealLog(int mealLogId,
                       const MealLog &mealLog,
                       const QList<MealLogDishItem> &dishItems,
                       QString *errorMessage = nullptr) const;
    bool deleteMealLog(int mealLogId,
                       QString *errorMessage = nullptr) const;
    QList<MealLog> loadRecentMealLogs(int limit = 10) const;
    QList<int> loadRecentDishIds(int limit = 12) const;
    int mealLogCount() const;

private:
    QString m_connectionName;
};
