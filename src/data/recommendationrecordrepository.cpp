#include "recommendationrecordrepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QStringList>

#include <algorithm>

namespace
{
QString trimmedPreviewText(const QString &text, int maxLength = 96)
{
    const QString simplified = text.simplified();
    if (simplified.size() <= maxLength) {
        return simplified;
    }

    return simplified.left(std::max(0, maxLength - 1)) + QStringLiteral("…");
}

QString leadingSentence(const QString &text, int maxLength = 88)
{
    const QString simplified = text.simplified();
    if (simplified.isEmpty()) {
        return QString();
    }

    int cutoff = simplified.size();
    const QStringList separators = {
        QStringLiteral("。"),
        QStringLiteral("；"),
        QStringLiteral("，")
    };

    for (const QString &separator : separators) {
        const int index = simplified.indexOf(separator);
        if (index >= 0) {
            cutoff = std::min(cutoff, index + static_cast<int>(separator.size()));
        }
    }

    return trimmedPreviewText(simplified.left(cutoff), maxLength);
}

double scoreGapValueV2(double topScore, double selectedScore)
{
    if (topScore <= 0.0 || selectedScore <= 0.0) {
        return -1.0;
    }
    return std::max(0.0, topScore - selectedScore);
}

QString scoreGapSummaryV2(double topScore, double selectedScore)
{
    const double gap = scoreGapValueV2(topScore, selectedScore);
    if (gap < 0.0) {
        return QString();
    }
    if (gap <= 0.15) {
        return QStringLiteral("这次实际选择和首推几乎打平，更像是临场取舍差异。");
    }
    if (gap <= 0.80) {
        return QStringLiteral("这次实际选择只比首推低 %1 分，属于非常接近的一组候选。")
            .arg(QString::number(gap, 'f', 1));
    }
    if (gap <= 1.80) {
        return QStringLiteral("这次实际选择比首推低 %1 分，建议优先对照两边理由是否都成立。")
            .arg(QString::number(gap, 'f', 1));
    }
    return QStringLiteral("首推比实际选择高 %1 分，说明当时排序判断的差距比较明显。")
        .arg(QString::number(gap, 'f', 1));
}

QString scoreGapTagV2(double topScore, double candidateScore)
{
    const double gap = scoreGapValueV2(topScore, candidateScore);
    if (gap < 0.0) {
        return QString();
    }
    if (gap <= 0.15) {
        return QStringLiteral("几乎打平");
    }
    if (gap <= 0.80) {
        return QStringLiteral("只差 %1 分").arg(QString::number(gap, 'f', 1));
    }
    return QStringLiteral("低 %1 分").arg(QString::number(gap, 'f', 1));
}

QString candidateBadgeTextV2(int rank, bool selected)
{
    if (selected) {
        return rank == 1 ? QStringLiteral("首推，也是这次实际选择")
                         : QStringLiteral("这次实际选择");
    }
    return rank == 1 ? QStringLiteral("当时首推")
                     : QStringLiteral("第 %1 候选").arg(rank);
}

QString selectionSummaryV2(int selectedCandidateRank, const QString &selectedDishName)
{
    if (selectedCandidateRank > 0 && !selectedDishName.isEmpty()) {
        return QStringLiteral("这次实际吃的是当时的第 %1 候选：%2")
            .arg(selectedCandidateRank)
            .arg(selectedDishName);
    }
    return QStringLiteral("这条历史记录还没有读到明确的最终命中候选。");
}

QString comparisonSummaryV2(int selectedCandidateRank,
                            const QString &topCandidateName,
                            const QString &selectedDishName)
{
    if (selectedCandidateRank == 1 && !selectedDishName.isEmpty()) {
        return QStringLiteral("这次吃的就是当时首推。");
    }
    if (selectedCandidateRank > 1 && !topCandidateName.isEmpty() &&
        !selectedDishName.isEmpty()) {
        return QStringLiteral("当时首推是 %1，最后吃了第 %2 候选 %3。")
            .arg(topCandidateName)
            .arg(selectedCandidateRank)
            .arg(selectedDishName);
    }
    return QStringLiteral("这次更适合回看当时的候选排序。");
}

QString comparePriorityHeadlineV2(int selectedCandidateRank)
{
    if (selectedCandidateRank == 1) {
        return QStringLiteral("先确认：首推为什么这次也成立");
    }
    if (selectedCandidateRank > 1) {
        return QStringLiteral("先看分差，再看首推和实际选择的理由");
    }
    return QStringLiteral("先看候选顺序，再决定要不要继续深看");
}

QString compareGuideTextV2(int selectedCandidateRank)
{
    if (selectedCandidateRank == 1) {
        return QStringLiteral("这次已经命中首推，优先确认首推理由现在是否仍然站得住。");
    }
    if (selectedCandidateRank > 1) {
        return QStringLiteral("先看分差，再对照“当时首推”和“实际选择”的理由。");
    }
    return QStringLiteral("先看候选顺序，再判断这条历史记录是否值得继续追踪。");
}

QString previewTextV2(int selectedCandidateRank, const QStringList &candidateNames)
{
    if (selectedCandidateRank > 0) {
        return QStringLiteral("这餐最终命中了当时的第 %1 候选，候选顺序是：%2")
            .arg(selectedCandidateRank)
            .arg(candidateNames.join(QStringLiteral(" / ")));
    }
    if (candidateNames.isEmpty()) {
        return QStringLiteral("未读到当时的候选详情。");
    }
    return QStringLiteral("当时的候选顺序是：%1")
        .arg(candidateNames.join(QStringLiteral(" / ")));
}

QString scoreGapSummary(double topScore, double selectedScore)
{
    if (topScore <= 0.0 || selectedScore <= 0.0) {
        return QString();
    }

    const double gap = std::max(0.0, topScore - selectedScore);
    if (gap <= 0.15) {
        return QStringLiteral("实际选择和首推几乎打平，更像是临场取舍差异。");
    }
    if (gap <= 0.80) {
        return QStringLiteral("实际选择只比首推低 %1 分，属于非常接近的一组候选。")
            .arg(QString::number(gap, 'f', 1));
    }
    if (gap <= 1.80) {
        return QStringLiteral("实际选择比首推低 %1 分，建议优先对照两条理由是否都成立。")
            .arg(QString::number(gap, 'f', 1));
    }
    return QStringLiteral("首推比实际选择高 %1 分，说明当时排序判断差距比较明显。")
        .arg(QString::number(gap, 'f', 1));
}

QString scoreGapTag(double topScore, double candidateScore)
{
    if (topScore <= 0.0 || candidateScore <= 0.0) {
        return QString();
    }

    const double gap = std::max(0.0, topScore - candidateScore);
    if (gap <= 0.15) {
        return QStringLiteral("几乎打平");
    }
    if (gap <= 0.80) {
        return QStringLiteral("只差 %1 分").arg(QString::number(gap, 'f', 1));
    }
    return QStringLiteral("低 %1 分").arg(QString::number(gap, 'f', 1));
}
}

