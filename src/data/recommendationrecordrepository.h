#pragma once

#include "../core/domainmodels.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariantMap>

class RecommendationRecordRepository
{
public:
    explicit RecommendationRecordRepository(QString connectionName);

    bool addRecord(const RecommendationRecord &record,
                   QString *errorMessage = nullptr) const;
    bool markMatchingRecommendationSelected(const QString &mealType,
                                            const QList<int> &selectedDishIds,
                                            const QDateTime &selectedAt,
                                            int mealLogId,
                                            int *matchedRecordId = nullptr,
                                            int *matchedDishId = nullptr,
                                            int *matchedCandidateRank = nullptr,
                                            QString *errorMessage = nullptr) const;
    int loadSelectedRecommendationRecordIdForMealLog(int mealLogId) const;
    RecommendationSelectionLink loadSelectionLinkForMealLog(int mealLogId) const;
    QVariantMap loadRecordDetails(int recordId) const;

private:
    QString m_connectionName;
};
