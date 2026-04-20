#pragma once

#include "../core/domainmodels.h"

#include <QString>

class PlanningRepository
{
public:
    explicit PlanningRepository(QString connectionName);

    QList<PlanningPolicy> loadPlanningPolicies() const;
    QList<RecommendationProfile> loadRecommendationProfiles() const;

private:
    QString m_connectionName;
};