RecommendationRecordRepository::RecommendationRecordRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

bool RecommendationRecordRepository::addRecord(const RecommendationRecord &record,
                                               QString *errorMessage) const
{
    const QString timestamp =
        (record.generatedAt.isValid() ? record.generatedAt : QDateTime::currentDateTime())
            .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "INSERT INTO recommendation_records "
        "(recommended_for_meal_type, generated_at, context_summary, strategy_profile_id, "
        "candidate_1_dish_id, candidate_1_score, candidate_1_reason, "
        "candidate_2_dish_id, candidate_2_score, candidate_2_reason, "
        "candidate_3_dish_id, candidate_3_score, candidate_3_reason, selected_dish_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(record.recommendedForMealType);
    query.addBindValue(timestamp);
    query.addBindValue(record.contextSummary.isEmpty() ? QVariant()
                                                       : QVariant(record.contextSummary));
    query.addBindValue(record.strategyProfileId > 0 ? QVariant(record.strategyProfileId)
                                                    : QVariant());
    query.addBindValue(record.candidate1.dishId > 0 ? QVariant(record.candidate1.dishId)
                                                    : QVariant());
    query.addBindValue(record.candidate1.score > 0.0 ? QVariant(record.candidate1.score)
                                                     : QVariant());
    query.addBindValue(record.candidate1.reason.isEmpty() ? QVariant()
                                                          : QVariant(record.candidate1.reason));
    query.addBindValue(record.candidate2.dishId > 0 ? QVariant(record.candidate2.dishId)
                                                    : QVariant());
    query.addBindValue(record.candidate2.score > 0.0 ? QVariant(record.candidate2.score)
                                                     : QVariant());
    query.addBindValue(record.candidate2.reason.isEmpty() ? QVariant()
                                                          : QVariant(record.candidate2.reason));
    query.addBindValue(record.candidate3.dishId > 0 ? QVariant(record.candidate3.dishId)
                                                    : QVariant());
    query.addBindValue(record.candidate3.score > 0.0 ? QVariant(record.candidate3.score)
                                                     : QVariant());
    query.addBindValue(record.candidate3.reason.isEmpty() ? QVariant()
                                                          : QVariant(record.candidate3.reason));
    query.addBindValue(record.selectedDishId > 0 ? QVariant(record.selectedDishId)
                                                 : QVariant());

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

bool RecommendationRecordRepository::markMatchingRecommendationSelected(
    const QString &mealType,
    const QList<int> &selectedDishIds,
    const QDateTime &selectedAt,
    int mealLogId,
    int *matchedRecordId,
    int *matchedDishId,
    int *matchedCandidateRank,
    QString *errorMessage) const
{
    if (matchedRecordId != nullptr) {
        *matchedRecordId = 0;
    }
    if (matchedDishId != nullptr) {
        *matchedDishId = 0;
    }
    if (matchedCandidateRank != nullptr) {
        *matchedCandidateRank = 0;
    }

    if (mealType.trimmed().isEmpty() || selectedDishIds.isEmpty() || mealLogId <= 0) {
        return true;
    }

    QSet<int> selectedDishSet;
    for (const int dishId : selectedDishIds) {
        if (dishId > 0) {
            selectedDishSet.insert(dishId);
        }
    }

    if (selectedDishSet.isEmpty()) {
        return true;
    }

    const QString selectedAtText =
        (selectedAt.isValid() ? selectedAt : QDateTime::currentDateTime())
            .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, candidate_1_dish_id, candidate_2_dish_id, candidate_3_dish_id, "
        "generated_at "
        "FROM recommendation_records "
        "WHERE recommended_for_meal_type = ? "
        "AND selected_dish_id IS NULL "
        "AND generated_at >= datetime(?, '-18 hours') "
        "AND generated_at <= datetime(?, '+6 hours') "
        "ORDER BY generated_at DESC"));
    query.addBindValue(mealType.trimmed());
    query.addBindValue(selectedAtText);
    query.addBindValue(selectedAtText);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    int resolvedRecordId = 0;
    int resolvedDishId = 0;
    int resolvedCandidateRank = 0;
    QDateTime resolvedGeneratedAt;

    while (query.next()) {
        const int recordId = query.value(0).toInt();
        const QList<int> candidateDishIds = {
            query.value(1).toInt(),
            query.value(2).toInt(),
            query.value(3).toInt()
        };

        int localRank = 0;
        int localDishId = 0;
        for (int index = 0; index < candidateDishIds.size(); ++index) {
            const int candidateDishId = candidateDishIds.at(index);
            if (candidateDishId > 0 && selectedDishSet.contains(candidateDishId)) {
                localRank = index + 1;
                localDishId = candidateDishId;
                break;
            }
        }

        if (localRank <= 0) {
            continue;
        }

        const QDateTime generatedAt =
            QDateTime::fromString(query.value(4).toString(),
                                  QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        const bool shouldReplace =
            resolvedRecordId <= 0 ||
            localRank < resolvedCandidateRank ||
            (localRank == resolvedCandidateRank && generatedAt.isValid() &&
             (!resolvedGeneratedAt.isValid() || generatedAt > resolvedGeneratedAt));
        if (!shouldReplace) {
            continue;
        }

        resolvedRecordId = recordId;
        resolvedDishId = localDishId;
        resolvedCandidateRank = localRank;
        resolvedGeneratedAt = generatedAt;
    }

    if (resolvedRecordId <= 0 || resolvedDishId <= 0 || resolvedCandidateRank <= 0) {
        return true;
    }

    QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
    updateQuery.prepare(QStringLiteral(
        "UPDATE recommendation_records "
        "SET selected_dish_id = ?, selected_meal_log_id = ?, selected_candidate_rank = ? "
        "WHERE id = ?"));
    updateQuery.addBindValue(resolvedDishId);
    updateQuery.addBindValue(mealLogId);
    updateQuery.addBindValue(resolvedCandidateRank);
    updateQuery.addBindValue(resolvedRecordId);

    if (!updateQuery.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = updateQuery.lastError().text();
        }
        return false;
    }

    if (matchedRecordId != nullptr) {
        *matchedRecordId = resolvedRecordId;
    }
    if (matchedDishId != nullptr) {
        *matchedDishId = resolvedDishId;
    }
    if (matchedCandidateRank != nullptr) {
        *matchedCandidateRank = resolvedCandidateRank;
    }

    return true;
}

