#include "planningrepository.h"

#include <QSqlDatabase>
#include <QSqlQuery>

PlanningRepository::PlanningRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

QList<PlanningPolicy> PlanningRepository::loadPlanningPolicies() const
{
    QList<PlanningPolicy> policies;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, name, enabled_weekdays, include_commute_days, "
        "skip_non_enabled_days, default_daily_budget, flexible_budget_cap, "
        "breakfast_recommendation_enabled, created_at, updated_at "
        "FROM planning_policies ORDER BY id"));

    if (!query.exec()) {
        return policies;
    }

    while (query.next()) {
        PlanningPolicy policy;
        policy.id = query.value(0).toInt();
        policy.name = query.value(1).toString();
        policy.enabledWeekdays = parseWeekdayList(query.value(2).toString());
        policy.includeCommuteDays = query.value(3).toBool();
        policy.skipNonEnabledDays = query.value(4).toBool();
        policy.defaultDailyBudget = query.value(5).toDouble();
        policy.flexibleBudgetCap = query.value(6).toDouble();
        policy.breakfastRecommendationEnabled = query.value(7).toBool();
        policy.createdAt = query.value(8).toDateTime();
        policy.updatedAt = query.value(9).toDateTime();
        policies.append(policy);
    }

    return policies;
}

QList<RecommendationProfile> PlanningRepository::loadRecommendationProfiles() const
{
    QList<RecommendationProfile> profiles;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, name, has_class_priority, health_priority, budget_priority, "
        "time_effort_priority, satiety_priority, carb_control_priority, "
        "fat_control_priority, protein_priority, sleepiness_avoid_priority, "
        "breakfast_recommendation_enabled, enabled_weekdays, planning_scope, "
        "is_default, created_at, updated_at "
        "FROM recommendation_profiles ORDER BY id"));

    if (!query.exec()) {
        return profiles;
    }

    while (query.next()) {
        RecommendationProfile profile;
        profile.id = query.value(0).toInt();
        profile.name = query.value(1).toString();
        profile.hasClassPriority = query.value(2).toInt();
        profile.healthPriority = query.value(3).toInt();
        profile.budgetPriority = query.value(4).toInt();
        profile.timeEffortPriority = query.value(5).toInt();
        profile.satietyPriority = query.value(6).toInt();
        profile.carbControlPriority = query.value(7).toInt();
        profile.fatControlPriority = query.value(8).toInt();
        profile.proteinPriority = query.value(9).toInt();
        profile.sleepinessAvoidPriority = query.value(10).toInt();
        profile.breakfastRecommendationEnabled = query.value(11).toBool();
        profile.enabledWeekdays = parseWeekdayList(query.value(12).toString());
        profile.planningScope = query.value(13).toString();
        profile.isDefault = query.value(14).toBool();
        profile.createdAt = query.value(15).toDateTime();
        profile.updatedAt = query.value(16).toDateTime();
        profiles.append(profile);
    }

    return profiles;
}
