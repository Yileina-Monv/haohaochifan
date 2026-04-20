#pragma once

#include "../core/domainmodels.h"

#include <QDateTime>
#include <QString>

class MealFeedbackRepository
{
public:
    explicit MealFeedbackRepository(QString connectionName);

    MealFeedback loadFeedbackForMealLog(int mealLogId) const;
    bool upsertFeedback(const MealFeedback &feedback,
                        QString *errorMessage = nullptr) const;
    QList<DishFeedbackAggregate> loadDishFeedbackAggregates() const;

private:
    QString m_connectionName;
};