int RecommendationRecordRepository::loadSelectedRecommendationRecordIdForMealLog(int mealLogId) const
{
    if (mealLogId <= 0) {
        return 0;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id FROM recommendation_records "
        "WHERE selected_meal_log_id = ? "
        "ORDER BY generated_at DESC LIMIT 1"));
    query.addBindValue(mealLogId);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

RecommendationSelectionLink RecommendationRecordRepository::loadSelectionLinkForMealLog(int mealLogId) const
{
    RecommendationSelectionLink link;
    if (mealLogId <= 0) {
        return link;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT rr.id, rr.selected_candidate_rank, rr.selected_dish_id, d.name, rr.generated_at "
        "FROM recommendation_records rr "
        "LEFT JOIN dishes d ON d.id = rr.selected_dish_id "
        "WHERE rr.selected_meal_log_id = ? "
        "ORDER BY rr.generated_at DESC LIMIT 1"));
    query.addBindValue(mealLogId);

    if (!query.exec() || !query.next()) {
        return link;
    }

    link.recommendationRecordId = query.value(0).toInt();
    link.selectedCandidateRank = query.value(1).toInt();
    link.selectedDishId = query.value(2).toInt();
    link.selectedDishName = query.value(3).toString();
    link.generatedAt = QDateTime::fromString(query.value(4).toString(),
                                             QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    return link;
}

