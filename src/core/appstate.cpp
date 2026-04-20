#include "appstate.h"

#include "domainmodels.h"

#include "../data/dishrepository.h"
#include "../data/meallogrepository.h"
#include "../data/planningrepository.h"

#include <QStringList>

namespace
{
QString weekdaySummary(const QList<int> &weekdays)
{
    QStringList labels;

    for (const int weekday : weekdays) {
        switch (weekday) {
        case 1:
            labels.append(QStringLiteral("Mon"));
            break;
        case 2:
            labels.append(QStringLiteral("Tue"));
            break;
        case 3:
            labels.append(QStringLiteral("Wed"));
            break;
        case 4:
            labels.append(QStringLiteral("Thu"));
            break;
        case 5:
            labels.append(QStringLiteral("Fri"));
            break;
        case 6:
            labels.append(QStringLiteral("Sat"));
            break;
        case 7:
            labels.append(QStringLiteral("Sun"));
            break;
        default:
            break;
        }
    }

    return labels.join(QStringLiteral(", "));
}
}

AppState::AppState(const DatabaseManager &databaseManager, QObject *parent)
    : QObject(parent),
      m_databaseManager(databaseManager)
{
    reload();
}

QString AppState::databasePath() const
{
    return m_databaseManager.databasePath();
}

QString AppState::planningSummary() const
{
    return m_planningSummary;
}

QString AppState::defaultProfileName() const
{
    return m_defaultProfileName;
}

QString AppState::budgetSummary() const
{
    return m_budgetSummary;
}

int AppState::activeDishCount() const
{
    return m_activeDishCount;
}

int AppState::mealLogCount() const
{
    return m_mealLogCount;
}

void AppState::reload()
{
    PlanningRepository planningRepository(m_databaseManager.connectionName());
    DishRepository dishRepository(m_databaseManager.connectionName());
    MealLogRepository mealLogRepository(m_databaseManager.connectionName());

    const QList<PlanningPolicy> policies = planningRepository.loadPlanningPolicies();
    const QList<RecommendationProfile> profiles =
        planningRepository.loadRecommendationProfiles();

    if (!policies.isEmpty()) {
        const PlanningPolicy &policy = policies.first();
        m_planningSummary = QStringLiteral("Active planning: %1")
                                .arg(weekdaySummary(policy.enabledWeekdays));
        m_budgetSummary = QStringLiteral("Budget: RMB %1, flexible up to %2")
                              .arg(policy.defaultDailyBudget, 0, 'f', 0)
                              .arg(policy.flexibleBudgetCap, 0, 'f', 0);
    } else {
        m_planningSummary = QStringLiteral("No planning policy loaded.");
        m_budgetSummary = QStringLiteral("Budget policy unavailable.");
    }

    m_defaultProfileName = QStringLiteral("No recommendation profile loaded.");
    for (const RecommendationProfile &profile : profiles) {
        if (profile.isDefault) {
            m_defaultProfileName =
                QStringLiteral("Default profile: %1").arg(profile.name);
            break;
        }
    }

    m_activeDishCount = dishRepository.activeDishCount();
    m_mealLogCount = mealLogRepository.mealLogCount();

    emit stateChanged();
}
