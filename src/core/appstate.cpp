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
            labels.append(QStringLiteral("周一"));
            break;
        case 2:
            labels.append(QStringLiteral("周二"));
            break;
        case 3:
            labels.append(QStringLiteral("周三"));
            break;
        case 4:
            labels.append(QStringLiteral("周四"));
            break;
        case 5:
            labels.append(QStringLiteral("周五"));
            break;
        case 6:
            labels.append(QStringLiteral("周六"));
            break;
        case 7:
            labels.append(QStringLiteral("周日"));
            break;
        default:
            break;
        }
    }

    return labels.join(QStringLiteral("、"));
}

QString recommendationProfileDisplayName(const QString &name)
{
    if (name == QStringLiteral("Class Day")) {
        return QStringLiteral("有课日");
    }
    if (name == QStringLiteral("Commute Day")) {
        return QStringLiteral("通勤日");
    }
    return name;
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
        m_planningSummary = QStringLiteral("规划范围：%1")
                                .arg(weekdaySummary(policy.enabledWeekdays));
        m_budgetSummary = QStringLiteral("预算：日常 %1 元，弹性上限 %2 元")
                              .arg(policy.defaultDailyBudget, 0, 'f', 0)
                              .arg(policy.flexibleBudgetCap, 0, 'f', 0);
    } else {
        m_planningSummary = QStringLiteral("尚未加载规划策略。");
        m_budgetSummary = QStringLiteral("预算策略不可用。");
    }

    m_defaultProfileName = QStringLiteral("尚未加载推荐策略。");
    for (const RecommendationProfile &profile : profiles) {
        if (profile.isDefault) {
            m_defaultProfileName =
                QStringLiteral("默认推荐策略：%1")
                    .arg(recommendationProfileDisplayName(profile.name));
            break;
        }
    }

    m_activeDishCount = dishRepository.activeDishCount();
    m_mealLogCount = mealLogRepository.mealLogCount();

    emit stateChanged();
}
