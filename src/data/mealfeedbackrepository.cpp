#include "mealfeedbackrepository.h"

#include <QDateTime>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>

#include <cmath>

namespace
{
struct WeightedAccumulator
{
    int sampleCount = 0;
    int recentSampleCount = 0;
    double effectiveSampleWeight = 0.0;

    double tasteSum = 0.0;
    double tasteWeight = 0.0;
    double repeatSum = 0.0;
    double repeatWeight = 0.0;
    double sleepinessSum = 0.0;
    double sleepinessWeight = 0.0;
    double comfortSum = 0.0;
    double comfortWeight = 0.0;
    double focusSum = 0.0;
    double focusWeight = 0.0;
    double wouldEatAgainSum = 0.0;
    double wouldEatAgainWeight = 0.0;
};

double clampWeight(double value, double minimumValue, double maximumValue)
{
    return qBound(minimumValue, value, maximumValue);
}

QDateTime parseSqlDateTime(const QVariant &value)
{
    const QDateTime parsedIso = QDateTime::fromString(value.toString(), Qt::ISODate);
    if (parsedIso.isValid()) {
        return parsedIso;
    }

    return QDateTime::fromString(value.toString(), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

double feedbackEntryWeight(const QString &mealType,
                           bool isBeverage,
                           double portionRatio,
                           double mealImpactWeight,
                           const QDateTime &eatenAt,
                           const QDateTime &now)
{
    const double portionWeight = clampWeight(portionRatio > 0.0 ? portionRatio : 1.0, 0.35, 1.75);
    const double impactWeight =
        clampWeight(mealImpactWeight > 0.0 ? mealImpactWeight : 1.0, 0.25, 1.75);

    double contextWeight = 1.0;
    if (mealType == QStringLiteral("snack")) {
        contextWeight *= 0.58;
    }
    if (isBeverage) {
        contextWeight *= 0.72;
    }
    if (mealType == QStringLiteral("breakfast")) {
        contextWeight *= 0.90;
    }

    double recencyWeight = 1.0;
    if (eatenAt.isValid()) {
        const double daysAgo = std::max(0.0, eatenAt.secsTo(now) / 86400.0);
        recencyWeight = clampWeight(std::exp(-daysAgo / 45.0), 0.18, 1.0);
    }

    return portionWeight * impactWeight * contextWeight * recencyWeight;
}

void addWeightedMetric(double value, double weight, double *sum, double *denominator)
{
    if (value <= 0.0 || weight <= 0.0) {
        return;
    }

    *sum += value * weight;
    *denominator += weight;
}

double finalizeAverage(double sum, double denominator)
{
    return denominator > 0.0 ? sum / denominator : 0.0;
}
}

MealFeedbackRepository::MealFeedbackRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

MealFeedback MealFeedbackRepository::loadFeedbackForMealLog(int mealLogId) const
{
    MealFeedback feedback;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, meal_log_id, recommendation_record_id, fullness_level, "
        "sleepiness_level, comfort_level, focus_impact_level, taste_rating, "
        "repeat_willingness, would_eat_again, free_text_feedback, created_at "
        "FROM meal_feedback WHERE meal_log_id = ?"));
    query.addBindValue(mealLogId);

    if (!query.exec() || !query.next()) {
        return feedback;
    }

    feedback.id = query.value(0).toInt();
    feedback.mealLogId = query.value(1).toInt();
    feedback.recommendationRecordId = query.value(2).toInt();
    feedback.fullnessLevel = query.value(3).toInt();
    feedback.sleepinessLevel = query.value(4).toInt();
    feedback.comfortLevel = query.value(5).toInt();
    feedback.focusImpactLevel = query.value(6).toInt();
    feedback.tasteRating = query.value(7).toInt();
    feedback.repeatWillingness = query.value(8).toInt();
    feedback.wouldEatAgain = query.value(9).toBool();
    feedback.freeTextFeedback = query.value(10).toString();
    feedback.createdAt = parseSqlDateTime(query.value(11));
    return feedback;
}