QVariantMap RecommendationRecordRepository::loadRecordDetails(int recordId) const
{
    QVariantMap result;
    if (recordId <= 0) {
        return result;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT rr.id, rr.generated_at, rr.context_summary, "
        "rr.selected_candidate_rank, rr.selected_dish_id, "
        "rr.candidate_1_dish_id, d1.name, rr.candidate_1_score, rr.candidate_1_reason, "
        "rr.candidate_2_dish_id, d2.name, rr.candidate_2_score, rr.candidate_2_reason, "
        "rr.candidate_3_dish_id, d3.name, rr.candidate_3_score, rr.candidate_3_reason "
        "FROM recommendation_records rr "
        "LEFT JOIN dishes d1 ON d1.id = rr.candidate_1_dish_id "
        "LEFT JOIN dishes d2 ON d2.id = rr.candidate_2_dish_id "
        "LEFT JOIN dishes d3 ON d3.id = rr.candidate_3_dish_id "
        "WHERE rr.id = ? LIMIT 1"));
    query.addBindValue(recordId);

    if (!query.exec() || !query.next()) {
        return result;
    }

    const int selectedCandidateRank = query.value(3).toInt();
    const int selectedDishId = query.value(4).toInt();
    const QDateTime generatedAt = QDateTime::fromString(
        query.value(1).toString(), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    const QString contextSummary = query.value(2).toString();

    result.insert(QStringLiteral("id"), query.value(0).toInt());
    result.insert(QStringLiteral("generatedAt"),
                  generatedAt.isValid()
                      ? generatedAt.toString(QStringLiteral("yyyy-MM-dd hh:mm"))
                      : QString());
    result.insert(QStringLiteral("contextSummary"), contextSummary);
    result.insert(QStringLiteral("selectedCandidateRank"), selectedCandidateRank);
    result.insert(QStringLiteral("selectedDishId"), selectedDishId);

    QVariantList candidates;
    QStringList candidateNames;
    QString selectedDishName;
    QString selectedReason;
    double selectedScore = 0.0;
    QString topCandidateName;
    QString topCandidateReason;
    double topCandidateScore = 0.0;

    for (int index = 0; index < 3; ++index) {
        const int offset = 5 + index * 4;
        const int dishId = query.value(offset).toInt();
        if (dishId <= 0) {
            continue;
        }

        QVariantMap candidate;
        candidate.insert(QStringLiteral("rank"), index + 1);
        candidate.insert(QStringLiteral("dishId"), dishId);
        candidate.insert(QStringLiteral("dishName"), query.value(offset + 1).toString());
        candidate.insert(QStringLiteral("score"), query.value(offset + 2).toDouble());
        candidate.insert(QStringLiteral("scoreText"),
                         QString::number(query.value(offset + 2).toDouble(), 'f', 1));
        candidate.insert(QStringLiteral("reason"), query.value(offset + 3).toString());
        candidate.insert(QStringLiteral("selected"), selectedCandidateRank == index + 1);
        candidate.insert(QStringLiteral("selectedRank"),
                         selectedCandidateRank == index + 1 ? index + 1 : 0);
        candidate.insert(QStringLiteral("badgeText"),
                         candidateBadgeTextV2(index + 1, false));
        candidates.append(candidate);

        const QString dishName = candidate.value(QStringLiteral("dishName")).toString();
        if (!dishName.isEmpty()) {
            candidateNames.append(dishName);
        }
        if (index == 0) {
            topCandidateName = dishName;
            topCandidateReason = candidate.value(QStringLiteral("reason")).toString();
            topCandidateScore = candidate.value(QStringLiteral("score")).toDouble();
        }
        if (selectedCandidateRank == index + 1) {
            selectedDishName = dishName;
            selectedReason = candidate.value(QStringLiteral("reason")).toString();
            selectedScore = candidate.value(QStringLiteral("score")).toDouble();
        }
    }

    for (int index = 0; index < candidates.size(); ++index) {
        QVariantMap candidate = candidates.at(index).toMap();
        candidate.insert(QStringLiteral("scoreGapTag"),
                         scoreGapTagV2(topCandidateScore,
                                       candidate.value(QStringLiteral("score")).toDouble()));
        candidate.insert(QStringLiteral("badgeText"),
                         candidateBadgeTextV2(
                             candidate.value(QStringLiteral("rank")).toInt(),
                             candidate.value(QStringLiteral("selected")).toBool()));
        candidates[index] = candidate;
    }

    const double selectedScoreGapValue = scoreGapValueV2(topCandidateScore, selectedScore);
    result.insert(QStringLiteral("candidates"), candidates);
    result.insert(QStringLiteral("contextHeadline"), leadingSentence(contextSummary));
    result.insert(QStringLiteral("topCandidateDishName"), topCandidateName);
    result.insert(QStringLiteral("topCandidateScoreText"),
                  topCandidateScore > 0.0
                      ? QString::number(topCandidateScore, 'f', 1)
                      : QString());
    result.insert(QStringLiteral("selectedCandidateDishName"), selectedDishName);
    result.insert(QStringLiteral("selectedCandidateScoreText"),
                  selectedScore > 0.0 ? QString::number(selectedScore, 'f', 1)
                                      : QString());
    result.insert(QStringLiteral("selectionSummary"),
                  selectionSummaryV2(selectedCandidateRank, selectedDishName));
    result.insert(QStringLiteral("comparisonSummary"),
                  comparisonSummaryV2(selectedCandidateRank,
                                      topCandidateName,
                                      selectedDishName));
    result.insert(QStringLiteral("comparePriorityHeadline"),
                  comparePriorityHeadlineV2(selectedCandidateRank));
    result.insert(QStringLiteral("compareGuideText"),
                  compareGuideTextV2(selectedCandidateRank));
    result.insert(QStringLiteral("scoreGapValue"), selectedScoreGapValue);
    result.insert(QStringLiteral("scoreGapSummary"),
                  scoreGapSummaryV2(topCandidateScore, selectedScore));
    result.insert(QStringLiteral("selectedReason"),
                  trimmedPreviewText(selectedReason));
    result.insert(QStringLiteral("topCandidateReason"),
                  trimmedPreviewText(topCandidateReason));
    result.insert(QStringLiteral("previewText"),
                  previewTextV2(selectedCandidateRank, candidateNames));

    return result;
}