bool MealFeedbackRepository::upsertFeedback(const MealFeedback &feedback,
                                            QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "INSERT INTO meal_feedback "
        "(meal_log_id, recommendation_record_id, fullness_level, sleepiness_level, comfort_level, "
        "focus_impact_level, taste_rating, repeat_willingness, would_eat_again, "
        "free_text_feedback, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, datetime('now')) "
        "ON CONFLICT(meal_log_id) DO UPDATE SET "
        "recommendation_record_id = excluded.recommendation_record_id, "
        "fullness_level = excluded.fullness_level, "
        "sleepiness_level = excluded.sleepiness_level, "
        "comfort_level = excluded.comfort_level, "
        "focus_impact_level = excluded.focus_impact_level, "
        "taste_rating = excluded.taste_rating, "
        "repeat_willingness = excluded.repeat_willingness, "
        "would_eat_again = excluded.would_eat_again, "
        "free_text_feedback = excluded.free_text_feedback"));
    query.addBindValue(feedback.mealLogId);
    query.addBindValue(feedback.recommendationRecordId > 0
                           ? QVariant(feedback.recommendationRecordId)
                           : QVariant());
    query.addBindValue(feedback.fullnessLevel > 0 ? QVariant(feedback.fullnessLevel)
                                                  : QVariant());
    query.addBindValue(feedback.sleepinessLevel > 0 ? QVariant(feedback.sleepinessLevel)
                                                    : QVariant());
    query.addBindValue(feedback.comfortLevel > 0 ? QVariant(feedback.comfortLevel)
                                                 : QVariant());
    query.addBindValue(feedback.focusImpactLevel > 0 ? QVariant(feedback.focusImpactLevel)
                                                     : QVariant());
    query.addBindValue(feedback.tasteRating > 0 ? QVariant(feedback.tasteRating)
                                                : QVariant());
    query.addBindValue(feedback.repeatWillingness > 0 ? QVariant(feedback.repeatWillingness)
                                                      : QVariant());
    query.addBindValue(feedback.wouldEatAgain);
    query.addBindValue(feedback.freeTextFeedback.trimmed().isEmpty()
                           ? QVariant()
                           : QVariant(feedback.freeTextFeedback.trimmed()));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

QList<DishFeedbackAggregate> MealFeedbackRepository::loadDishFeedbackAggregates() const
{
    QList<DishFeedbackAggregate> aggregates;
    QHash<int, WeightedAccumulator> accumulatorByDishId;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT mld.dish_id, mld.portion_ratio, d.meal_impact_weight, d.is_beverage, "
        "ml.meal_type, ml.eaten_at, mf.taste_rating, mf.repeat_willingness, "
        "mf.sleepiness_level, mf.comfort_level, mf.focus_impact_level, mf.would_eat_again "
        "FROM meal_feedback mf "
        "INNER JOIN meal_logs ml ON ml.id = mf.meal_log_id "
        "INNER JOIN meal_log_dishes mld ON mld.meal_log_id = mf.meal_log_id "
        "INNER JOIN dishes d ON d.id = mld.dish_id"));

    if (!query.exec()) {
        return aggregates;
    }

    const QDateTime now = QDateTime::currentDateTime();
    while (query.next()) {
        const int dishId = query.value(0).toInt();
        if (dishId <= 0) {
            continue;
        }

        WeightedAccumulator &accumulator = accumulatorByDishId[dishId];
        const QDateTime eatenAt = parseSqlDateTime(query.value(5));
        const double weight = feedbackEntryWeight(query.value(4).toString(),
                                                  query.value(3).toBool(),
                                                  query.value(1).toDouble(),
                                                  query.value(2).toDouble(),
                                                  eatenAt, now);

        accumulator.sampleCount += 1;
        accumulator.effectiveSampleWeight += weight;
        if (eatenAt.isValid() && eatenAt.daysTo(now) <= 21) {
            accumulator.recentSampleCount += 1;
        }

        addWeightedMetric(query.value(6).toDouble(), weight,
                          &accumulator.tasteSum, &accumulator.tasteWeight);
        addWeightedMetric(query.value(7).toDouble(), weight,
                          &accumulator.repeatSum, &accumulator.repeatWeight);
        addWeightedMetric(query.value(8).toDouble(), weight,
                          &accumulator.sleepinessSum, &accumulator.sleepinessWeight);
        addWeightedMetric(query.value(9).toDouble(), weight,
                          &accumulator.comfortSum, &accumulator.comfortWeight);
        addWeightedMetric(query.value(10).toDouble(), weight,
                          &accumulator.focusSum, &accumulator.focusWeight);
        if (!query.isNull(11)) {
            accumulator.wouldEatAgainSum += (query.value(11).toBool() ? 1.0 : 0.0) * weight;
            accumulator.wouldEatAgainWeight += weight;
        }
    }

    for (auto it = accumulatorByDishId.constBegin(); it != accumulatorByDishId.constEnd(); ++it) {
        DishFeedbackAggregate aggregate;
        aggregate.dishId = it.key();
        aggregate.sampleCount = it.value().sampleCount;
        aggregate.recentSampleCount = it.value().recentSampleCount;
        aggregate.effectiveSampleWeight = it.value().effectiveSampleWeight;
        aggregate.avgTasteRating =
            finalizeAverage(it.value().tasteSum, it.value().tasteWeight);
        aggregate.avgRepeatWillingness =
            finalizeAverage(it.value().repeatSum, it.value().repeatWeight);
        aggregate.avgSleepinessLevel =
            finalizeAverage(it.value().sleepinessSum, it.value().sleepinessWeight);
        aggregate.avgComfortLevel =
            finalizeAverage(it.value().comfortSum, it.value().comfortWeight);
        aggregate.avgFocusImpactLevel =
            finalizeAverage(it.value().focusSum, it.value().focusWeight);
        aggregate.wouldEatAgainRate = finalizeAverage(
            it.value().wouldEatAgainSum, it.value().wouldEatAgainWeight);
        aggregates.append(aggregate);
    }

    return aggregates;
}
