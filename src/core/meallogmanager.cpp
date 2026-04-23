#include "meallogmanager.h"

#include "../data/dishrepository.h"
#include "../data/mealfeedbackrepository.h"
#include "../data/meallogrepository.h"
#include "../data/merchantrepository.h"
#include "../data/recommendationrecordrepository.h"

#include <algorithm>
#include <cmath>
#include <QDateTime>
#include <QHash>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSet>
#include <QVariantMap>

namespace
{
struct RankedDishSignal
{
    int dishId = 0;
    double primaryValue = 0.0;
    double secondaryValue = 0.0;
    int sampleCount = 0;
};

struct DishTrendSample
{
    QDateTime eatenAt;
    double taste = 0.0;
    double repeat = 0.0;
    double comfort = 0.0;
    double focus = 0.0;
    double sleepiness = 0.0;
};

struct MetricAverage
{
    double taste = 0.0;
    double repeat = 0.0;
    double comfort = 0.0;
    double focus = 0.0;
    double sleepiness = 0.0;
};

QString mealTypeDisplayLabel(const QString &mealType)
{
    if (mealType == QStringLiteral("breakfast")) {
        return QStringLiteral("早餐");
    }
    if (mealType == QStringLiteral("lunch")) {
        return QStringLiteral("午餐");
    }
    if (mealType == QStringLiteral("dinner")) {
        return QStringLiteral("晚餐");
    }
    if (mealType == QStringLiteral("snack")) {
        return QStringLiteral("加餐");
    }
    return mealType;
}

QString weightKeyDisplayName(const QString &key)
{
    static const QHash<QString, QString> labels = {
        {QStringLiteral("scene.class_fit"), QStringLiteral("课前适配判断")},
        {QStringLiteral("nutrition.sleepiness_risk_fit"), QStringLiteral("犯困风险惩罚")},
        {QStringLiteral("nutrition.digestive_burden_fit"), QStringLiteral("消化负担惩罚")},
        {QStringLiteral("group.scene_fit"), QStringLiteral("场景适配总权重")},
        {QStringLiteral("group.diversity_fit"), QStringLiteral("多餐补偿总权重")},
        {QStringLiteral("group.preference_fit"), QStringLiteral("偏好信号总权重")},
        {QStringLiteral("preference.taste_rating"), QStringLiteral("口味反馈信号")},
        {QStringLiteral("preference.repeat_willingness"), QStringLiteral("复吃意愿信号")},
        {QStringLiteral("preference.preference_score"), QStringLiteral("综合偏好回传")}
    };
    return labels.value(key, key);
}

QString weightDirectionDisplayLabel(const QString &direction)
{
    if (direction == QStringLiteral("increase")) {
        return QStringLiteral("建议提高");
    }
    if (direction == QStringLiteral("decrease")) {
        return QStringLiteral("建议降低");
    }
    return QStringLiteral("建议复核");
}

QString weightStrengthDisplayLabel(const QString &strength)
{
    if (strength == QStringLiteral("slight")) {
        return QStringLiteral("轻微调整");
    }
    if (strength == QStringLiteral("moderate")) {
        return QStringLiteral("中等调整");
    }
    if (strength == QStringLiteral("strong")) {
        return QStringLiteral("明显调整");
    }
    return strength;
}

QString joinedNonEmptyParts(const QStringList &parts,
                            const QString &separator = QStringLiteral(" · "))
{
    QStringList filtered;
    for (const QString &part : parts) {
        const QString trimmed = part.trimmed();
        if (!trimmed.isEmpty()) {
            filtered.append(trimmed);
        }
    }
    return filtered.join(separator);
}

QString firstReadableField(const QVariantList &items, const QString &fieldName)
{
    for (const QVariant &itemVariant : items) {
        const QVariantMap item = itemVariant.toMap();
        const QString value = item.value(fieldName).toString().trimmed();
        if (!value.isEmpty()) {
            return value;
        }
    }
    return QString();
}

QString compactReadableText(const QString &text, int maxLength = 28)
{
    const QString simplified = text.simplified();
    if (simplified.size() <= maxLength) {
        return simplified;
    }

    return simplified.left(std::max(0, maxLength - 1)) + QStringLiteral("…");
}

QString humanWeightDirectionDisplayLabel(const QString &direction)
{
    if (direction == QStringLiteral("increase")) {
        return QStringLiteral("建议提高");
    }
    if (direction == QStringLiteral("decrease")) {
        return QStringLiteral("建议降低");
    }
    return QStringLiteral("建议复核");
}

QString humanWeightStrengthDisplayLabel(const QString &strength)
{
    if (strength == QStringLiteral("slight")) {
        return QStringLiteral("轻微调整");
    }
    if (strength == QStringLiteral("moderate")) {
        return QStringLiteral("中等调整");
    }
    if (strength == QStringLiteral("strong")) {
        return QStringLiteral("明显调整");
    }
    return strength;
}

QString buildReadableInsightEvidenceSummary(const QVariantList &supportingDishes,
                                            const QVariantList &supportingMeals,
                                            const QVariantList &weightHints)
{
    QStringList parts;
    if (!supportingDishes.isEmpty()) {
        parts.append(QStringLiteral("%1 个代表菜样本").arg(supportingDishes.size()));
    }
    if (!supportingMeals.isEmpty()) {
        parts.append(QStringLiteral("%1 条代表餐次").arg(supportingMeals.size()));
    }
    if (!weightHints.isEmpty()) {
        parts.append(QStringLiteral("%1 条调权建议").arg(weightHints.size()));
    }
    return joinedNonEmptyParts(parts);
}

QString buildReadableInsightQuickLook(const QVariantList &supportingDishes,
                                      const QVariantList &supportingMeals,
                                      const QVariantList &weightHints)
{
    const QString topWeightHint =
        compactReadableText(firstReadableField(weightHints, QStringLiteral("actionText")), 24);
    const QString topDishName =
        compactReadableText(firstReadableField(supportingDishes, QStringLiteral("name")), 18);
    const QString topMealSummary =
        compactReadableText(firstReadableField(supportingMeals, QStringLiteral("dishSummary")), 22);

    if (!weightHints.isEmpty() && !supportingDishes.isEmpty()) {
        return QStringLiteral("先看“%1”，再核对代表菜 %2。")
            .arg(topWeightHint, topDishName);
    }
    if (!weightHints.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看“%1”，再回看代表餐次 %2。")
            .arg(topWeightHint, topMealSummary);
    }
    if (!supportingDishes.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看代表菜 %1，再对照餐次 %2。")
            .arg(topDishName, topMealSummary);
    }
    if (!supportingDishes.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表菜 %1。").arg(topDishName);
    }
    if (!supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表餐次 %1。").arg(topMealSummary);
    }
    return QStringLiteral("先看这条洞察的概括，再决定要不要继续深读。");
}

QString buildReadableInsightPriorityHeadline(const QVariantList &supportingDishes,
                                             const QVariantList &supportingMeals,
                                             const QVariantList &weightHints)
{
    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    if (!topWeightHint.isEmpty()) {
        return QStringLiteral("优先动作：%1").arg(topWeightHint);
    }

    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    if (!topDishName.isEmpty()) {
        return QStringLiteral("先看：%1").arg(topDishName);
    }

    const QString topMealSummary =
        firstReadableField(supportingMeals, QStringLiteral("dishSummary"));
    if (!topMealSummary.isEmpty()) {
        return QStringLiteral("先看这顿：%1").arg(compactReadableText(topMealSummary, 24));
    }

    return QString();
}

QVariantList buildInsightScanSteps(const QVariantList &supportingDishes,
                                   const QVariantList &supportingMeals,
                                   const QVariantList &weightHints)
{
    QVariantList steps;

    auto appendStep = [&steps](const QString &title, const QString &body) {
        if (body.trimmed().isEmpty()) {
            return;
        }

        QVariantMap step;
        step.insert(QStringLiteral("title"), title);
        step.insert(QStringLiteral("body"), body);
        steps.append(step);
    };

    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    const QString topMealSummary =
        compactReadableText(firstReadableField(supportingMeals, QStringLiteral("dishSummary")),
                            24);

    if (!topWeightHint.isEmpty()) {
        appendStep(QStringLiteral("第 1 步：先看动作"),
                   QStringLiteral("先判断这条建议是否值得动手调整：%1。")
                       .arg(topWeightHint));
    }
    if (!topDishName.isEmpty()) {
        appendStep(QStringLiteral("第 %1 步：再看代表菜").arg(steps.size() + 1),
                   QStringLiteral("优先核对 %1 这道菜，确认问题是单点现象还是稳定趋势。")
                       .arg(topDishName));
    }
    if (!topMealSummary.isEmpty()) {
        appendStep(QStringLiteral("第 %1 步：最后回看餐次").arg(steps.size() + 1),
                   QStringLiteral("再回到 %1 这顿，确认当时场景和反馈是否支持这条判断。")
                       .arg(topMealSummary));
    }

    if (steps.isEmpty()) {
        appendStep(QStringLiteral("先看概括"),
                   QStringLiteral("这一条暂时没有可优先展开的样本，先看概括再决定是否深读。"));
    }

    return steps;
}

QString readableMealTypeDisplayLabelV2(const QString &mealType)
{
    if (mealType == QStringLiteral("breakfast")) {
        return QStringLiteral("早餐");
    }
    if (mealType == QStringLiteral("lunch")) {
        return QStringLiteral("午餐");
    }
    if (mealType == QStringLiteral("dinner")) {
        return QStringLiteral("晚餐");
    }
    if (mealType == QStringLiteral("snack")) {
        return QStringLiteral("加餐");
    }
    return mealType;
}

QString readableWeightDirectionDisplayLabelV2(const QString &direction)
{
    if (direction == QStringLiteral("increase")) {
        return QStringLiteral("建议提高");
    }
    if (direction == QStringLiteral("decrease")) {
        return QStringLiteral("建议降低");
    }
    return QStringLiteral("建议复核");
}

QString readableWeightStrengthDisplayLabelV2(const QString &strength)
{
    if (strength == QStringLiteral("slight")) {
        return QStringLiteral("轻微调整");
    }
    if (strength == QStringLiteral("moderate")) {
        return QStringLiteral("中等调整");
    }
    if (strength == QStringLiteral("strong")) {
        return QStringLiteral("明显调整");
    }
    return strength;
}

QString readableWeightKeyDisplayNameV2(const QString &key)
{
    static const QHash<QString, QString> labels = {
        {QStringLiteral("scene.class_fit"), QStringLiteral("课前适配判断")},
        {QStringLiteral("nutrition.sleepiness_risk_fit"), QStringLiteral("犯困风险惩罚")},
        {QStringLiteral("nutrition.digestive_burden_fit"), QStringLiteral("消化负担惩罚")},
        {QStringLiteral("group.scene_fit"), QStringLiteral("场景适配总权重")},
        {QStringLiteral("group.diversity_fit"), QStringLiteral("多餐补偿总权重")},
        {QStringLiteral("group.preference_fit"), QStringLiteral("偏好信号总权重")},
        {QStringLiteral("preference.taste_rating"), QStringLiteral("口味反馈信号")},
        {QStringLiteral("preference.repeat_willingness"), QStringLiteral("复吃意愿信号")},
        {QStringLiteral("preference.preference_score"), QStringLiteral("综合偏好回传")}
    };
    return labels.value(key, key);
}

QString readableWeightHintReasonV2(const QString &key,
                                   const QString &direction,
                                   const QString &strength)
{
    Q_UNUSED(strength);

    if (key == QStringLiteral("scene.class_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("有课压力下的反馈还不够稳，课前适配需要再严格一点。")
                   : QStringLiteral("课前适配目前可能压得过重，可以回头复核。");
    }
    if (key == QStringLiteral("nutrition.sleepiness_risk_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("最近反馈说明犯困问题仍然明显，这条风险约束值得更早生效。")
                   : QStringLiteral("犯困风险近期没有那么突出，可以回头复核这条约束。");
    }
    if (key == QStringLiteral("nutrition.digestive_burden_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("舒适度和负担反馈还在提醒你，偏重的餐需要更早被压住。")
                   : QStringLiteral("消化负担约束可能偏重，可以结合样本再看。");
    }
    if (key == QStringLiteral("group.scene_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("最近命中走势说明场景判断仍然值得给更高权重。")
                   : QStringLiteral("场景适配整体可能偏重，可以回头复核。");
    }
    if (key == QStringLiteral("group.diversity_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("旧偏好和重复选择仍然偏强，多餐补偿可以稍微再上来一点。")
                   : QStringLiteral("多餐补偿目前可能给得偏多，可以回头复核。");
    }
    if (key == QStringLiteral("group.preference_fit")) {
        return direction == QStringLiteral("decrease")
                   ? QStringLiteral("旧偏好可能还在拖慢反馈回传，偏好总权重可以先收一点。")
                   : QStringLiteral("这些稳定偏好值得更快体现在排序里。");
    }
    if (key == QStringLiteral("preference.taste_rating")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("口味反馈已经累积出稳定信号，可以更放心地体现在排序里。")
                   : QStringLiteral("口味反馈当前可能放大过度，可以回头复核。");
    }
    if (key == QStringLiteral("preference.repeat_willingness")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("复吃意愿正在稳定地区分该保留和该降温的菜，值得更快回传。")
                   : QStringLiteral("复吃意愿这条信号当前可能有些过重。");
    }
    if (key == QStringLiteral("preference.preference_score")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("综合偏好回传可以更快反映最近的反馈变化。")
                   : QStringLiteral("综合偏好回传目前可以先放慢一点。");
    }

    return QStringLiteral("这条提示已经出现稳定信号，适合做小步人工调整。");
}

QString readableInsightEvidenceSummaryV2(const QVariantList &supportingDishes,
                                         const QVariantList &supportingMeals,
                                         const QVariantList &weightHints)
{
    QStringList parts;
    if (!supportingDishes.isEmpty()) {
        parts.append(QStringLiteral("%1 个代表菜样本").arg(supportingDishes.size()));
    }
    if (!supportingMeals.isEmpty()) {
        parts.append(QStringLiteral("%1 条代表餐次").arg(supportingMeals.size()));
    }
    if (!weightHints.isEmpty()) {
        parts.append(QStringLiteral("%1 条调权建议").arg(weightHints.size()));
    }
    return joinedNonEmptyParts(parts);
}

QString readableInsightQuickLookV2(const QVariantList &supportingDishes,
                                   const QVariantList &supportingMeals,
                                   const QVariantList &weightHints)
{
    const QString topWeightHint =
        compactReadableText(firstReadableField(weightHints, QStringLiteral("actionText")), 24);
    const QString topDishName =
        compactReadableText(firstReadableField(supportingDishes, QStringLiteral("name")), 18);
    const QString topMealSummary =
        compactReadableText(firstReadableField(supportingMeals, QStringLiteral("dishSummary")), 22);

    if (!weightHints.isEmpty() && !supportingDishes.isEmpty()) {
        return QStringLiteral("先看“%1”，再核对代表菜 %2。")
            .arg(topWeightHint, topDishName);
    }
    if (!weightHints.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看“%1”，再回看代表餐次 %2。")
            .arg(topWeightHint, topMealSummary);
    }
    if (!supportingDishes.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看代表菜 %1，再对照餐次 %2。")
            .arg(topDishName, topMealSummary);
    }
    if (!supportingDishes.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表菜 %1。").arg(topDishName);
    }
    if (!supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表餐次 %1。").arg(topMealSummary);
    }
    return QStringLiteral("先看这条洞察的概括，再决定要不要继续深读。");
}

QString readableInsightPriorityHeadlineV2(const QVariantList &supportingDishes,
                                          const QVariantList &supportingMeals,
                                          const QVariantList &weightHints)
{
    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    if (!topWeightHint.isEmpty()) {
        return QStringLiteral("优先动作：%1").arg(topWeightHint);
    }

    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    if (!topDishName.isEmpty()) {
        return QStringLiteral("先看：%1").arg(topDishName);
    }

    const QString topMealSummary =
        firstReadableField(supportingMeals, QStringLiteral("dishSummary"));
    if (!topMealSummary.isEmpty()) {
        return QStringLiteral("先看这顿：%1").arg(compactReadableText(topMealSummary, 24));
    }

    return QString();
}

QString normalizedInsightTitleV2(const QString &key, const QString &fallback)
{
    static const QHash<QString, QString> titles = {
        {QStringLiteral("feedback_coverage"), QStringLiteral("反馈覆盖情况")},
        {QStringLiteral("recommendation_hits"), QStringLiteral("推荐命中情况")},
        {QStringLiteral("context_split"), QStringLiteral("场景分层对比")},
        {QStringLiteral("ranking_momentum"), QStringLiteral("命中走势")},
        {QStringLiteral("stable_favorites"), QStringLiteral("稳定偏好")},
        {QStringLiteral("sleepiness_watch"), QStringLiteral("犯困观察")},
        {QStringLiteral("low_repeat"), QStringLiteral("复吃意愿偏低")},
        {QStringLiteral("improving_dishes"), QStringLiteral("近期转好")},
        {QStringLiteral("worsening_dishes"), QStringLiteral("近期转弱")},
        {QStringLiteral("weight_adjustment_suggestions"), QStringLiteral("调权方向建议")}
    };
    return titles.value(key, fallback);
}

QString normalizedInsightSummaryV2(const QString &key,
                                   const QString &fallback,
                                   const QVariantList &supportingDishes,
                                   const QVariantList &supportingMeals,
                                   const QVariantList &weightHints)
{
    Q_UNUSED(fallback);

    if (key == QStringLiteral("feedback_coverage")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("最近的反馈样本还在补齐，先把更关键的餐次补反馈。")
                   : QStringLiteral("最近已经有一批餐次可以作为反馈样本，足够继续往下看。");
    }
    if (key == QStringLiteral("recommendation_hits")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("推荐回链样本还不多，暂时先积累。")
                   : QStringLiteral("已经能回看一批有推荐回链的餐次，可以继续看首推是否经常被改选。");
    }
    if (key == QStringLiteral("context_split")) {
        return QStringLiteral("有课和放松场景的餐后表现已经开始分层，值得单独看。");
    }
    if (key == QStringLiteral("ranking_momentum")) {
        return QStringLiteral("最近的 top-1 命中走势已经能看出方向，适合继续跟踪。");
    }
    if (key == QStringLiteral("stable_favorites")) {
        return QStringLiteral("这些菜已经表现出稳定正反馈，可以视作更可靠的偏好。");
    }
    if (key == QStringLiteral("sleepiness_watch")) {
        return QStringLiteral("这些菜的犯困反馈更集中，适合优先盯住。");
    }
    if (key == QStringLiteral("low_repeat")) {
        return QStringLiteral("这些菜不是吃不下，而是吃完后不太想再点。");
    }
    if (key == QStringLiteral("improving_dishes")) {
        return QStringLiteral("这些菜最近在回升，可能逐渐形成新偏好。");
    }
    if (key == QStringLiteral("worsening_dishes")) {
        return QStringLiteral("这些菜最近在走弱，旧偏好可能需要降温。");
    }
    if (key == QStringLiteral("weight_adjustment_suggestions")) {
        return QStringLiteral("已经出现几类稳定模式，足以支持小步手动调权。");
    }

    if (!weightHints.isEmpty()) {
        return QStringLiteral("这条洞察已经给出动作方向，可以先看建议再看样本。");
    }
    if (!supportingDishes.isEmpty() || !supportingMeals.isEmpty()) {
        return QStringLiteral("这条洞察已经有足够样本，适合顺着代表样本往下看。");
    }
    return fallback;
}

QString normalizedInsightDetailV2(const QString &key,
                                  const QString &fallback,
                                  const QVariantList &supportingDishes,
                                  const QVariantList &supportingMeals,
                                  const QVariantList &weightHints)
{
    Q_UNUSED(fallback);

    if (key == QStringLiteral("feedback_coverage")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("优先把最近最关键的餐次补上反馈，后面的判断会更稳。")
                   : QStringLiteral("可以先看这些代表餐次，确认哪些反馈已经够用、哪些还需要补。");
    }
    if (key == QStringLiteral("recommendation_hits")) {
        return QStringLiteral("重点不是看“有没有推荐”，而是看首推是否经常被改成低位候选。");
    }
    if (key == QStringLiteral("context_split")) {
        return QStringLiteral("优先确认问题主要出在有课场景，还是放松场景也同样存在。");
    }
    if (key == QStringLiteral("ranking_momentum")) {
        return QStringLiteral("如果最近命中率在走弱，通常说明排序里某些旧判断已经跟不上了。");
    }
    if (key == QStringLiteral("stable_favorites")) {
        return QStringLiteral("这类菜更适合作为可靠偏好保留，但也要注意别让旧偏好长期霸榜。");
    }
    if (key == QStringLiteral("sleepiness_watch")) {
        return QStringLiteral("优先看这些菜是不是在课前或高压场景里反复带来更差的餐后状态。");
    }
    if (key == QStringLiteral("low_repeat")) {
        return QStringLiteral("这类菜常见的问题不是第一口不好吃，而是吃完之后不想再选。");
    }
    if (key == QStringLiteral("improving_dishes")) {
        return QStringLiteral("先确认这波回升是不是偶然，再决定要不要把它们更快抬上来。");
    }
    if (key == QStringLiteral("worsening_dishes")) {
        return QStringLiteral("先确认这是短期波动还是持续走弱，再决定是否要让它们更快降温。");
    }
    if (key == QStringLiteral("weight_adjustment_suggestions")) {
        return QStringLiteral("先做小步调整，不要一次动太多，继续观察后再决定是否扩大。");
    }

    if (!weightHints.isEmpty()) {
        return QStringLiteral("先看建议动作，再顺着代表样本确认这条判断是不是站得住。");
    }
    if (!supportingDishes.isEmpty() || !supportingMeals.isEmpty()) {
        return QStringLiteral("顺着代表样本往下看，会比逐段读长文案更快。");
    }
    return fallback;
}

QString readableMealTypeDisplayLabelV3(const QString &mealType)
{
    if (mealType == QStringLiteral("breakfast")) {
        return QStringLiteral("早餐");
    }
    if (mealType == QStringLiteral("lunch")) {
        return QStringLiteral("午餐");
    }
    if (mealType == QStringLiteral("dinner")) {
        return QStringLiteral("晚餐");
    }
    if (mealType == QStringLiteral("snack")) {
        return QStringLiteral("加餐");
    }
    return mealType;
}

QString readableWeightDirectionDisplayLabelV3(const QString &direction)
{
    if (direction == QStringLiteral("increase")) {
        return QStringLiteral("建议提高");
    }
    if (direction == QStringLiteral("decrease")) {
        return QStringLiteral("建议降低");
    }
    return QStringLiteral("建议复核");
}

QString readableWeightStrengthDisplayLabelV3(const QString &strength)
{
    if (strength == QStringLiteral("slight")) {
        return QStringLiteral("轻微调整");
    }
    if (strength == QStringLiteral("moderate")) {
        return QStringLiteral("中等调整");
    }
    if (strength == QStringLiteral("strong")) {
        return QStringLiteral("明显调整");
    }
    return strength;
}

QString readableWeightKeyDisplayNameV3(const QString &key)
{
    static const QHash<QString, QString> labels = {
        {QStringLiteral("scene.class_fit"), QStringLiteral("课前适配判断")},
        {QStringLiteral("nutrition.sleepiness_risk_fit"), QStringLiteral("犯困风险惩罚")},
        {QStringLiteral("nutrition.digestive_burden_fit"), QStringLiteral("消化负担惩罚")},
        {QStringLiteral("group.scene_fit"), QStringLiteral("场景适配总权重")},
        {QStringLiteral("group.diversity_fit"), QStringLiteral("多餐补偿总权重")},
        {QStringLiteral("group.preference_fit"), QStringLiteral("偏好信号总权重")},
        {QStringLiteral("preference.taste_rating"), QStringLiteral("口味反馈信号")},
        {QStringLiteral("preference.repeat_willingness"), QStringLiteral("复吃意愿信号")},
        {QStringLiteral("preference.preference_score"), QStringLiteral("综合偏好回传")}
    };
    return labels.value(key, key);
}

QString readableWeightHintReasonV3(const QString &key,
                                   const QString &direction,
                                   const QString &strength)
{
    Q_UNUSED(strength);

    if (key == QStringLiteral("scene.class_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("有课压力场景下的反馈还不够稳，课前适配需要再严格一点。")
                   : QStringLiteral("课前适配目前可能压得偏重，可以回头复核。");
    }
    if (key == QStringLiteral("nutrition.sleepiness_risk_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("最近反馈说明犯困问题仍然明显，这条风险约束值得更早生效。")
                   : QStringLiteral("犯困风险近期没有那么突出，可以回头复核这条约束。");
    }
    if (key == QStringLiteral("nutrition.digestive_burden_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("舒适度和负担反馈还在提醒你，偏重的餐需要更早被压住。")
                   : QStringLiteral("消化负担约束可能偏重，可以结合样本再看。");
    }
    if (key == QStringLiteral("group.scene_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("最近命中走势说明场景判断仍然值得给更高权重。")
                   : QStringLiteral("场景适配整体可能偏重，可以回头复核。");
    }
    if (key == QStringLiteral("group.diversity_fit")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("旧偏好和重复选择仍然偏强，多餐补偿可以稍微再抬一点。")
                   : QStringLiteral("多餐补偿目前可能给得偏多，可以回头复核。");
    }
    if (key == QStringLiteral("group.preference_fit")) {
        return direction == QStringLiteral("decrease")
                   ? QStringLiteral("旧偏好可能还在拖慢反馈回传，偏好总权重可以先收一点。")
                   : QStringLiteral("这些稳定偏好值得更快体现在排序里。");
    }
    if (key == QStringLiteral("preference.taste_rating")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("口味反馈已经累积出稳定信号，可以更放心地体现在排序里。")
                   : QStringLiteral("口味反馈当前可能放大过度，可以回头复核。");
    }
    if (key == QStringLiteral("preference.repeat_willingness")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("复吃意愿正在稳定地区分该保留和该降温的菜，值得更快回传。")
                   : QStringLiteral("复吃意愿这条信号当前可能有些过重。");
    }
    if (key == QStringLiteral("preference.preference_score")) {
        return direction == QStringLiteral("increase")
                   ? QStringLiteral("综合偏好回传可以更快反映最近的反馈变化。")
                   : QStringLiteral("综合偏好回传目前可以先放慢一点。");
    }

    return QStringLiteral("这条提示已经出现稳定信号，适合做小步人工调整。");
}

QString readableInsightEvidenceSummaryV3(const QVariantList &supportingDishes,
                                         const QVariantList &supportingMeals,
                                         const QVariantList &weightHints)
{
    QStringList parts;
    if (!supportingDishes.isEmpty()) {
        parts.append(QStringLiteral("%1 个代表菜样本").arg(supportingDishes.size()));
    }
    if (!supportingMeals.isEmpty()) {
        parts.append(QStringLiteral("%1 条代表餐次").arg(supportingMeals.size()));
    }
    if (!weightHints.isEmpty()) {
        parts.append(QStringLiteral("%1 条调权建议").arg(weightHints.size()));
    }
    return joinedNonEmptyParts(parts);
}

QString readableInsightQuickLookV3(const QVariantList &supportingDishes,
                                   const QVariantList &supportingMeals,
                                   const QVariantList &weightHints)
{
    const QString topWeightHint =
        compactReadableText(firstReadableField(weightHints, QStringLiteral("actionText")), 24);
    const QString topDishName =
        compactReadableText(firstReadableField(supportingDishes, QStringLiteral("name")), 18);
    const QString topMealSummary =
        compactReadableText(firstReadableField(supportingMeals, QStringLiteral("dishSummary")), 22);

    if (!weightHints.isEmpty() && !supportingDishes.isEmpty()) {
        return QStringLiteral("先看“%1”，再核对代表菜 %2。")
            .arg(topWeightHint, topDishName);
    }
    if (!weightHints.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看“%1”，再回看代表餐次 %2。")
            .arg(topWeightHint, topMealSummary);
    }
    if (!supportingDishes.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看代表菜 %1，再对照餐次 %2。")
            .arg(topDishName, topMealSummary);
    }
    if (!supportingDishes.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表菜 %1。").arg(topDishName);
    }
    if (!supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表餐次 %1。").arg(topMealSummary);
    }
    return QStringLiteral("先看这条洞察的概括，再决定要不要继续细看。");
}

QString readableInsightPriorityHeadlineV3(const QVariantList &supportingDishes,
                                          const QVariantList &supportingMeals,
                                          const QVariantList &weightHints)
{
    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    if (!topWeightHint.isEmpty()) {
        return QStringLiteral("优先动作：%1").arg(topWeightHint);
    }

    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    if (!topDishName.isEmpty()) {
        return QStringLiteral("先看：%1").arg(topDishName);
    }

    const QString topMealSummary =
        firstReadableField(supportingMeals, QStringLiteral("dishSummary"));
    if (!topMealSummary.isEmpty()) {
        return QStringLiteral("先看这餐：%1").arg(compactReadableText(topMealSummary, 24));
    }

    return QString();
}

QVariantList buildInsightScanStepsV3(const QVariantList &supportingDishes,
                                     const QVariantList &supportingMeals,
                                     const QVariantList &weightHints)
{
    QVariantList steps;

    auto appendStep = [&steps](const QString &title, const QString &body) {
        if (body.trimmed().isEmpty()) {
            return;
        }

        QVariantMap step;
        step.insert(QStringLiteral("title"), title);
        step.insert(QStringLiteral("body"), body);
        steps.append(step);
    };

    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    const QString topMealSummary =
        compactReadableText(firstReadableField(supportingMeals, QStringLiteral("dishSummary")),
                            24);

    if (!topWeightHint.isEmpty()) {
        appendStep(QStringLiteral("第 1 步：先看动作"),
                   QStringLiteral("先判断这条建议是否值得动手调整：%1。")
                       .arg(topWeightHint));
    }
    if (!topDishName.isEmpty()) {
        appendStep(QStringLiteral("第 %1 步：再看代表菜").arg(steps.size() + 1),
                   QStringLiteral("优先核对 %1 这道菜，确认问题是单点现象还是稳定趋势。")
                       .arg(topDishName));
    }
    if (!topMealSummary.isEmpty()) {
        appendStep(QStringLiteral("第 %1 步：最后回看餐次").arg(steps.size() + 1),
                   QStringLiteral("再回到 %1 这餐，确认当时场景和反馈是否支持这条判断。")
                       .arg(topMealSummary));
    }

    if (steps.isEmpty()) {
        appendStep(QStringLiteral("先看概括"),
                   QStringLiteral("这一条暂时没有可优先展开的样本，先看概括再决定是否深读。"));
    }

    return steps;
}

QString normalizedInsightTitleV3(const QString &key, const QString &fallback)
{
    static const QHash<QString, QString> titles = {
        {QStringLiteral("feedback_coverage"), QStringLiteral("反馈覆盖情况")},
        {QStringLiteral("recommendation_hits"), QStringLiteral("推荐命中情况")},
        {QStringLiteral("context_split"), QStringLiteral("场景分层对比")},
        {QStringLiteral("ranking_momentum"), QStringLiteral("命中走势")},
        {QStringLiteral("stable_favorites"), QStringLiteral("稳定偏好")},
        {QStringLiteral("sleepiness_watch"), QStringLiteral("犯困观察")},
        {QStringLiteral("low_repeat"), QStringLiteral("复吃意愿偏低")},
        {QStringLiteral("improving_dishes"), QStringLiteral("近期转好")},
        {QStringLiteral("worsening_dishes"), QStringLiteral("近期转弱")},
        {QStringLiteral("weight_adjustment_suggestions"), QStringLiteral("调权方向建议")}
    };
    return titles.value(key, fallback);
}

QString normalizedInsightSummaryV3(const QString &key,
                                   const QString &fallback,
                                   const QVariantList &supportingDishes,
                                   const QVariantList &supportingMeals,
                                   const QVariantList &weightHints)
{
    Q_UNUSED(fallback);

    if (key == QStringLiteral("feedback_coverage")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("最近反馈还没补齐，先补关键餐次。")
                   : QStringLiteral("先看最该补反馈的代表性餐次。");
    }
    if (key == QStringLiteral("recommendation_hits")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("回链样本还不够，先继续积累。")
                   : QStringLiteral("先看命中过推荐的代表性餐次。");
    }
    if (key == QStringLiteral("context_split")) {
        return QStringLiteral("先分清问题更偏有课还是放松场景。");
    }
    if (key == QStringLiteral("ranking_momentum")) {
        return QStringLiteral("先看 top-1 命中率最近是在上升还是回落。");
    }
    if (key == QStringLiteral("stable_favorites")) {
        return QStringLiteral("这些菜正在形成稳定偏好。");
    }
    if (key == QStringLiteral("sleepiness_watch")) {
        return QStringLiteral("这些菜的犯困反馈更值得优先看。");
    }
    if (key == QStringLiteral("low_repeat")) {
        return QStringLiteral("这些菜能吃完，但不太想再点。");
    }
    if (key == QStringLiteral("improving_dishes")) {
        return QStringLiteral("这些菜最近反馈在转好。");
    }
    if (key == QStringLiteral("worsening_dishes")) {
        return QStringLiteral("这些菜最近反馈在转弱。");
    }
    if (key == QStringLiteral("weight_adjustment_suggestions")) {
        return QStringLiteral("已经出现可支持小步调权的模式。");
    }

    if (!weightHints.isEmpty()) {
        return QStringLiteral("先看调权建议，再核对样本。");
    }
    if (!supportingDishes.isEmpty() || !supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在前面的代表性样本。");
    }
    return fallback;
}

QString normalizedInsightDetailV3(const QString &key,
                                  const QString &fallback,
                                  const QVariantList &supportingDishes,
                                  const QVariantList &supportingMeals,
                                  const QVariantList &weightHints)
{
    Q_UNUSED(fallback);

    if (key == QStringLiteral("feedback_coverage")) {
        return supportingMeals.isEmpty()
                   ? QStringLiteral("最近关键餐次的反馈还没补齐，先补最近几餐，后面的判断才会更稳。")
                   : QStringLiteral("先补排在前面的代表性餐次反馈，再回头看整体覆盖率会更清楚。");
    }
    if (key == QStringLiteral("recommendation_hits")) {
        return QStringLiteral("先看哪些回链餐次命中了 top-1 / top-2 / top-3，再判断当前排序是轻微偏移还是已经开始失真。");
    }
    if (key == QStringLiteral("context_split")) {
        return QStringLiteral("先对比有课和放松场景的犯困、专注和舒适差异，确认问题是场景性偏差还是普遍偏差。");
    }
    if (key == QStringLiteral("ranking_momentum")) {
        return QStringLiteral("先对比最近窗口和更早窗口的 top-1 命中率，判断当前排序是在收敛还是开始跑偏。");
    }
    if (key == QStringLiteral("stable_favorites")) {
        return QStringLiteral("先看这些高口味、高复吃的样本，再判断哪些偏好已经稳定到值得更快体现在排序里。");
    }
    if (key == QStringLiteral("sleepiness_watch")) {
        return QStringLiteral("先看这些高犯困样本和对应餐次，确认是否需要更早压低相关候选。");
    }
    if (key == QStringLiteral("low_repeat")) {
        return QStringLiteral("先看吃完后不太想再点的样本，判断是口味降温，还是综合偏好回传还不够快。");
    }
    if (key == QStringLiteral("improving_dishes")) {
        return QStringLiteral("先看近期明显转好的样本，确认它们是否已经值得被更快抬高。");
    }
    if (key == QStringLiteral("worsening_dishes")) {
        return QStringLiteral("先看近期转弱的样本，确认是否需要更快把这些菜往下压。");
    }
    if (key == QStringLiteral("weight_adjustment_suggestions")) {
        return QStringLiteral("先从排在前面的调权建议入手，再用代表性菜样本和餐次样本核对这条建议站不站得住。");
    }

    if (!weightHints.isEmpty()) {
        return QStringLiteral("先看调权建议，再顺着代表性样本核对理由。");
    }
    if (!supportingDishes.isEmpty() || !supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在前面的代表性样本，再决定要不要展开细看。");
    }
    return fallback;
}

QString representativeMealSupportPriorityLabelV3(int matchedDishCount,
                                                 int linkedRank,
                                                 bool feedbackSaved,
                                                 int feedbackMetricCount,
                                                 bool feedbackLinkedToRecommendation)
{
    if (matchedDishCount >= 2 && feedbackLinkedToRecommendation) {
        return QStringLiteral("多菜命中 + 推荐已回链");
    }
    if (matchedDishCount >= 2) {
        return QStringLiteral("覆盖 %1 个相关菜样本")
            .arg(matchedDishCount);
    }
    if (feedbackLinkedToRecommendation) {
        return QStringLiteral("反馈已回链推荐");
    }
    if (linkedRank > 0 && feedbackMetricCount >= 4) {
        return QStringLiteral("命中 top-%1 + 反馈完整")
            .arg(linkedRank);
    }
    if (feedbackMetricCount >= 4) {
        return QStringLiteral("反馈维度完整");
    }
    if (linkedRank > 0) {
        return QStringLiteral("可回看 top-%1 命中")
            .arg(linkedRank);
    }
    if (feedbackSaved) {
        return QStringLiteral("已保存反馈");
    }
    return QStringLiteral("最近且值得补反馈");
}

QString dishSupportPriorityLabelV3(int linkedFeedbackMealCount,
                                   int linkedRecommendationMealCount,
                                   int recentSampleCount,
                                   int sampleCount)
{
    if (linkedFeedbackMealCount > 0) {
        return QStringLiteral("已回链反馈 + 推荐命中");
    }
    if (linkedRecommendationMealCount > 0 && recentSampleCount >= 2) {
        return QStringLiteral("近期样本多，且可回看推荐排序");
    }
    if (sampleCount >= 4) {
        return QStringLiteral("样本较多，趋势更稳");
    }
    return QStringLiteral("近期样本，适合先看");
}

QString dishSupportQuickNoteV3(const QString &priorityLabel,
                               int recentSampleCount,
                               int sampleCount,
                               int linkedRecommendationMealCount)
{
    return joinedNonEmptyParts(
        {priorityLabel,
         QStringLiteral("近期 %1 次，累计 %2 次")
             .arg(recentSampleCount)
             .arg(sampleCount),
         linkedRecommendationMealCount > 0
             ? QStringLiteral("%1 餐可直接回看推荐排序")
                   .arg(linkedRecommendationMealCount)
             : QString()});
}

QString buildInsightEvidenceSummary(const QVariantList &supportingDishes,
                                    const QVariantList &supportingMeals,
                                    const QVariantList &weightHints)
{
    QStringList parts;
    if (!supportingDishes.isEmpty()) {
        parts.append(QStringLiteral("%1 个代表菜样本").arg(supportingDishes.size()));
    }
    if (!supportingMeals.isEmpty()) {
        parts.append(QStringLiteral("%1 条代表餐次").arg(supportingMeals.size()));
    }
    if (!weightHints.isEmpty()) {
        parts.append(QStringLiteral("%1 条调权建议").arg(weightHints.size()));
    }
    return joinedNonEmptyParts(parts);
}

QString buildInsightQuickLook(const QVariantList &supportingDishes,
                              const QVariantList &supportingMeals,
                              const QVariantList &weightHints)
{
    if (!weightHints.isEmpty() && !supportingDishes.isEmpty()) {
        return QStringLiteral("先看调权建议，再核对排在最前面的代表菜样本。");
    }
    if (!weightHints.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看调权建议，再回看最有代表性的餐次。");
    }
    if (!supportingDishes.isEmpty() && !supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表菜样本，再对照对应餐次。");
    }
    if (!supportingDishes.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表菜样本。");
    }
    if (!supportingMeals.isEmpty()) {
        return QStringLiteral("先看排在最前面的代表餐次。");
    }
    return QStringLiteral("先看这条洞察的概要结论。");
}

QString buildInsightPriorityHeadline(const QVariantList &supportingDishes,
                                     const QVariantList &supportingMeals,
                                     const QVariantList &weightHints)
{
    const QString topWeightHint =
        firstReadableField(weightHints, QStringLiteral("actionText"));
    if (!topWeightHint.isEmpty()) {
        return QStringLiteral("优先动作：%1").arg(topWeightHint);
    }

    const QString topDishName =
        firstReadableField(supportingDishes, QStringLiteral("name"));
    if (!topDishName.isEmpty()) {
        return QStringLiteral("先看：%1").arg(topDishName);
    }

    const QString topMealSummary =
        firstReadableField(supportingMeals, QStringLiteral("dishSummary"));
    if (!topMealSummary.isEmpty()) {
        return QStringLiteral("先看这餐：%1").arg(topMealSummary);
    }

    return QString();
}

int candidateRankPriority(int rank)
{
    if (rank == 1) {
        return 20;
    }
    if (rank == 2) {
        return 14;
    }
    if (rank == 3) {
        return 10;
    }
    return 0;
}

int compareRichnessPriority(const QVariantMap &item)
{
    int priority = 0;
    if (!item.value(QStringLiteral("recommendationComparePriorityHeadline"))
             .toString()
             .trimmed()
             .isEmpty()) {
        priority += 3;
    }
    if (!item.value(QStringLiteral("recommendationScoreGapSummary"))
             .toString()
             .trimmed()
             .isEmpty()) {
        priority += 2;
    }
    if (!item.value(QStringLiteral("recommendationTopCandidateReason"))
             .toString()
             .trimmed()
             .isEmpty()) {
        priority += 2;
    }
    if (!item.value(QStringLiteral("recommendationSelectedReason"))
             .toString()
             .trimmed()
             .isEmpty()) {
        priority += 2;
    }
    if (!item.value(QStringLiteral("recommendationCandidates")).toList().isEmpty()) {
        priority += 1;
    }
    return priority;
}

int overlapDishCount(const QVariantMap &mealMap, const QSet<int> &focusDishIds)
{
    if (focusDishIds.isEmpty()) {
        return 0;
    }

    int overlapCount = 0;
    const QVariantList dishIds = mealMap.value(QStringLiteral("dishIds")).toList();
    for (const QVariant &dishIdVariant : dishIds) {
        if (focusDishIds.contains(dishIdVariant.toInt())) {
            overlapCount += 1;
        }
    }
    return overlapCount;
}

QVariantMap enrichMealSupport(QVariantMap mealMap, const QSet<int> &focusDishIds)
{
    const int matchedDishCount = overlapDishCount(mealMap, focusDishIds);
    const bool feedbackSaved = mealMap.value(QStringLiteral("feedbackSaved")).toBool();
    const int linkedRank =
        mealMap.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt();
    const bool linkedRecommendation =
        mealMap.value(QStringLiteral("linkedRecommendationRecordId")).toInt() > 0;

    int supportPriorityScore = matchedDishCount * 50;
    if (feedbackSaved) {
        supportPriorityScore += 18;
    }
    if (linkedRecommendation) {
        supportPriorityScore += 6 + candidateRankPriority(linkedRank);
    }
    if (mealMap.value(QStringLiteral("hasClassAfterMeal")).toBool()) {
        supportPriorityScore += 2;
    }

    QString priorityLabel;
    if (matchedDishCount >= 2) {
        priorityLabel =
            QStringLiteral("同时覆盖 %1 个相关菜样本，代表性更强").arg(matchedDishCount);
    } else if (linkedRank > 0) {
        priorityLabel =
            QStringLiteral("这餐命中过往推荐 top-%1，便于回看排序是否合理").arg(linkedRank);
    } else if (feedbackSaved) {
        priorityLabel = QStringLiteral("已保存完整反馈，适合优先回看");
    } else {
        priorityLabel = QStringLiteral("最近的相关样本，可补更多反馈");
    }
    mealMap.insert(QStringLiteral("supportMatchedDishCount"), matchedDishCount);
    mealMap.insert(QStringLiteral("supportPriorityScore"), supportPriorityScore);
    mealMap.insert(QStringLiteral("supportPriorityLabel"), priorityLabel);
    return mealMap;
}

QVariantList sortSupportMeals(QList<QVariantMap> meals, int limit = 6)
{
    std::sort(meals.begin(), meals.end(),
              [](const QVariantMap &left, const QVariantMap &right) {
                  const int leftPriority =
                      left.value(QStringLiteral("supportPriorityScore")).toInt();
                  const int rightPriority =
                      right.value(QStringLiteral("supportPriorityScore")).toInt();
                  if (leftPriority != rightPriority) {
                      return leftPriority > rightPriority;
                  }

                  const int leftMatchedDishCount =
                      left.value(QStringLiteral("supportMatchedDishCount")).toInt();
                  const int rightMatchedDishCount =
                      right.value(QStringLiteral("supportMatchedDishCount")).toInt();
                  if (leftMatchedDishCount != rightMatchedDishCount) {
                      return leftMatchedDishCount > rightMatchedDishCount;
                  }

                  const int leftRankPriority = candidateRankPriority(
                      left.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt());
                  const int rightRankPriority = candidateRankPriority(
                      right.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt());
                  if (leftRankPriority != rightRankPriority) {
                      return leftRankPriority > rightRankPriority;
                  }

                  const bool leftFeedbackSaved =
                      left.value(QStringLiteral("feedbackSaved")).toBool();
                  const bool rightFeedbackSaved =
                      right.value(QStringLiteral("feedbackSaved")).toBool();
                  if (leftFeedbackSaved != rightFeedbackSaved) {
                      return leftFeedbackSaved;
                  }

                  const qint64 leftTimestamp =
                      left.value(QStringLiteral("sortTimestamp")).toLongLong();
                  const qint64 rightTimestamp =
                      right.value(QStringLiteral("sortTimestamp")).toLongLong();
                  if (leftTimestamp != rightTimestamp) {
                      return leftTimestamp > rightTimestamp;
                  }

                  return left.value(QStringLiteral("id")).toInt() >
                         right.value(QStringLiteral("id")).toInt();
              });

    QVariantList result;
    const int cappedLimit =
        limit > 0 ? std::min(limit, static_cast<int>(meals.size()))
                  : static_cast<int>(meals.size());
    for (int index = 0; index < cappedLimit; ++index) {
        result.append(meals.at(index));
    }
    return result;
}

QVariantList sortSupportDishes(QList<QVariantMap> dishes, int limit = 3)
{
    std::sort(dishes.begin(), dishes.end(),
              [](const QVariantMap &left, const QVariantMap &right) {
                  const double leftPriority =
                      left.value(QStringLiteral("supportPriorityScore")).toDouble();
                  const double rightPriority =
                      right.value(QStringLiteral("supportPriorityScore")).toDouble();
                  if (!qFuzzyCompare(leftPriority + 1.0, rightPriority + 1.0)) {
                      return leftPriority > rightPriority;
                  }

                  const int leftRecentSamples =
                      left.value(QStringLiteral("recentSampleCount")).toInt();
                  const int rightRecentSamples =
                      right.value(QStringLiteral("recentSampleCount")).toInt();
                  if (leftRecentSamples != rightRecentSamples) {
                      return leftRecentSamples > rightRecentSamples;
                  }

                  const int leftSampleCount =
                      left.value(QStringLiteral("sampleCount")).toInt();
                  const int rightSampleCount =
                      right.value(QStringLiteral("sampleCount")).toInt();
                  if (leftSampleCount != rightSampleCount) {
                      return leftSampleCount > rightSampleCount;
                  }

                  const double leftWeight =
                      left.value(QStringLiteral("effectiveSampleWeightValue")).toDouble();
                  const double rightWeight =
                      right.value(QStringLiteral("effectiveSampleWeightValue")).toDouble();
                  if (!qFuzzyCompare(leftWeight + 1.0, rightWeight + 1.0)) {
                      return leftWeight > rightWeight;
                  }

                  return left.value(QStringLiteral("name")).toString() <
                         right.value(QStringLiteral("name")).toString();
              });

    QVariantList result;
    const int cappedLimit =
        limit > 0 ? std::min(limit, static_cast<int>(dishes.size()))
                  : static_cast<int>(dishes.size());
    for (int index = 0; index < cappedLimit; ++index) {
        result.append(dishes.at(index));
    }
    return result;
}

QVariantMap enrichRepresentativeMealSupport(QVariantMap mealMap,
                                            const QSet<int> &focusDishIds)
{
    const int matchedDishCount = overlapDishCount(mealMap, focusDishIds);
    const bool feedbackSaved = mealMap.value(QStringLiteral("feedbackSaved")).toBool();
    const int linkedRank =
        mealMap.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt();
    const bool linkedRecommendation =
        mealMap.value(QStringLiteral("linkedRecommendationRecordId")).toInt() > 0;
    const bool feedbackLinkedToRecommendation =
        mealMap.value(QStringLiteral("feedbackRecommendationRecordId")).toInt() > 0;
    const bool hasRecommendationInspect =
        !mealMap.value(QStringLiteral("recommendationPreviewText")).toString().trimmed().isEmpty();
    const int dishCount = mealMap.value(QStringLiteral("dishIds")).toList().size();

    int feedbackMetricCount = 0;
    if (mealMap.value(QStringLiteral("feedbackTasteRating")).toInt() > 0) {
        feedbackMetricCount += 1;
    }
    if (mealMap.value(QStringLiteral("feedbackRepeatWillingness")).toInt() > 0) {
        feedbackMetricCount += 1;
    }
    if (mealMap.value(QStringLiteral("feedbackFullnessLevel")).toInt() > 0) {
        feedbackMetricCount += 1;
    }
    if (mealMap.value(QStringLiteral("feedbackSleepinessLevel")).toInt() > 0) {
        feedbackMetricCount += 1;
    }
    if (mealMap.value(QStringLiteral("feedbackComfortLevel")).toInt() > 0) {
        feedbackMetricCount += 1;
    }
    if (mealMap.value(QStringLiteral("feedbackFocusImpactLevel")).toInt() > 0) {
        feedbackMetricCount += 1;
    }

    int supportPriorityScore = matchedDishCount * 50;
    if (feedbackSaved) {
        supportPriorityScore += 12;
    }
    supportPriorityScore += feedbackMetricCount * 4;
    if (linkedRecommendation) {
        supportPriorityScore += 6 + candidateRankPriority(linkedRank);
    }
    if (feedbackLinkedToRecommendation) {
        supportPriorityScore += 14;
    }
    if (hasRecommendationInspect) {
        supportPriorityScore += 6;
    }
    if (dishCount > 1) {
        supportPriorityScore += std::min(dishCount, 4) * 2;
    }
    if (mealMap.value(QStringLiteral("hasClassAfterMeal")).toBool()) {
        supportPriorityScore += 2;
    }

    mealMap.insert(QStringLiteral("supportMatchedDishCount"), matchedDishCount);
    mealMap.insert(QStringLiteral("supportPriorityScore"), supportPriorityScore);
    mealMap.insert(QStringLiteral("supportPriorityLabel"),
                   representativeMealSupportPriorityLabelV3(
                       matchedDishCount,
                       linkedRank,
                       feedbackSaved,
                       feedbackMetricCount,
                       feedbackLinkedToRecommendation));
    mealMap.insert(QStringLiteral("supportFeedbackMetricCount"), feedbackMetricCount);
    mealMap.insert(QStringLiteral("supportFeedbackLinked"), feedbackLinkedToRecommendation);
    mealMap.insert(QStringLiteral("supportHasRecommendationInspect"),
                   hasRecommendationInspect);
    mealMap.insert(QStringLiteral("supportDishCount"), dishCount);
    return mealMap;
}

QVariantList sortSupportMealsV2(QList<QVariantMap> meals, int limit = 6)
{
    std::sort(meals.begin(), meals.end(),
              [](const QVariantMap &left, const QVariantMap &right) {
                  const int leftPriority =
                      left.value(QStringLiteral("supportPriorityScore")).toInt();
                  const int rightPriority =
                      right.value(QStringLiteral("supportPriorityScore")).toInt();
                  if (leftPriority != rightPriority) {
                      return leftPriority > rightPriority;
                  }

                  const int leftMatchedDishCount =
                      left.value(QStringLiteral("supportMatchedDishCount")).toInt();
                  const int rightMatchedDishCount =
                      right.value(QStringLiteral("supportMatchedDishCount")).toInt();
                  if (leftMatchedDishCount != rightMatchedDishCount) {
                      return leftMatchedDishCount > rightMatchedDishCount;
                  }

                  const bool leftFeedbackLinked =
                      left.value(QStringLiteral("supportFeedbackLinked")).toBool();
                  const bool rightFeedbackLinked =
                      right.value(QStringLiteral("supportFeedbackLinked")).toBool();
                  if (leftFeedbackLinked != rightFeedbackLinked) {
                      return leftFeedbackLinked;
                  }

                  const int leftFeedbackMetricCount =
                      left.value(QStringLiteral("supportFeedbackMetricCount")).toInt();
                  const int rightFeedbackMetricCount =
                      right.value(QStringLiteral("supportFeedbackMetricCount")).toInt();
                  if (leftFeedbackMetricCount != rightFeedbackMetricCount) {
                      return leftFeedbackMetricCount > rightFeedbackMetricCount;
                  }

                  const bool leftHasInspect =
                      left.value(QStringLiteral("supportHasRecommendationInspect")).toBool();
                  const bool rightHasInspect =
                      right.value(QStringLiteral("supportHasRecommendationInspect")).toBool();
                  if (leftHasInspect != rightHasInspect) {
                      return leftHasInspect;
                  }

                  const int leftComparePriority = compareRichnessPriority(left);
                  const int rightComparePriority = compareRichnessPriority(right);
                  if (leftComparePriority != rightComparePriority) {
                      return leftComparePriority > rightComparePriority;
                  }

                  const int leftRankPriority = candidateRankPriority(
                      left.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt());
                  const int rightRankPriority = candidateRankPriority(
                      right.value(QStringLiteral("linkedRecommendationCandidateRank")).toInt());
                  if (leftRankPriority != rightRankPriority) {
                      return leftRankPriority > rightRankPriority;
                  }

                  const QVariant leftScoreGapVariant =
                      left.value(QStringLiteral("recommendationScoreGapValue"));
                  const QVariant rightScoreGapVariant =
                      right.value(QStringLiteral("recommendationScoreGapValue"));
                  const double leftScoreGap =
                      leftScoreGapVariant.isValid() ? leftScoreGapVariant.toDouble() : -1.0;
                  const double rightScoreGap =
                      rightScoreGapVariant.isValid() ? rightScoreGapVariant.toDouble() : -1.0;
                  const bool leftHasScoreGap = leftScoreGap >= 0.0;
                  const bool rightHasScoreGap = rightScoreGap >= 0.0;
                  if (leftHasScoreGap != rightHasScoreGap) {
                      return leftHasScoreGap;
                  }
                  if (leftHasScoreGap && rightHasScoreGap &&
                      !qFuzzyCompare(leftScoreGap + 1.0, rightScoreGap + 1.0)) {
                      return leftScoreGap < rightScoreGap;
                  }

                  const bool leftFeedbackSaved =
                      left.value(QStringLiteral("feedbackSaved")).toBool();
                  const bool rightFeedbackSaved =
                      right.value(QStringLiteral("feedbackSaved")).toBool();
                  if (leftFeedbackSaved != rightFeedbackSaved) {
                      return leftFeedbackSaved;
                  }

                  const int leftDishCount =
                      left.value(QStringLiteral("supportDishCount")).toInt();
                  const int rightDishCount =
                      right.value(QStringLiteral("supportDishCount")).toInt();
                  if (leftDishCount != rightDishCount) {
                      return leftDishCount > rightDishCount;
                  }

                  const QString leftDishSummary =
                      left.value(QStringLiteral("dishSummary")).toString();
                  const QString rightDishSummary =
                      right.value(QStringLiteral("dishSummary")).toString();
                  if (leftDishSummary != rightDishSummary) {
                      return leftDishSummary < rightDishSummary;
                  }

                  const qint64 leftTimestamp =
                      left.value(QStringLiteral("sortTimestamp")).toLongLong();
                  const qint64 rightTimestamp =
                      right.value(QStringLiteral("sortTimestamp")).toLongLong();
                  if (leftTimestamp != rightTimestamp) {
                      return leftTimestamp > rightTimestamp;
                  }

                  return left.value(QStringLiteral("id")).toInt() >
                         right.value(QStringLiteral("id")).toInt();
              });

    QVariantList result;
    const int cappedLimit =
        limit > 0 ? std::min(limit, static_cast<int>(meals.size()))
                  : static_cast<int>(meals.size());
    for (int index = 0; index < cappedLimit; ++index) {
        result.append(meals.at(index));
    }
    return result;
}

QVariantList sortSupportDishesV2(QList<QVariantMap> dishes, int limit = 3)
{
    std::sort(dishes.begin(), dishes.end(),
              [](const QVariantMap &left, const QVariantMap &right) {
                  const double leftPriority =
                      left.value(QStringLiteral("supportPriorityScore")).toDouble();
                  const double rightPriority =
                      right.value(QStringLiteral("supportPriorityScore")).toDouble();
                  if (!qFuzzyCompare(leftPriority + 1.0, rightPriority + 1.0)) {
                      return leftPriority > rightPriority;
                  }

                  const int leftFeedbackLinkedCount =
                      left.value(QStringLiteral("linkedFeedbackMealCount")).toInt();
                  const int rightFeedbackLinkedCount =
                      right.value(QStringLiteral("linkedFeedbackMealCount")).toInt();
                  if (leftFeedbackLinkedCount != rightFeedbackLinkedCount) {
                      return leftFeedbackLinkedCount > rightFeedbackLinkedCount;
                  }

                  const int leftLinkedMealCount =
                      left.value(QStringLiteral("linkedRecommendationMealCount")).toInt();
                  const int rightLinkedMealCount =
                      right.value(QStringLiteral("linkedRecommendationMealCount")).toInt();
                  if (leftLinkedMealCount != rightLinkedMealCount) {
                      return leftLinkedMealCount > rightLinkedMealCount;
                  }

                  const int leftRepresentativeFeedback =
                      left.value(QStringLiteral("representativeFeedbackMetricCount")).toInt();
                  const int rightRepresentativeFeedback =
                      right.value(QStringLiteral("representativeFeedbackMetricCount")).toInt();
                  if (leftRepresentativeFeedback != rightRepresentativeFeedback) {
                      return leftRepresentativeFeedback > rightRepresentativeFeedback;
                  }

                  const int leftTopMealPriority =
                      left.value(QStringLiteral("topSupportingMealPriorityScore")).toInt();
                  const int rightTopMealPriority =
                      right.value(QStringLiteral("topSupportingMealPriorityScore")).toInt();
                  if (leftTopMealPriority != rightTopMealPriority) {
                      return leftTopMealPriority > rightTopMealPriority;
                  }

                  const int leftTopMealCompare =
                      left.value(QStringLiteral("topSupportingMealCompareRichness")).toInt();
                  const int rightTopMealCompare =
                      right.value(QStringLiteral("topSupportingMealCompareRichness")).toInt();
                  if (leftTopMealCompare != rightTopMealCompare) {
                      return leftTopMealCompare > rightTopMealCompare;
                  }

                  const QVariant leftTopMealScoreGapVariant =
                      left.value(QStringLiteral("topSupportingMealScoreGapValue"));
                  const QVariant rightTopMealScoreGapVariant =
                      right.value(QStringLiteral("topSupportingMealScoreGapValue"));
                  const double leftTopMealScoreGap =
                      leftTopMealScoreGapVariant.isValid()
                          ? leftTopMealScoreGapVariant.toDouble()
                          : -1.0;
                  const double rightTopMealScoreGap =
                      rightTopMealScoreGapVariant.isValid()
                          ? rightTopMealScoreGapVariant.toDouble()
                          : -1.0;
                  const bool leftHasTopScoreGap = leftTopMealScoreGap >= 0.0;
                  const bool rightHasTopScoreGap = rightTopMealScoreGap >= 0.0;
                  if (leftHasTopScoreGap != rightHasTopScoreGap) {
                      return leftHasTopScoreGap;
                  }
                  if (leftHasTopScoreGap && rightHasTopScoreGap &&
                      !qFuzzyCompare(leftTopMealScoreGap + 1.0,
                                     rightTopMealScoreGap + 1.0)) {
                      return leftTopMealScoreGap < rightTopMealScoreGap;
                  }

                  const int leftRecentSamples =
                      left.value(QStringLiteral("recentSampleCount")).toInt();
                  const int rightRecentSamples =
                      right.value(QStringLiteral("recentSampleCount")).toInt();
                  if (leftRecentSamples != rightRecentSamples) {
                      return leftRecentSamples > rightRecentSamples;
                  }

                  const int leftSampleCount =
                      left.value(QStringLiteral("sampleCount")).toInt();
                  const int rightSampleCount =
                      right.value(QStringLiteral("sampleCount")).toInt();
                  if (leftSampleCount != rightSampleCount) {
                      return leftSampleCount > rightSampleCount;
                  }

                  const double leftWeight =
                      left.value(QStringLiteral("effectiveSampleWeightValue")).toDouble();
                  const double rightWeight =
                      right.value(QStringLiteral("effectiveSampleWeightValue")).toDouble();
                  if (!qFuzzyCompare(leftWeight + 1.0, rightWeight + 1.0)) {
                      return leftWeight > rightWeight;
                  }

                  return left.value(QStringLiteral("name")).toString() <
                         right.value(QStringLiteral("name")).toString();
              });

    QVariantList result;
    const int cappedLimit =
        limit > 0 ? std::min(limit, static_cast<int>(dishes.size()))
                  : static_cast<int>(dishes.size());
    for (int index = 0; index < cappedLimit; ++index) {
        result.append(dishes.at(index));
    }
    return result;
}

template <typename CountMap>
QVariantList topItemsFromCounts(const QVariantList &items,
                                const CountMap &counts,
                                const QString &idKey)
{
    QList<QPair<int, int>> rankedIds;
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        rankedIds.append(qMakePair(it.key(), it.value()));
    }

    std::sort(rankedIds.begin(), rankedIds.end(),
              [](const QPair<int, int> &left, const QPair<int, int> &right) {
                  if (left.second != right.second) {
                      return left.second > right.second;
                  }
                  return left.first < right.first;
              });

    QVariantList topItems;
    for (const QPair<int, int> &pair : rankedIds) {
        for (const QVariant &itemVariant : items) {
            const QVariantMap item = itemVariant.toMap();
            if (item.value(idKey).toInt() == pair.first) {
                topItems.append(item);
                break;
            }
        }

        if (topItems.size() >= 6) {
            break;
        }
    }

    return topItems;
}

int searchPriority(const QVariantMap &item,
                   const QString &searchText,
                   const QStringList &orderedKeys)
{
    if (searchText.isEmpty()) {
        return 0;
    }

    int bestPriority = 9999;
    for (int keyIndex = 0; keyIndex < orderedKeys.size(); ++keyIndex) {
        const QString value =
            item.value(orderedKeys.at(keyIndex)).toString().trimmed().toLower();
        if (value.isEmpty()) {
            continue;
        }

        if (value == searchText) {
            bestPriority = std::min(bestPriority, keyIndex * 10);
            continue;
        }
        if (value.startsWith(searchText)) {
            bestPriority = std::min(bestPriority, keyIndex * 10 + 1);
            continue;
        }
        if (value.contains(searchText)) {
            bestPriority = std::min(bestPriority, keyIndex * 10 + 2);
        }
    }

    return bestPriority;
}

void sortAvailableDishMatches(QVariantList *items, const QString &searchText)
{
    const QString loweredSearch = searchText.trimmed().toLower();
    std::sort(items->begin(), items->end(),
              [&loweredSearch](const QVariant &leftVariant,
                               const QVariant &rightVariant) {
                  const QVariantMap left = leftVariant.toMap();
                  const QVariantMap right = rightVariant.toMap();

                  if (!loweredSearch.isEmpty()) {
                      const int leftPriority = searchPriority(
                          left, loweredSearch,
                          {QStringLiteral("name"), QStringLiteral("merchantName"),
                           QStringLiteral("category"),
                           QStringLiteral("sleepinessRiskLevel"),
                           QStringLiteral("notes")});
                      const int rightPriority = searchPriority(
                          right, loweredSearch,
                          {QStringLiteral("name"), QStringLiteral("merchantName"),
                           QStringLiteral("category"),
                           QStringLiteral("sleepinessRiskLevel"),
                           QStringLiteral("notes")});
                      if (leftPriority != rightPriority) {
                          return leftPriority < rightPriority;
                      }
                  }

                  const int leftRecentUse =
                      left.value(QStringLiteral("recentUseCount")).toInt();
                  const int rightRecentUse =
                      right.value(QStringLiteral("recentUseCount")).toInt();
                  if (leftRecentUse != rightRecentUse) {
                      return leftRecentUse > rightRecentUse;
                  }

                  const QString leftName =
                      left.value(QStringLiteral("name")).toString().trimmed().toLower();
                  const QString rightName =
                      right.value(QStringLiteral("name")).toString().trimmed().toLower();
                  if (leftName != rightName) {
                      return leftName < rightName;
                  }

                  const QString leftMerchant =
                      left.value(QStringLiteral("merchantName")).toString().trimmed().toLower();
                  const QString rightMerchant =
                      right.value(QStringLiteral("merchantName")).toString().trimmed().toLower();
                  if (leftMerchant != rightMerchant) {
                      return leftMerchant < rightMerchant;
                  }

                  return left.value(QStringLiteral("id")).toInt() <
                         right.value(QStringLiteral("id")).toInt();
              });
}

QVariantMap buildWeightHint(const QString &key,
                           const QString &label,
                           const QString &direction,
                           const QString &strength,
                           const QString &reason)
{
    QVariantMap hint;
    hint.insert(QStringLiteral("key"), key);
    const QString displayLabel =
        label == key || label.trimmed().isEmpty() ? readableWeightKeyDisplayNameV3(key)
                                                  : label;
    hint.insert(QStringLiteral("label"), displayLabel);
    hint.insert(QStringLiteral("displayLabel"), displayLabel);
    hint.insert(QStringLiteral("direction"), direction);
    hint.insert(QStringLiteral("directionLabel"),
                readableWeightDirectionDisplayLabelV3(direction));
    hint.insert(QStringLiteral("strength"), strength);
    hint.insert(QStringLiteral("strengthLabel"),
                readableWeightStrengthDisplayLabelV3(strength));
    hint.insert(QStringLiteral("actionText"),
                QStringLiteral("%1 %2")
                    .arg(readableWeightDirectionDisplayLabelV3(direction), displayLabel));
    hint.insert(QStringLiteral("reason"),
                readableWeightHintReasonV3(key, direction, strength));
    return hint;
}

QVariantMap buildInsight(const QString &key,
                         const QString &title,
                         const QString &summary,
                         const QString &detail,
                         const QString &tone,
                         const QVariantList &supportingDishes = {},
                         const QVariantList &supportingMeals = {},
                         const QVariantList &weightHints = {})
{
    QVariantMap item;
    const QVariantList scanSteps =
        buildInsightScanStepsV3(supportingDishes, supportingMeals, weightHints);
    item.insert(QStringLiteral("key"), key);
    item.insert(QStringLiteral("title"), normalizedInsightTitleV3(key, title));
    item.insert(QStringLiteral("summary"),
                normalizedInsightSummaryV3(key, summary, supportingDishes,
                                           supportingMeals, weightHints));
    item.insert(QStringLiteral("detail"),
                normalizedInsightDetailV3(key, detail, supportingDishes,
                                          supportingMeals, weightHints));
    item.insert(QStringLiteral("tone"), tone);
    item.insert(QStringLiteral("supportingDishes"), supportingDishes);
    item.insert(QStringLiteral("supportingMeals"), supportingMeals);
    item.insert(QStringLiteral("weightHints"), weightHints);
    item.insert(QStringLiteral("scanSteps"), scanSteps);
    if (!scanSteps.isEmpty()) {
        const QVariantMap firstScanStep = scanSteps.first().toMap();
        item.insert(QStringLiteral("firstScanStepTitle"),
                    firstScanStep.value(QStringLiteral("title")));
        item.insert(QStringLiteral("firstScanStepBody"),
                    firstScanStep.value(QStringLiteral("body")));
        item.insert(QStringLiteral("remainingScanStepCount"),
                    std::max(0, static_cast<int>(scanSteps.size()) - 1));
    } else {
        item.insert(QStringLiteral("firstScanStepTitle"), QString());
        item.insert(QStringLiteral("firstScanStepBody"), QString());
        item.insert(QStringLiteral("remainingScanStepCount"), 0);
    }
    item.insert(QStringLiteral("dishCount"), supportingDishes.size());
    item.insert(QStringLiteral("mealCount"), supportingMeals.size());
    item.insert(QStringLiteral("weightHintCount"), weightHints.size());
    item.insert(QStringLiteral("evidenceSummary"),
                readableInsightEvidenceSummaryV3(supportingDishes,
                                                supportingMeals,
                                                weightHints));
    item.insert(QStringLiteral("quickLookText"),
                readableInsightQuickLookV3(supportingDishes, supportingMeals,
                                           weightHints));
    item.insert(QStringLiteral("priorityHeadline"),
                readableInsightPriorityHeadlineV3(supportingDishes,
                                                 supportingMeals,
                                                 weightHints));
    return item;
}

void appendInsight(QVariantList *insights, const QVariantMap &item)
{
    if (insights == nullptr || item.isEmpty()) {
        return;
    }

    insights->append(item);
}

QVariantList toVariantList(QList<QVariantMap> items, int limit = -1)
{
    std::sort(items.begin(), items.end(),
              [](const QVariantMap &left, const QVariantMap &right) {
                  return left.value(QStringLiteral("sortTimestamp")).toLongLong() >
                         right.value(QStringLiteral("sortTimestamp")).toLongLong();
              });

    QVariantList result;
    const int itemCount = static_cast<int>(items.size());
    const int cappedLimit = limit > 0 ? std::min(limit, itemCount) : itemCount;
    for (int index = 0; index < cappedLimit; ++index) {
        result.append(items.at(index));
    }

    return result;
}

QVariantList gatherMealSupportList(const QList<int> &dishIds,
                                   const QHash<int, QList<QVariantMap>> &mealSamplesByDishId,
                                   int limit = 6)
{
    QList<QVariantMap> supportingMeals;
    QSet<int> seenMealIds;
    QSet<int> focusDishIds;
    for (const int dishId : dishIds) {
        if (dishId > 0) {
            focusDishIds.insert(dishId);
        }
    }

    for (const int dishId : dishIds) {
        const QList<QVariantMap> mealSamples = mealSamplesByDishId.value(dishId);
        for (const QVariantMap &mealSample : mealSamples) {
            const int mealId = mealSample.value(QStringLiteral("id")).toInt();
            if (mealId <= 0 || seenMealIds.contains(mealId)) {
                continue;
            }

            seenMealIds.insert(mealId);
            supportingMeals.append(
                enrichRepresentativeMealSupport(mealSample, focusDishIds));
        }
    }

    return sortSupportMealsV2(supportingMeals, limit);
}

QVariantList buildDishSupportList(const QList<RankedDishSignal> &dishSignals,
                                  const QHash<int, DishFeedbackAggregate> &feedbackByDishId,
                                  const QHash<int, Dish> &dishById,
                                  const QHash<int, QString> &merchantNameById,
                                  const QHash<int, QList<QVariantMap>> &mealSamplesByDishId,
                                  const QString &primaryLabel,
                                  const QString &secondaryLabel,
                                  int limit = 3)
{
    QList<QVariantMap> rankedDishes;

    for (const RankedDishSignal &signal : dishSignals) {
        if (!dishById.contains(signal.dishId)) {
            continue;
        }

        const Dish dish = dishById.value(signal.dishId);
        const DishFeedbackAggregate aggregate = feedbackByDishId.value(signal.dishId);

        QVariantMap item;
        item.insert(QStringLiteral("id"), dish.id);
        item.insert(QStringLiteral("name"), dish.name);
        item.insert(QStringLiteral("merchantName"),
                    merchantNameById.value(dish.merchantId));
        item.insert(QStringLiteral("category"), dish.category);
        item.insert(QStringLiteral("price"), dish.price);
        item.insert(QStringLiteral("sampleCount"),
                    aggregate.sampleCount > 0 ? aggregate.sampleCount : signal.sampleCount);
        item.insert(QStringLiteral("recentSampleCount"), aggregate.recentSampleCount);
        item.insert(QStringLiteral("effectiveSampleWeightValue"),
                    aggregate.effectiveSampleWeight);
        item.insert(QStringLiteral("effectiveSampleWeight"),
                    QString::number(aggregate.effectiveSampleWeight, 'f', 1));
        item.insert(QStringLiteral("avgTasteRating"),
                    QString::number(aggregate.avgTasteRating, 'f', 1));
        item.insert(QStringLiteral("avgRepeatWillingness"),
                    QString::number(aggregate.avgRepeatWillingness, 'f', 1));
        item.insert(QStringLiteral("avgSleepinessLevel"),
                    QString::number(aggregate.avgSleepinessLevel, 'f', 1));
        item.insert(QStringLiteral("avgComfortLevel"),
                    QString::number(aggregate.avgComfortLevel, 'f', 1));
        item.insert(QStringLiteral("avgFocusImpactLevel"),
                    QString::number(aggregate.avgFocusImpactLevel, 'f', 1));
        item.insert(QStringLiteral("wouldEatAgainRate"),
                    QString::number(aggregate.wouldEatAgainRate * 100.0, 'f', 0));
        item.insert(QStringLiteral("signalPrimaryValue"),
                    QString::number(signal.primaryValue, 'f', 1));
        item.insert(QStringLiteral("signalSecondaryValue"),
                    QString::number(signal.secondaryValue, 'f', 1));
        item.insert(QStringLiteral("signalSummary"),
                    QStringLiteral("%1 %2 | %3 %4 | 样本 %5")
                        .arg(primaryLabel)
                        .arg(QString::number(signal.primaryValue, 'f', 1))
                        .arg(secondaryLabel)
                        .arg(QString::number(signal.secondaryValue, 'f', 1))
                        .arg(item.value(QStringLiteral("sampleCount")).toInt()));
        const double supportPriorityScore =
            std::abs(signal.primaryValue) * 12.0 +
            std::abs(signal.secondaryValue) * 6.0 +
            item.value(QStringLiteral("sampleCount")).toInt() * 8.0 +
            aggregate.recentSampleCount * 6.0 +
            aggregate.effectiveSampleWeight * 4.0;
        item.insert(QStringLiteral("supportPriorityScore"), supportPriorityScore);
        item.insert(QStringLiteral("supportPriorityLabel"),
                    aggregate.recentSampleCount >= 2
                        ? QStringLiteral("近期样本更密集，代表当前状态")
                        : item.value(QStringLiteral("sampleCount")).toInt() >= 4
                            ? QStringLiteral("累计样本较多，结论更稳定")
                            : QStringLiteral("信号明显，可先查看"));
        item.insert(QStringLiteral("supportQuickNote"),
                    joinedNonEmptyParts(
                        {item.value(QStringLiteral("supportPriorityLabel")).toString(),
                         QStringLiteral("近期 %1 次，累计 %2 次")
                             .arg(aggregate.recentSampleCount)
                             .arg(item.value(QStringLiteral("sampleCount")).toInt())}));
        item.insert(QStringLiteral("supportingMeals"),
                    gatherMealSupportList({signal.dishId}, mealSamplesByDishId, 4));
        rankedDishes.append(item);
    }

    return sortSupportDishes(rankedDishes, limit);
}

QVariantList buildDishSupportListV2(const QList<RankedDishSignal> &dishSignals,
                                    const QHash<int, DishFeedbackAggregate> &feedbackByDishId,
                                    const QHash<int, Dish> &dishById,
                                    const QHash<int, QString> &merchantNameById,
                                    const QHash<int, QList<QVariantMap>> &mealSamplesByDishId,
                                    const QString &primaryLabel,
                                    const QString &secondaryLabel,
                                    int limit = 3)
{
    QList<QVariantMap> rankedDishes;

    for (const RankedDishSignal &signal : dishSignals) {
        if (!dishById.contains(signal.dishId)) {
            continue;
        }

        const Dish dish = dishById.value(signal.dishId);
        const DishFeedbackAggregate aggregate = feedbackByDishId.value(signal.dishId);
        const QVariantList supportingMeals =
            gatherMealSupportList({signal.dishId}, mealSamplesByDishId, 4);

        int linkedRecommendationMealCount = 0;
        int linkedFeedbackMealCount = 0;
        int representativeFeedbackMetricCount = 0;
        int topSupportingMealPriorityScore = 0;
        int topSupportingMealCompareRichness = 0;
        double topSupportingMealScoreGapValue = -1.0;
        for (const QVariant &mealVariant : supportingMeals) {
            const QVariantMap meal = mealVariant.toMap();
            if (meal.value(QStringLiteral("linkedRecommendationRecordId")).toInt() > 0) {
                linkedRecommendationMealCount += 1;
            }
            if (meal.value(QStringLiteral("supportFeedbackLinked")).toBool()) {
                linkedFeedbackMealCount += 1;
            }
            representativeFeedbackMetricCount = std::max(
                representativeFeedbackMetricCount,
                meal.value(QStringLiteral("supportFeedbackMetricCount")).toInt());
            topSupportingMealPriorityScore = std::max(
                topSupportingMealPriorityScore,
                meal.value(QStringLiteral("supportPriorityScore")).toInt());
            topSupportingMealCompareRichness = std::max(
                topSupportingMealCompareRichness,
                compareRichnessPriority(meal));
            const QVariant mealScoreGapVariant =
                meal.value(QStringLiteral("recommendationScoreGapValue"));
            const double mealScoreGap =
                mealScoreGapVariant.isValid() ? mealScoreGapVariant.toDouble() : -1.0;
            if (mealScoreGap >= 0.0 &&
                (topSupportingMealScoreGapValue < 0.0 ||
                 mealScoreGap < topSupportingMealScoreGapValue)) {
                topSupportingMealScoreGapValue = mealScoreGap;
            }
        }

        QVariantMap item;
        item.insert(QStringLiteral("id"), dish.id);
        item.insert(QStringLiteral("name"), dish.name);
        item.insert(QStringLiteral("merchantName"),
                    merchantNameById.value(dish.merchantId));
        item.insert(QStringLiteral("category"), dish.category);
        item.insert(QStringLiteral("price"), dish.price);
        item.insert(QStringLiteral("sampleCount"),
                    aggregate.sampleCount > 0 ? aggregate.sampleCount : signal.sampleCount);
        item.insert(QStringLiteral("recentSampleCount"), aggregate.recentSampleCount);
        item.insert(QStringLiteral("effectiveSampleWeightValue"),
                    aggregate.effectiveSampleWeight);
        item.insert(QStringLiteral("effectiveSampleWeight"),
                    QString::number(aggregate.effectiveSampleWeight, 'f', 1));
        item.insert(QStringLiteral("avgTasteRating"),
                    QString::number(aggregate.avgTasteRating, 'f', 1));
        item.insert(QStringLiteral("avgRepeatWillingness"),
                    QString::number(aggregate.avgRepeatWillingness, 'f', 1));
        item.insert(QStringLiteral("avgSleepinessLevel"),
                    QString::number(aggregate.avgSleepinessLevel, 'f', 1));
        item.insert(QStringLiteral("avgComfortLevel"),
                    QString::number(aggregate.avgComfortLevel, 'f', 1));
        item.insert(QStringLiteral("avgFocusImpactLevel"),
                    QString::number(aggregate.avgFocusImpactLevel, 'f', 1));
        item.insert(QStringLiteral("wouldEatAgainRate"),
                    QString::number(aggregate.wouldEatAgainRate * 100.0, 'f', 0));
        item.insert(QStringLiteral("signalPrimaryValue"),
                    QString::number(signal.primaryValue, 'f', 1));
        item.insert(QStringLiteral("signalSecondaryValue"),
                    QString::number(signal.secondaryValue, 'f', 1));
        item.insert(QStringLiteral("signalSummary"),
                    QStringLiteral("%1 %2 | %3 %4 | 样本 %5")
                        .arg(primaryLabel)
                        .arg(QString::number(signal.primaryValue, 'f', 1))
                        .arg(secondaryLabel)
                        .arg(QString::number(signal.secondaryValue, 'f', 1))
                        .arg(item.value(QStringLiteral("sampleCount")).toInt()));

        const double supportPriorityScore =
            std::abs(signal.primaryValue) * 12.0 +
            std::abs(signal.secondaryValue) * 6.0 +
            item.value(QStringLiteral("sampleCount")).toInt() * 8.0 +
            aggregate.recentSampleCount * 6.0 +
            aggregate.effectiveSampleWeight * 4.0 +
            linkedRecommendationMealCount * 7.0 +
            linkedFeedbackMealCount * 9.0 +
            representativeFeedbackMetricCount * 2.5;
        item.insert(QStringLiteral("supportPriorityScore"), supportPriorityScore);
        item.insert(QStringLiteral("linkedRecommendationMealCount"),
                    linkedRecommendationMealCount);
        item.insert(QStringLiteral("linkedFeedbackMealCount"),
                    linkedFeedbackMealCount);
        item.insert(QStringLiteral("representativeFeedbackMetricCount"),
                    representativeFeedbackMetricCount);
        item.insert(QStringLiteral("topSupportingMealPriorityScore"),
                    topSupportingMealPriorityScore);
        item.insert(QStringLiteral("topSupportingMealCompareRichness"),
                    topSupportingMealCompareRichness);
        item.insert(QStringLiteral("topSupportingMealScoreGapValue"),
                    topSupportingMealScoreGapValue);
        const QString supportPriorityLabel =
            dishSupportPriorityLabelV3(linkedFeedbackMealCount,
                                       linkedRecommendationMealCount,
                                       aggregate.recentSampleCount,
                                       item.value(QStringLiteral("sampleCount")).toInt());
        item.insert(QStringLiteral("supportPriorityLabel"), supportPriorityLabel);
        item.insert(QStringLiteral("supportQuickNote"),
                    dishSupportQuickNoteV3(
                        supportPriorityLabel,
                        aggregate.recentSampleCount,
                        item.value(QStringLiteral("sampleCount")).toInt(),
                        linkedRecommendationMealCount));
        item.insert(QStringLiteral("supportingMeals"), supportingMeals);
        rankedDishes.append(item);
    }

    return sortSupportDishesV2(rankedDishes, limit);
}

QString formatDishSignalList(const QList<RankedDishSignal> &dishSignals,
                             const QHash<int, QString> &dishNameById,
                             const QString &primaryLabel,
                             const QString &secondaryLabel)
{
    QStringList parts;
    for (const RankedDishSignal &signal : dishSignals) {
        const QString dishName =
            dishNameById.value(signal.dishId,
                               QStringLiteral("Dish #%1").arg(signal.dishId));
        parts.append(QStringLiteral("%1 %2%3 / %4%5（样本 %6）")
                         .arg(dishName)
                         .arg(QString::number(signal.primaryValue, 'f', 1))
                         .arg(primaryLabel)
                         .arg(QString::number(signal.secondaryValue, 'f', 1))
                         .arg(secondaryLabel)
                         .arg(signal.sampleCount));
    }
    return parts.join(QStringLiteral(" | "));
}

double desirabilityScore(const MetricAverage &average)
{
    const double rested = std::max(0.0, 6.0 - average.sleepiness);
    return average.taste * 0.26 +
           average.repeat * 0.24 +
           average.comfort * 0.18 +
           average.focus * 0.17 +
           rested * 0.15;
}

MetricAverage averageTrendSamples(const QList<DishTrendSample> &samples)
{
    MetricAverage average;
    if (samples.isEmpty()) {
        return average;
    }

    for (const DishTrendSample &sample : samples) {
        average.taste += sample.taste;
        average.repeat += sample.repeat;
        average.comfort += sample.comfort;
        average.focus += sample.focus;
        average.sleepiness += sample.sleepiness;
    }

    const double divisor = static_cast<double>(samples.size());
    average.taste /= divisor;
    average.repeat /= divisor;
    average.comfort /= divisor;
    average.focus /= divisor;
    average.sleepiness /= divisor;
    return average;
}
}

MealLogManager::MealLogManager(const DatabaseManager &databaseManager, QObject *parent)
    : QObject(parent),
      m_databaseManager(databaseManager)
{
    refreshState();
}

QVariantList MealLogManager::availableDishes() const
{
    return m_availableDishes;
}

QVariantList MealLogManager::filteredAvailableDishes() const
{
    return m_filteredAvailableDishes;
}

QVariantList MealLogManager::frequentDishes() const
{
    return m_frequentDishes;
}

QVariantList MealLogManager::selectedDishes() const
{
    return m_selectedDishes;
}

QVariantList MealLogManager::recentMeals() const
{
    return m_recentMeals;
}

QVariantList MealLogManager::feedbackInsights() const
{
    return m_feedbackInsights;
}

QString MealLogManager::dishSearch() const
{
    return m_dishSearch;
}

int MealLogManager::editingMealLogId() const
{
    return m_editingMealLogId;
}

QString MealLogManager::lastError() const
{
    return m_lastError;
}

void MealLogManager::reload()
{
    refreshState();
    emit stateChanged();
}

void MealLogManager::setDishSearch(const QString &searchText)
{
    m_dishSearch = searchText.trimmed();
    refreshFilteredState();
    emit stateChanged();
}

bool MealLogManager::addSelectedDish(int dishId,
                                     double portionRatio,
                                     const QString &customNotes)
{
    if (dishId <= 0) {
        m_lastError = QStringLiteral("Please select one dish.");
        emit stateChanged();
        return false;
    }

    QVariantMap dishToAdd;
    for (const QVariant &dishVariant : m_availableDishes) {
        const QVariantMap dishMap = dishVariant.toMap();
        if (dishMap.value(QStringLiteral("id")).toInt() == dishId) {
            dishToAdd = dishMap;
            break;
        }
    }

    if (dishToAdd.isEmpty()) {
        m_lastError = QStringLiteral("Dish not found.");
        emit stateChanged();
        return false;
    }

    const double normalizedRatio = portionRatio > 0.0
                                       ? portionRatio
                                       : dishToAdd.value(QStringLiteral("mealImpactWeight")).toDouble();
    dishToAdd.insert(QStringLiteral("portionRatio"), normalizedRatio > 0.0 ? normalizedRatio : 1.0);
    dishToAdd.insert(QStringLiteral("customNotes"), customNotes.trimmed());
    m_selectedDishes.append(dishToAdd);

    m_lastError.clear();
    emit stateChanged();
    return true;
}

void MealLogManager::removeSelectedDish(int index)
{
    if (index < 0 || index >= m_selectedDishes.size()) {
        return;
    }

    m_selectedDishes.removeAt(index);
    emit stateChanged();
}

void MealLogManager::clearSelection()
{
    m_selectedDishes.clear();
    m_lastError.clear();
    emit stateChanged();
}

QVariantMap MealLogManager::loadMealLogForEdit(int mealLogId)
{
    QVariantMap result;
    if (mealLogId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid meal log.");
        emit stateChanged();
        return result;
    }

    MealLogRepository repository(m_databaseManager.connectionName());
    const MealLog mealLog = repository.loadMealLog(mealLogId);
    if (mealLog.id <= 0) {
        m_lastError = QStringLiteral("Meal log not found.");
        emit stateChanged();
        return result;
    }

    const QList<MealLogDishItem> dishItems = repository.loadMealLogDishItems(mealLogId);
    m_selectedDishes.clear();
    for (const MealLogDishItem &item : dishItems) {
        for (const QVariant &dishVariant : m_availableDishes) {
            QVariantMap dishMap = dishVariant.toMap();
            if (dishMap.value(QStringLiteral("id")).toInt() == item.dishId) {
                dishMap.insert(QStringLiteral("portionRatio"), item.portionRatio);
                dishMap.insert(QStringLiteral("customNotes"), item.customNotes);
                m_selectedDishes.append(dishMap);
                break;
            }
        }
    }

    m_editingMealLogId = mealLogId;
    m_lastError.clear();

    result.insert(QStringLiteral("id"), mealLog.id);
    result.insert(QStringLiteral("mealType"), mealLog.mealType);
    result.insert(QStringLiteral("eatenAt"), mealLog.eatenAt.toString(Qt::ISODate));
    result.insert(QStringLiteral("weekday"), mealLog.weekday);
    result.insert(QStringLiteral("hasClassAfterMeal"), mealLog.hasClassAfterMeal);
    result.insert(QStringLiteral("minutesUntilNextClass"), mealLog.minutesUntilNextClass);
    result.insert(QStringLiteral("locationType"), mealLog.locationType);
    result.insert(QStringLiteral("diningMode"), mealLog.diningMode);
    result.insert(QStringLiteral("totalPrice"), mealLog.totalPrice);
    result.insert(QStringLiteral("totalEatTimeMinutes"), mealLog.totalEatTimeMinutes);
    result.insert(QStringLiteral("preMealHungerLevel"), mealLog.preMealHungerLevel);
    result.insert(QStringLiteral("preMealEnergyLevel"), mealLog.preMealEnergyLevel);
    result.insert(QStringLiteral("moodTag"), mealLog.moodTag);
    result.insert(QStringLiteral("notes"), mealLog.notes);
    emit stateChanged();
    return result;
}

void MealLogManager::cancelEditingMealLog()
{
    m_editingMealLogId = 0;
    m_selectedDishes.clear();
    m_lastError.clear();
    emit stateChanged();
}

bool MealLogManager::deleteMealLog(int mealLogId)
{
    if (mealLogId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid meal log.");
        emit stateChanged();
        return false;
    }

    MealLogRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.deleteMealLog(mealLogId, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to delete meal log.")
                          : errorMessage;
        emit stateChanged();
        return false;
    }

    if (m_editingMealLogId == mealLogId) {
        m_editingMealLogId = 0;
        m_selectedDishes.clear();
    }

    m_lastError.clear();
    reload();
    return true;
}

QVariantMap MealLogManager::loadMealFeedback(int mealLogId)
{
    QVariantMap result;
    if (mealLogId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid meal log.");
        emit stateChanged();
        return result;
    }

    MealFeedbackRepository repository(m_databaseManager.connectionName());
    const MealFeedback feedback = repository.loadFeedbackForMealLog(mealLogId);
    if (feedback.id <= 0) {
        m_lastError.clear();
        emit stateChanged();
        return result;
    }

    result.insert(QStringLiteral("mealLogId"), feedback.mealLogId);
    result.insert(QStringLiteral("recommendationRecordId"),
                  feedback.recommendationRecordId);
    result.insert(QStringLiteral("fullnessLevel"), feedback.fullnessLevel);
    result.insert(QStringLiteral("sleepinessLevel"), feedback.sleepinessLevel);
    result.insert(QStringLiteral("comfortLevel"), feedback.comfortLevel);
    result.insert(QStringLiteral("focusImpactLevel"), feedback.focusImpactLevel);
    result.insert(QStringLiteral("tasteRating"), feedback.tasteRating);
    result.insert(QStringLiteral("repeatWillingness"), feedback.repeatWillingness);
    result.insert(QStringLiteral("wouldEatAgain"), feedback.wouldEatAgain);
    result.insert(QStringLiteral("freeTextFeedback"), feedback.freeTextFeedback);
    m_lastError.clear();
    emit stateChanged();
    return result;
}

bool MealLogManager::saveMealFeedback(int mealLogId,
                                      int fullnessLevel,
                                      int sleepinessLevel,
                                      int comfortLevel,
                                      int focusImpactLevel,
                                      bool wouldEatAgain,
                                      int tasteRating,
                                      int repeatWillingness,
                                      const QString &freeTextFeedback)
{
    if (mealLogId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid meal log.");
        emit stateChanged();
        return false;
    }

    const auto isValidScore = [](int value) {
        return value >= 1 && value <= 5;
    };

    if (!isValidScore(fullnessLevel) || !isValidScore(sleepinessLevel) ||
        !isValidScore(comfortLevel) || !isValidScore(focusImpactLevel) ||
        !isValidScore(tasteRating) || !isValidScore(repeatWillingness)) {
        m_lastError = QStringLiteral("Feedback scores must stay within 1 to 5.");
        emit stateChanged();
        return false;
    }

    MealFeedback feedback;
    feedback.mealLogId = mealLogId;
    RecommendationRecordRepository recommendationRecordRepository(
        m_databaseManager.connectionName());
    feedback.recommendationRecordId =
        recommendationRecordRepository.loadSelectedRecommendationRecordIdForMealLog(
            mealLogId);
    feedback.fullnessLevel = fullnessLevel;
    feedback.sleepinessLevel = sleepinessLevel;
    feedback.comfortLevel = comfortLevel;
    feedback.focusImpactLevel = focusImpactLevel;
    feedback.tasteRating = tasteRating;
    feedback.repeatWillingness = repeatWillingness;
    feedback.wouldEatAgain = wouldEatAgain;
    feedback.freeTextFeedback = freeTextFeedback.trimmed();

    MealFeedbackRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.upsertFeedback(feedback, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to save meal feedback.")
                          : errorMessage;
        emit stateChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool MealLogManager::saveMealLog(const QString &mealType,
                                 const QString &eatenAtIso,
                                 int weekday,
                                 bool hasClassAfterMeal,
                                 int minutesUntilNextClass,
                                 const QString &locationType,
                                 const QString &diningMode,
                                 double totalPrice,
                                 int totalEatTimeMinutes,
                                 int preMealHungerLevel,
                                 int preMealEnergyLevel,
                                 const QString &moodTag,
                                 const QString &notes)
{
    const auto isValidMealType = [](const QString &value) {
        return value == QStringLiteral("breakfast") ||
               value == QStringLiteral("lunch") ||
               value == QStringLiteral("dinner") ||
               value == QStringLiteral("snack");
    };
    const auto isValidLocationType = [](const QString &value) {
        return value == QStringLiteral("campus") ||
               value == QStringLiteral("dorm") ||
               value == QStringLiteral("commute") ||
               value == QStringLiteral("off_campus");
    };
    const auto isValidDiningMode = [](const QString &value) {
        return value == QStringLiteral("dine_in") ||
               value == QStringLiteral("takeaway") ||
               value == QStringLiteral("delivery");
    };
    const auto isValidMoodLevel = [](int value) {
        return value >= 1 && value <= 5;
    };

    if (mealType.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Meal type is required.");
        emit stateChanged();
        return false;
    }

    if (!isValidMealType(mealType.trimmed())) {
        m_lastError = QStringLiteral("Meal type is invalid.");
        emit stateChanged();
        return false;
    }

    if (m_selectedDishes.isEmpty()) {
        m_lastError = QStringLiteral("Add at least one dish to this meal.");
        emit stateChanged();
        return false;
    }

    if (totalPrice < 0.0 || totalEatTimeMinutes < 0 || minutesUntilNextClass < 0) {
        m_lastError = QStringLiteral("Numeric fields cannot be negative.");
        emit stateChanged();
        return false;
    }

    if (weekday < 1 || weekday > 7) {
        m_lastError = QStringLiteral("Weekday must stay within 1 to 7.");
        emit stateChanged();
        return false;
    }

    if (!isValidLocationType(locationType.trimmed())) {
        m_lastError = QStringLiteral("Location type is invalid.");
        emit stateChanged();
        return false;
    }

    if (!isValidDiningMode(diningMode.trimmed())) {
        m_lastError = QStringLiteral("Dining mode is invalid.");
        emit stateChanged();
        return false;
    }

    if (!isValidMoodLevel(preMealHungerLevel) ||
        !isValidMoodLevel(preMealEnergyLevel)) {
        m_lastError = QStringLiteral("Pre-meal hunger and energy must stay within 1 to 5.");
        emit stateChanged();
        return false;
    }

    if (hasClassAfterMeal && minutesUntilNextClass <= 0) {
        m_lastError = QStringLiteral("If you mark class after meal, minutes until next class must be greater than 0.");
        emit stateChanged();
        return false;
    }

    MealLog mealLog;
    mealLog.id = m_editingMealLogId;
    mealLog.mealType = mealType.trimmed();
    const QString trimmedEatenAt = eatenAtIso.trimmed();
    const QRegularExpression strictIsoPattern(
        QStringLiteral("^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}$"));
    mealLog.eatenAt = QDateTime::fromString(trimmedEatenAt, Qt::ISODate);
    if (!strictIsoPattern.match(trimmedEatenAt).hasMatch() ||
        !mealLog.eatenAt.isValid() ||
        mealLog.eatenAt.toString(Qt::ISODate) != trimmedEatenAt) {
        m_lastError = QStringLiteral("Use a valid meal time such as 2026-04-22T12:30:00.");
        emit stateChanged();
        return false;
    }
    mealLog.weekday = weekday;
    mealLog.hasClassAfterMeal = hasClassAfterMeal;
    mealLog.minutesUntilNextClass = hasClassAfterMeal ? minutesUntilNextClass : 0;
    mealLog.locationType = locationType.trimmed();
    mealLog.diningMode = diningMode.trimmed();
    mealLog.totalPrice = totalPrice;
    mealLog.totalEatTimeMinutes = totalEatTimeMinutes;
    mealLog.preMealHungerLevel = preMealHungerLevel;
    mealLog.preMealEnergyLevel = preMealEnergyLevel;
    mealLog.moodTag = moodTag.trimmed();
    mealLog.notes = notes.trimmed();

    QList<MealLogDishItem> dishItems;
    for (const QVariant &dishVariant : m_selectedDishes) {
        const QVariantMap dishMap = dishVariant.toMap();
        MealLogDishItem item;
        item.dishId = dishMap.value(QStringLiteral("id")).toInt();
        item.portionRatio = dishMap.value(QStringLiteral("portionRatio")).toDouble();
        item.customNotes = dishMap.value(QStringLiteral("customNotes")).toString().trimmed();
        if (item.dishId <= 0 || item.portionRatio <= 0.0) {
            m_lastError = QStringLiteral("Each selected dish must keep a valid id and positive weight.");
            emit stateChanged();
            return false;
        }
        dishItems.append(item);
    }

    MealLogRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    int persistedMealLogId = m_editingMealLogId;
    const bool ok = m_editingMealLogId > 0
                        ? repository.updateMealLog(m_editingMealLogId, mealLog, dishItems, &errorMessage)
                        : repository.addMealLog(mealLog, dishItems, &persistedMealLogId,
                                                &errorMessage);
    if (!ok) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to save meal log.")
                          : errorMessage;
        emit stateChanged();
        return false;
    }

    if (persistedMealLogId > 0 && !dishItems.isEmpty()) {
        QList<int> selectedDishIds;
        selectedDishIds.reserve(dishItems.size());
        for (const MealLogDishItem &item : dishItems) {
            if (item.dishId > 0 && !selectedDishIds.contains(item.dishId)) {
                selectedDishIds.append(item.dishId);
            }
        }

        RecommendationRecordRepository recommendationRecordRepository(
            m_databaseManager.connectionName());
        recommendationRecordRepository.markMatchingRecommendationSelected(
            mealLog.mealType, selectedDishIds, mealLog.eatenAt, persistedMealLogId);
    }

    m_editingMealLogId = 0;
    m_selectedDishes.clear();
    m_lastError.clear();
    reload();
    return true;
}

void MealLogManager::refreshState()
{
    DishRepository dishRepository(m_databaseManager.connectionName());
    MerchantRepository merchantRepository(m_databaseManager.connectionName());
    MealLogRepository mealLogRepository(m_databaseManager.connectionName());
    MealFeedbackRepository mealFeedbackRepository(m_databaseManager.connectionName());
    RecommendationRecordRepository recommendationRecordRepository(
        m_databaseManager.connectionName());

    const QList<Dish> dishes = dishRepository.loadActiveDishes();
    const QList<Dish> allDishes = dishRepository.loadAllDishes();
    const QList<Merchant> merchants = merchantRepository.loadAllMerchants();
    const QList<MealLog> mealLogs = mealLogRepository.loadRecentMealLogs();
    const QList<MealLog> insightMealLogs = mealLogRepository.loadRecentMealLogs(24);
    const QList<int> recentDishIds = mealLogRepository.loadRecentDishIds(24);
    const QList<DishFeedbackAggregate> feedbackAggregates =
        mealFeedbackRepository.loadDishFeedbackAggregates();

    QHash<int, QString> merchantNameById;
    for (const Merchant &merchant : merchants) {
        merchantNameById.insert(merchant.id, merchant.name);
    }

    QHash<int, QString> dishNameById;
    QHash<int, Dish> dishById;
    for (const Dish &dish : allDishes) {
        dishById.insert(dish.id, dish);
        dishNameById.insert(dish.id, dish.name);
    }

    QHash<int, DishFeedbackAggregate> feedbackByDishId;
    for (const DishFeedbackAggregate &aggregate : feedbackAggregates) {
        feedbackByDishId.insert(aggregate.dishId, aggregate);
    }

    QHash<int, int> dishCounts;
    for (const int dishId : recentDishIds) {
        dishCounts[dishId] += 1;
    }

    m_availableDishes.clear();
    for (const Dish &dish : dishes) {
        QVariantMap dishMap;
        dishMap.insert(QStringLiteral("id"), dish.id);
        dishMap.insert(QStringLiteral("name"), dish.name);
        dishMap.insert(QStringLiteral("merchantId"), dish.merchantId);
        dishMap.insert(QStringLiteral("merchantName"),
                       merchantNameById.value(dish.merchantId));
        dishMap.insert(QStringLiteral("category"), dish.category);
        dishMap.insert(QStringLiteral("price"), dish.price);
        dishMap.insert(QStringLiteral("mealImpactWeight"), dish.mealImpactWeight);
        dishMap.insert(QStringLiteral("isBeverage"), dish.isBeverage);
        dishMap.insert(QStringLiteral("sleepinessRiskLevel"),
                       dish.sleepinessRiskLevel);
        dishMap.insert(QStringLiteral("defaultDiningMode"), dish.defaultDiningMode);
        dishMap.insert(QStringLiteral("recentUseCount"),
                       dishCounts.value(dish.id));
        dishMap.insert(QStringLiteral("notes"), dish.notes);
        m_availableDishes.append(dishMap);
    }
    m_frequentDishes =
        topItemsFromCounts(m_availableDishes, dishCounts, QStringLiteral("id"));

    m_recentMeals.clear();
    m_feedbackInsights.clear();
    QHash<int, QVariantMap> mealMapById;
    QHash<int, QVariantMap> recommendationDetailCache;
    QHash<int, QList<QVariantMap>> mealSamplesByDishId;
    QList<QVariantMap> mealsWithFeedbackSamples;
    QList<QVariantMap> mealsMissingFeedbackSamples;
    QList<QVariantMap> linkedRecommendationMealSamples;
    QList<QVariantMap> classFeedbackMealSamples;
    QList<QVariantMap> relaxedFeedbackMealSamples;
    QList<QVariantMap> linkedRecommendationTimelineMeals;

    const auto buildMealMap =
        [&](const MealLog &mealLog,
            const MealFeedback &feedback,
            const RecommendationSelectionLink &selectionLink,
            const QList<MealLogDishItem> &dishItems) {
            QStringList dishNames;
            QVariantList dishItemsForUi;
            QVariantList dishIdsForUi;
            QSet<int> seenDishIds;

            for (const MealLogDishItem &dishItem : dishItems) {
                QVariantMap dishItemMap;
                const Dish dish = dishById.value(dishItem.dishId);
                dishItemMap.insert(QStringLiteral("id"), dishItem.dishId);
                dishItemMap.insert(QStringLiteral("name"),
                                   dish.id > 0
                                       ? dish.name
                                       : QStringLiteral("Dish #%1").arg(dishItem.dishId));
                dishItemMap.insert(QStringLiteral("merchantName"),
                                   merchantNameById.value(dish.merchantId));
                dishItemMap.insert(QStringLiteral("defaultDiningMode"),
                                   dish.defaultDiningMode);
                dishItemMap.insert(QStringLiteral("portionRatio"), dishItem.portionRatio);
                dishItemMap.insert(QStringLiteral("customNotes"), dishItem.customNotes);
                dishItemsForUi.append(dishItemMap);
                dishNames.append(dishItemMap.value(QStringLiteral("name")).toString());

                if (!seenDishIds.contains(dishItem.dishId)) {
                    seenDishIds.insert(dishItem.dishId);
                    dishIdsForUi.append(dishItem.dishId);
                }
            }

            QVariantMap recommendationDetail;
            if (selectionLink.recommendationRecordId > 0) {
                if (!recommendationDetailCache.contains(
                        selectionLink.recommendationRecordId)) {
                    recommendationDetailCache.insert(
                        selectionLink.recommendationRecordId,
                        recommendationRecordRepository.loadRecordDetails(
                            selectionLink.recommendationRecordId));
                }
                recommendationDetail =
                    recommendationDetailCache.value(selectionLink.recommendationRecordId);
            }

            QVariantMap mealMap;
            mealMap.insert(QStringLiteral("sortTimestamp"),
                           mealLog.eatenAt.toMSecsSinceEpoch());
            mealMap.insert(QStringLiteral("id"), mealLog.id);
            mealMap.insert(QStringLiteral("mealType"), mealLog.mealType);
            mealMap.insert(QStringLiteral("mealTypeLabel"),
                           readableMealTypeDisplayLabelV3(mealLog.mealType));
            mealMap.insert(QStringLiteral("eatenAt"),
                           mealLog.eatenAt.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
            mealMap.insert(QStringLiteral("weekday"), mealLog.weekday);
            mealMap.insert(QStringLiteral("hasClassAfterMeal"),
                           mealLog.hasClassAfterMeal);
            mealMap.insert(QStringLiteral("minutesUntilNextClass"),
                           mealLog.minutesUntilNextClass);
            mealMap.insert(QStringLiteral("locationType"), mealLog.locationType);
            mealMap.insert(QStringLiteral("diningMode"), mealLog.diningMode);
            mealMap.insert(QStringLiteral("totalPrice"), mealLog.totalPrice);
            mealMap.insert(QStringLiteral("totalEatTimeMinutes"),
                           mealLog.totalEatTimeMinutes);
            mealMap.insert(QStringLiteral("preMealHungerLevel"),
                           mealLog.preMealHungerLevel);
            mealMap.insert(QStringLiteral("preMealEnergyLevel"),
                           mealLog.preMealEnergyLevel);
            mealMap.insert(QStringLiteral("dishSummary"),
                           dishNames.join(QStringLiteral(", ")));
            mealMap.insert(QStringLiteral("dishItems"), dishItemsForUi);
            mealMap.insert(QStringLiteral("dishIds"), dishIdsForUi);
            mealMap.insert(QStringLiteral("notes"), mealLog.notes);
            mealMap.insert(QStringLiteral("linkedRecommendationRecordId"),
                           selectionLink.recommendationRecordId);
            mealMap.insert(QStringLiteral("linkedRecommendationCandidateRank"),
                           selectionLink.selectedCandidateRank);
            mealMap.insert(QStringLiteral("linkedRecommendationDishId"),
                           selectionLink.selectedDishId);
            mealMap.insert(QStringLiteral("linkedRecommendationDishName"),
                           selectionLink.selectedDishName);
            mealMap.insert(QStringLiteral("linkedRecommendationGeneratedAt"),
                           selectionLink.generatedAt.isValid()
                               ? selectionLink.generatedAt.toString(
                                     QStringLiteral("yyyy-MM-dd hh:mm"))
                               : QString());
            mealMap.insert(QStringLiteral("recommendationContextSummary"),
                           recommendationDetail.value(
                               QStringLiteral("contextSummary")));
            mealMap.insert(QStringLiteral("recommendationContextHeadline"),
                           recommendationDetail.value(
                               QStringLiteral("contextHeadline")));
            mealMap.insert(QStringLiteral("recommendationPreviewText"),
                           recommendationDetail.value(QStringLiteral("previewText")));
            mealMap.insert(QStringLiteral("recommendationSelectionSummary"),
                           recommendationDetail.value(
                               QStringLiteral("selectionSummary")));
            mealMap.insert(QStringLiteral("recommendationComparisonSummary"),
                           recommendationDetail.value(
                               QStringLiteral("comparisonSummary")));
            mealMap.insert(QStringLiteral("recommendationCompareGuideText"),
                           recommendationDetail.value(
                               QStringLiteral("compareGuideText")));
            mealMap.insert(QStringLiteral("recommendationComparePriorityHeadline"),
                           recommendationDetail.value(
                               QStringLiteral("comparePriorityHeadline")));
            mealMap.insert(QStringLiteral("recommendationScoreGapValue"),
                           recommendationDetail.value(
                               QStringLiteral("scoreGapValue")));
            mealMap.insert(QStringLiteral("recommendationScoreGapSummary"),
                           recommendationDetail.value(
                               QStringLiteral("scoreGapSummary")));
            mealMap.insert(QStringLiteral("recommendationSelectedReason"),
                           recommendationDetail.value(
                               QStringLiteral("selectedReason")));
            mealMap.insert(QStringLiteral("recommendationTopCandidateReason"),
                           recommendationDetail.value(
                               QStringLiteral("topCandidateReason")));
            mealMap.insert(QStringLiteral("recommendationTopCandidateDishName"),
                           recommendationDetail.value(
                               QStringLiteral("topCandidateDishName")));
            mealMap.insert(QStringLiteral("recommendationTopCandidateScoreText"),
                           recommendationDetail.value(
                               QStringLiteral("topCandidateScoreText")));
            mealMap.insert(QStringLiteral("recommendationSelectedCandidateDishName"),
                           recommendationDetail.value(
                               QStringLiteral("selectedCandidateDishName")));
            mealMap.insert(QStringLiteral("recommendationSelectedCandidateScoreText"),
                           recommendationDetail.value(
                               QStringLiteral("selectedCandidateScoreText")));
            mealMap.insert(QStringLiteral("recommendationCandidates"),
                           recommendationDetail.value(QStringLiteral("candidates")));
            mealMap.insert(QStringLiteral("feedbackSaved"), feedback.id > 0);
            mealMap.insert(QStringLiteral("feedbackRecommendationRecordId"),
                           feedback.recommendationRecordId);
            mealMap.insert(QStringLiteral("feedbackFullnessLevel"), feedback.fullnessLevel);
            mealMap.insert(QStringLiteral("feedbackSleepinessLevel"), feedback.sleepinessLevel);
            mealMap.insert(QStringLiteral("feedbackComfortLevel"), feedback.comfortLevel);
            mealMap.insert(QStringLiteral("feedbackFocusImpactLevel"), feedback.focusImpactLevel);
            mealMap.insert(QStringLiteral("feedbackTasteRating"), feedback.tasteRating);
            mealMap.insert(QStringLiteral("feedbackRepeatWillingness"), feedback.repeatWillingness);
            mealMap.insert(QStringLiteral("feedbackWouldEatAgain"), feedback.wouldEatAgain);
            mealMap.insert(QStringLiteral("feedbackText"), feedback.freeTextFeedback);
            mealMap.insert(QStringLiteral("feedbackQuickSummary"),
                           feedback.id > 0
                               ? QStringLiteral("口味 %1 / 复吃 %2 / 困倦 %3")
                                     .arg(feedback.tasteRating)
                                     .arg(feedback.repeatWillingness)
                                     .arg(feedback.sleepinessLevel)
                               : QStringLiteral("这餐还没有补反馈"));
            mealMap.insert(QStringLiteral("feedbackSummary"),
                           feedback.id > 0
                                ? QStringLiteral(
                                      "口味 %1 | 复吃 %2 | 犯困 %3 | 专注 %4")
                                      .arg(feedback.tasteRating)
                                      .arg(feedback.repeatWillingness)
                                      .arg(feedback.sleepinessLevel)
                                      .arg(feedback.focusImpactLevel)
                                : QStringLiteral("这餐还没保存反馈"));
            return mealMap;
        };

    const auto addUniqueMealSample =
        [](QList<QVariantMap> *target, const QVariantMap &mealMap) {
            if (target == nullptr || mealMap.isEmpty()) {
                return;
            }

            const int mealId = mealMap.value(QStringLiteral("id")).toInt();
            for (const QVariantMap &existing : *target) {
                if (existing.value(QStringLiteral("id")).toInt() == mealId) {
                    return;
                }
            }

            target->append(mealMap);
        };

    const auto dishIdsFromSignals = [](const QList<RankedDishSignal> &dishSignalList) {
        QList<int> ids;
        for (const RankedDishSignal &signal : dishSignalList) {
            if (signal.dishId > 0) {
                ids.append(signal.dishId);
            }
        }
        return ids;
    };

    QList<QVariantMap> synthesizedMealSamples;
    QList<QVariantMap> synthesizedDishSamples;
    QVariantList synthesizedWeightHints;
    QSet<QString> synthesizedWeightHintKeys;
    QSet<int> synthesizedDishIds;
    QStringList synthesizedPatternTitles;
    const auto addSynthesizedHint =
        [&](const QVariantList &weightHints,
            const QVariantList &supportingDishes,
            const QVariantList &supportingMeals,
            const QString &patternTitle) {
            if (weightHints.isEmpty()) {
                return;
            }

            if (!patternTitle.isEmpty() &&
                !synthesizedPatternTitles.contains(patternTitle)) {
                synthesizedPatternTitles.append(patternTitle);
            }

            for (const QVariant &hintVariant : weightHints) {
                const QVariantMap hint = hintVariant.toMap();
                const QString uniqueKey =
                    hint.value(QStringLiteral("key")).toString() +
                    QStringLiteral("|") +
                    hint.value(QStringLiteral("direction")).toString();
                if (uniqueKey.isEmpty() ||
                    synthesizedWeightHintKeys.contains(uniqueKey)) {
                    continue;
                }

                synthesizedWeightHintKeys.insert(uniqueKey);
                synthesizedWeightHints.append(hint);
            }

            for (const QVariant &dishVariant : supportingDishes) {
                const QVariantMap dishMap = dishVariant.toMap();
                const int dishId = dishMap.value(QStringLiteral("id")).toInt();
                if (dishId <= 0 || synthesizedDishIds.contains(dishId)) {
                    continue;
                }

                synthesizedDishIds.insert(dishId);
                synthesizedDishSamples.append(dishMap);
            }

            for (const QVariant &mealVariant : supportingMeals) {
                addUniqueMealSample(&synthesizedMealSamples, mealVariant.toMap());
            }
        };

    int feedbackSavedCount = 0;
    int linkedRecommendationCount = 0;
    int linkedFeedbackCount = 0;
    int top1HitCount = 0;
    int top2HitCount = 0;
    int top3HitCount = 0;
    for (const MealLog &mealLog : mealLogs) {
        const QList<MealLogDishItem> dishItems =
            mealLogRepository.loadMealLogDishItems(mealLog.id);
        const MealFeedback feedback =
            mealFeedbackRepository.loadFeedbackForMealLog(mealLog.id);
        const RecommendationSelectionLink selectionLink =
            recommendationRecordRepository.loadSelectionLinkForMealLog(mealLog.id);
        const QVariantMap mealMap =
            buildMealMap(mealLog, feedback, selectionLink, dishItems);
        mealMapById.insert(mealLog.id, mealMap);
        m_recentMeals.append(mealMap);

        if (feedback.id > 0) {
            feedbackSavedCount += 1;
            addUniqueMealSample(&mealsWithFeedbackSamples, mealMap);
        } else {
            addUniqueMealSample(&mealsMissingFeedbackSamples, mealMap);
        }
        if (feedback.recommendationRecordId > 0) {
            linkedFeedbackCount += 1;
        }
        if (selectionLink.recommendationRecordId > 0) {
            linkedRecommendationCount += 1;
            addUniqueMealSample(&linkedRecommendationMealSamples, mealMap);
            if (selectionLink.selectedCandidateRank == 1) {
                top1HitCount += 1;
            } else if (selectionLink.selectedCandidateRank == 2) {
                top2HitCount += 1;
            } else if (selectionLink.selectedCandidateRank == 3) {
                top3HitCount += 1;
            }
        }
    }

    int classFeedbackCount = 0;
    double classSleepinessSum = 0.0;
    double classFocusSum = 0.0;
    double classComfortSum = 0.0;
    int relaxedFeedbackCount = 0;
    double relaxedSleepinessSum = 0.0;
    double relaxedFocusSum = 0.0;
    double relaxedComfortSum = 0.0;
    QList<int> linkedRecommendationRanksTimeline;
    QHash<int, QList<DishTrendSample>> dishTrendSamples;

    for (const MealLog &mealLog : insightMealLogs) {
        const MealFeedback feedback =
            mealFeedbackRepository.loadFeedbackForMealLog(mealLog.id);
        if (feedback.id <= 0) {
            continue;
        }

        const QList<MealLogDishItem> dishItems =
            mealLogRepository.loadMealLogDishItems(mealLog.id);
        if (mealLog.hasClassAfterMeal) {
            classFeedbackCount += 1;
            classSleepinessSum += feedback.sleepinessLevel;
            classFocusSum += feedback.focusImpactLevel;
            classComfortSum += feedback.comfortLevel;
        } else {
            relaxedFeedbackCount += 1;
            relaxedSleepinessSum += feedback.sleepinessLevel;
            relaxedFocusSum += feedback.focusImpactLevel;
            relaxedComfortSum += feedback.comfortLevel;
        }

        const RecommendationSelectionLink selectionLink =
            recommendationRecordRepository.loadSelectionLinkForMealLog(mealLog.id);
        QVariantMap mealMap = mealMapById.value(mealLog.id);
        if (mealMap.isEmpty()) {
            mealMap = buildMealMap(mealLog, feedback, selectionLink, dishItems);
            mealMapById.insert(mealLog.id, mealMap);
        }

        if (mealLog.hasClassAfterMeal) {
            addUniqueMealSample(&classFeedbackMealSamples, mealMap);
        } else {
            addUniqueMealSample(&relaxedFeedbackMealSamples, mealMap);
        }

        if (selectionLink.selectedCandidateRank > 0) {
            linkedRecommendationRanksTimeline.append(selectionLink.selectedCandidateRank);
            addUniqueMealSample(&linkedRecommendationTimelineMeals, mealMap);
        }

        QSet<int> seenDishIds;
        for (const MealLogDishItem &dishItem : dishItems) {
            if (dishItem.dishId <= 0 || seenDishIds.contains(dishItem.dishId)) {
                continue;
            }
            seenDishIds.insert(dishItem.dishId);
            addUniqueMealSample(&mealSamplesByDishId[dishItem.dishId], mealMap);

            DishTrendSample sample;
            sample.eatenAt = mealLog.eatenAt;
            sample.taste = feedback.tasteRating;
            sample.repeat = feedback.repeatWillingness;
            sample.comfort = feedback.comfortLevel;
            sample.focus = feedback.focusImpactLevel;
            sample.sleepiness = feedback.sleepinessLevel;
            dishTrendSamples[dishItem.dishId].append(sample);
        }
    }

    const double feedbackCoverage = mealLogs.isEmpty()
                                        ? 0.0
                                        : 100.0 * feedbackSavedCount /
                                              static_cast<double>(mealLogs.size());

    bool hasContextSplitInsight = false;
    double classSleepiness = 0.0;
    double classFocus = 0.0;
    double classComfort = 0.0;
    double relaxedSleepiness = 0.0;
    double relaxedFocus = 0.0;
    double relaxedComfort = 0.0;
    QString contextTone;
    if (classFeedbackCount > 0 && relaxedFeedbackCount > 0) {
        hasContextSplitInsight = true;
        classSleepiness = classSleepinessSum / classFeedbackCount;
        classFocus = classFocusSum / classFeedbackCount;
        classComfort = classComfortSum / classFeedbackCount;
        relaxedSleepiness = relaxedSleepinessSum / relaxedFeedbackCount;
        relaxedFocus = relaxedFocusSum / relaxedFeedbackCount;
        relaxedComfort = relaxedComfortSum / relaxedFeedbackCount;
        contextTone =
            classSleepiness <= relaxedSleepiness + 0.15 &&
                    classFocus >= relaxedFocus - 0.15
                ? QStringLiteral("good")
                : classSleepiness >= relaxedSleepiness + 0.60
                    ? QStringLiteral("risk")
                    : QStringLiteral("watch");
    }

    bool hasRankingMomentumInsight = false;
    int momentumRecentWindow = 0;
    int momentumOlderWindow = 0;
    double recentTop1Rate = 0.0;
    double olderTop1Rate = 0.0;
    QString momentumTone;
    if (linkedRecommendationRanksTimeline.size() >= 4) {
        hasRankingMomentumInsight = true;
        const int splitIndex =
            (linkedRecommendationRanksTimeline.size() + 1) / 2;
        momentumRecentWindow = splitIndex;
        momentumOlderWindow =
            linkedRecommendationRanksTimeline.size() - splitIndex;

        int recentTop1 = 0;
        int olderTop1 = 0;
        for (int index = 0; index < linkedRecommendationRanksTimeline.size(); ++index) {
            const bool top1 = linkedRecommendationRanksTimeline.at(index) == 1;
            if (index < splitIndex) {
                recentTop1 += top1 ? 1 : 0;
            } else {
                olderTop1 += top1 ? 1 : 0;
            }
        }

        recentTop1Rate =
            100.0 * recentTop1 / static_cast<double>(momentumRecentWindow);
        olderTop1Rate =
            100.0 * olderTop1 / static_cast<double>(momentumOlderWindow);
        momentumTone = recentTop1Rate >= olderTop1Rate + 10.0
                           ? QStringLiteral("good")
                           : recentTop1Rate + 10.0 <= olderTop1Rate
                               ? QStringLiteral("risk")
                               : QStringLiteral("watch");
    }

    QList<RankedDishSignal> favoriteSignals;
    QList<RankedDishSignal> sleepySignals;
    QList<RankedDishSignal> lowRepeatSignals;
    QList<RankedDishSignal> improvingSignals;
    QList<RankedDishSignal> worseningSignals;
    for (const DishFeedbackAggregate &aggregate : feedbackAggregates) {
        if (aggregate.effectiveSampleWeight < 1.6 || aggregate.sampleCount < 2) {
            continue;
        }

        if (aggregate.avgTasteRating >= 4.0 && aggregate.avgRepeatWillingness >= 3.8) {
            favoriteSignals.append({aggregate.dishId,
                                    aggregate.avgTasteRating,
                                    aggregate.avgRepeatWillingness,
                                    aggregate.sampleCount});
        }

        if (aggregate.avgSleepinessLevel >= 3.8) {
            sleepySignals.append({aggregate.dishId,
                                  aggregate.avgSleepinessLevel,
                                  aggregate.avgComfortLevel,
                                  aggregate.sampleCount});
        }

        if (aggregate.avgRepeatWillingness > 0.0 &&
            aggregate.avgRepeatWillingness <= 2.8) {
            lowRepeatSignals.append({aggregate.dishId,
                                     aggregate.avgRepeatWillingness,
                                     aggregate.avgTasteRating,
                                     aggregate.sampleCount});
        }

        QList<DishTrendSample> trendSamples = dishTrendSamples.value(aggregate.dishId);
        if (trendSamples.size() >= 4) {
            std::sort(trendSamples.begin(), trendSamples.end(),
                      [](const DishTrendSample &left, const DishTrendSample &right) {
                          return left.eatenAt > right.eatenAt;
                      });
            const int splitIndex = (trendSamples.size() + 1) / 2;
            const QList<DishTrendSample> recentSamples = trendSamples.mid(0, splitIndex);
            const QList<DishTrendSample> olderSamples = trendSamples.mid(splitIndex);
            if (!olderSamples.isEmpty()) {
                const MetricAverage recentAverage =
                    averageTrendSamples(recentSamples);
                const MetricAverage olderAverage =
                    averageTrendSamples(olderSamples);
                const double delta =
                    desirabilityScore(recentAverage) -
                    desirabilityScore(olderAverage);
                if (delta >= 0.45) {
                    improvingSignals.append({aggregate.dishId,
                                             delta,
                                             desirabilityScore(recentAverage),
                                             aggregate.sampleCount});
                } else if (delta <= -0.45) {
                    worseningSignals.append({aggregate.dishId,
                                             -delta,
                                             desirabilityScore(recentAverage),
                                             aggregate.sampleCount});
                }
            }
        }
    }

    std::sort(favoriteSignals.begin(), favoriteSignals.end(),
              [](const RankedDishSignal &left, const RankedDishSignal &right) {
                  return (left.primaryValue + left.secondaryValue) >
                         (right.primaryValue + right.secondaryValue);
              });
    std::sort(sleepySignals.begin(), sleepySignals.end(),
              [](const RankedDishSignal &left, const RankedDishSignal &right) {
                  return left.primaryValue > right.primaryValue;
              });
    std::sort(lowRepeatSignals.begin(), lowRepeatSignals.end(),
              [](const RankedDishSignal &left, const RankedDishSignal &right) {
                  return left.primaryValue < right.primaryValue;
              });
    std::sort(improvingSignals.begin(), improvingSignals.end(),
              [](const RankedDishSignal &left, const RankedDishSignal &right) {
                  return left.primaryValue > right.primaryValue;
              });
    std::sort(worseningSignals.begin(), worseningSignals.end(),
              [](const RankedDishSignal &left, const RankedDishSignal &right) {
                  return left.primaryValue > right.primaryValue;
              });

    if (!favoriteSignals.isEmpty()) {
        favoriteSignals = favoriteSignals.mid(0, 3);
    }
    if (!sleepySignals.isEmpty()) {
        sleepySignals = sleepySignals.mid(0, 3);
    }
    if (!lowRepeatSignals.isEmpty()) {
        lowRepeatSignals = lowRepeatSignals.mid(0, 3);
    }
    if (!improvingSignals.isEmpty()) {
        improvingSignals = improvingSignals.mid(0, 3);
    }
    if (!worseningSignals.isEmpty()) {
        worseningSignals = worseningSignals.mid(0, 3);
    }

    const QVariantList favoriteDishSupports = buildDishSupportListV2(
        favoriteSignals, feedbackByDishId, dishById, merchantNameById,
        mealSamplesByDishId, QStringLiteral("口味"),
        QStringLiteral("复吃"));
    const QVariantList sleepyDishSupports = buildDishSupportListV2(
        sleepySignals, feedbackByDishId, dishById, merchantNameById,
        mealSamplesByDishId, QStringLiteral("犯困"),
        QStringLiteral("舒适"));
    const QVariantList lowRepeatDishSupports = buildDishSupportListV2(
        lowRepeatSignals, feedbackByDishId, dishById, merchantNameById,
        mealSamplesByDishId, QStringLiteral("复吃"),
        QStringLiteral("口味"));
    const QVariantList improvingDishSupports = buildDishSupportListV2(
        improvingSignals, feedbackByDishId, dishById, merchantNameById,
        mealSamplesByDishId, QStringLiteral("改善幅度"),
        QStringLiteral("近期得分"));
    const QVariantList worseningDishSupports = buildDishSupportListV2(
        worseningSignals, feedbackByDishId, dishById, merchantNameById,
        mealSamplesByDishId, QStringLiteral("恶化幅度"),
        QStringLiteral("近期得分"));

    QVariantMap coverageInsight;
    if (!mealLogs.isEmpty()) {
        coverageInsight = buildInsight(
            QStringLiteral("feedback_coverage"),
            QStringLiteral("反馈覆盖率"),
            QStringLiteral("最近 %2 餐里，已有 %1 餐保存反馈")
                .arg(feedbackSavedCount)
                .arg(mealLogs.size()),
            feedbackCoverage >= 60.0
                ? QStringLiteral(
                      "覆盖率 %1%%，其中 %2 次反馈已经回链到当时的推荐记录")
                      .arg(QString::number(feedbackCoverage, 'f', 0))
                      .arg(linkedFeedbackCount)
                : QStringLiteral(
                      "覆盖率 %1%%，下面这些餐次最适合优先补反馈，能更快增加证据")
                      .arg(QString::number(feedbackCoverage, 'f', 0)),
            feedbackCoverage >= 60.0 ? QStringLiteral("good")
                                     : QStringLiteral("watch"),
            {},
            toVariantList(feedbackCoverage >= 60.0 ? mealsWithFeedbackSamples
                                                   : mealsMissingFeedbackSamples,
                          6));
    }

    QVariantMap recommendationHitsInsight;
    if (linkedRecommendationCount > 0) {
        const double top1Rate =
            100.0 * top1HitCount / static_cast<double>(linkedRecommendationCount);
        recommendationHitsInsight = buildInsight(
            QStringLiteral("recommendation_hits"),
            QStringLiteral("推荐命中情况"),
            QStringLiteral("最近有 %1 餐能回链推荐，其中 top-1 命中率为 %2%%")
                .arg(linkedRecommendationCount)
                .arg(QString::number(top1Rate, 'f', 0)),
            QStringLiteral("top-2 命中 %1 次，top-3 命中 %2 次")
                .arg(top2HitCount)
                .arg(top3HitCount),
            top1Rate >= 60.0 ? QStringLiteral("good") : QStringLiteral("watch"),
            {},
            sortSupportMealsV2(linkedRecommendationMealSamples, 6));
    } else if (!mealLogs.isEmpty()) {
        recommendationHitsInsight = buildInsight(
            QStringLiteral("recommendation_hits"),
            QStringLiteral("推荐命中情况"),
            QStringLiteral("最近餐次里还没有成功回链到推荐记录"),
            QStringLiteral(
                "当前还看不出命中率走势，先继续记录能命中的餐次会更有价值"),
            QStringLiteral("watch"),
            {},
            sortSupportMealsV2(mealsWithFeedbackSamples, 6));
    }

    QVariantMap contextInsight;
    if (hasContextSplitInsight) {
        QVariantList contextWeightHints;
        if (contextTone != QStringLiteral("good")) {
            contextWeightHints.append(buildWeightHint(
                QStringLiteral("scene.class_fit"),
                QStringLiteral("课前适配判断"),
                QStringLiteral("increase"),
                QStringLiteral("moderate"),
                QStringLiteral(
                    "有课压力下的餐后表现仍明显差于放松场景，说明课前适配还需要更强约束。")));
            contextWeightHints.append(buildWeightHint(
                QStringLiteral("nutrition.sleepiness_risk_fit"),
                QStringLiteral("犯困风险惩罚"),
                QStringLiteral("increase"),
                QStringLiteral("moderate"),
                QStringLiteral(
                    "有课场景和放松场景的差异主要还在犯困反馈，说明这条惩罚还不够敏感。")));
            if (classComfort + 0.30 <= relaxedComfort) {
                contextWeightHints.append(buildWeightHint(
                    QStringLiteral("nutrition.digestive_burden_fit"),
                    QStringLiteral("消化负担惩罚"),
                    QStringLiteral("increase"),
                    QStringLiteral("slight"),
                    QStringLiteral(
                        "有课场景下舒适度也在下降，偏重的餐食还需要再压一点。")));
            }
        }

        QList<QVariantMap> contextMeals = classFeedbackMealSamples;
        for (const QVariantMap &mealMap : relaxedFeedbackMealSamples) {
            addUniqueMealSample(&contextMeals, mealMap);
        }

        contextInsight = buildInsight(
            QStringLiteral("context_split"),
            QStringLiteral("场景分层对比"),
            QStringLiteral("有课场景犯困 %1 / 专注 %2；放松场景犯困 %3 / 专注 %4")
                .arg(QString::number(classSleepiness, 'f', 1))
                .arg(QString::number(classFocus, 'f', 1))
                .arg(QString::number(relaxedSleepiness, 'f', 1))
                .arg(QString::number(relaxedFocus, 'f', 1)),
            QStringLiteral("舒适度 %1 vs %2，样本来自 %3 餐有课反馈和 %4 餐放松反馈")
                .arg(QString::number(classComfort, 'f', 1))
                .arg(QString::number(relaxedComfort, 'f', 1))
                .arg(classFeedbackCount)
                .arg(relaxedFeedbackCount),
            contextTone,
            sleepyDishSupports,
            sortSupportMealsV2(contextMeals, 6),
            contextWeightHints);
        addSynthesizedHint(contextWeightHints, sleepyDishSupports,
                           contextInsight.value(QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("场景分层对比"));
    }

    QVariantMap rankingMomentumInsight;
    if (hasRankingMomentumInsight) {
        QVariantList momentumWeightHints;
        if (momentumTone == QStringLiteral("risk")) {
            momentumWeightHints.append(buildWeightHint(
                QStringLiteral("group.scene_fit"),
                QStringLiteral("场景适配总权重"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "最近的 top-1 命中率落后于更早阶段，说明场景适配整体可能还不够有分量。")));
            momentumWeightHints.append(buildWeightHint(
                QStringLiteral("group.diversity_fit"),
                QStringLiteral("多餐补偿总权重"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "如果老偏好仍然长期排在前面，可以稍微加强多餐补偿，减少陈旧重复。")));
        }

        rankingMomentumInsight = buildInsight(
            QStringLiteral("ranking_momentum"),
            QStringLiteral("命中走势"),
            QStringLiteral("最近窗口 top-1 命中率 %1%%，较早窗口为 %2%%")
                .arg(QString::number(recentTop1Rate, 'f', 0))
                .arg(QString::number(olderTop1Rate, 'f', 0)),
            QStringLiteral("最近窗口共 %1 个已回链样本，较早窗口共 %2 个")
                .arg(momentumRecentWindow)
                .arg(momentumOlderWindow),
            momentumTone,
            {},
            sortSupportMealsV2(linkedRecommendationTimelineMeals, 6),
            momentumWeightHints);
        addSynthesizedHint(momentumWeightHints, {},
                           rankingMomentumInsight.value(
                               QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("命中走势"));
    }

    QVariantMap stableFavoritesInsight;
    if (!favoriteSignals.isEmpty()) {
        const QVariantList favoriteWeightHints = {
            buildWeightHint(
                QStringLiteral("preference.taste_rating"),
                QStringLiteral("口味反馈信号"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "有几道菜的口味评分已经稳定偏高，说明这条信号可以再稍微信任一些。")),
            buildWeightHint(
                QStringLiteral("preference.repeat_willingness"),
                QStringLiteral("复吃意愿信号"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "复吃意愿也比较稳定，说明这些经过验证的偏好可以更快体现在排序里。"))};
        stableFavoritesInsight = buildInsight(
            QStringLiteral("stable_favorites"),
            QStringLiteral("稳定偏好"),
            QStringLiteral("这些菜的口味和复吃意愿都比较稳，可以视为靠谱偏好"),
            formatDishSignalList(favoriteSignals, dishNameById,
                                 QStringLiteral(" 口味"),
                                 QStringLiteral(" 复吃")),
            QStringLiteral("good"),
            favoriteDishSupports,
            gatherMealSupportList(dishIdsFromSignals(favoriteSignals),
                                  mealSamplesByDishId, 6),
            favoriteWeightHints);
        addSynthesizedHint(favoriteWeightHints, favoriteDishSupports,
                           stableFavoritesInsight.value(
                               QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("稳定偏好"));
    }

    QVariantMap sleepinessWatchInsight;
    if (!sleepySignals.isEmpty()) {
        const QVariantList sleepyWeightHints = {
            buildWeightHint(
                QStringLiteral("nutrition.sleepiness_risk_fit"),
                QStringLiteral("犯困风险惩罚"),
                QStringLiteral("increase"),
                QStringLiteral("moderate"),
                QStringLiteral(
                    "这些菜多次带来更高的犯困反馈，课前场景应该更早把它们压下去。")),
            buildWeightHint(
                QStringLiteral("nutrition.digestive_burden_fit"),
                QStringLiteral("消化负担惩罚"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "舒适度并没有抵消犯困问题，偏重的餐还需要再加一点拖拽。"))};
        sleepinessWatchInsight = buildInsight(
            QStringLiteral("sleepiness_watch"),
            QStringLiteral("犯困观察"),
            QStringLiteral("这些菜的餐后犯困反馈反复偏高，值得优先盯住"),
            formatDishSignalList(sleepySignals, dishNameById,
                                 QStringLiteral(" 犯困"),
                                 QStringLiteral(" 舒适")),
            QStringLiteral("watch"),
            sleepyDishSupports,
            gatherMealSupportList(dishIdsFromSignals(sleepySignals),
                                  mealSamplesByDishId, 6),
            sleepyWeightHints);
        addSynthesizedHint(sleepyWeightHints, sleepyDishSupports,
                           sleepinessWatchInsight.value(
                               QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("犯困观察"));
    }

    QVariantMap lowRepeatInsight;
    if (!lowRepeatSignals.isEmpty()) {
        const QVariantList lowRepeatWeightHints = {
            buildWeightHint(
                QStringLiteral("preference.repeat_willingness"),
                QStringLiteral("复吃意愿信号"),
                QStringLiteral("increase"),
                QStringLiteral("moderate"),
                QStringLiteral(
                    "这里最稳定的负向信号就是复吃意愿偏低，它应该更快把这些菜往下拉。")),
            buildWeightHint(
                QStringLiteral("preference.preference_score"),
                QStringLiteral("综合偏好回传"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "如果总是吃完但下次不想再点，综合偏好回传应该更快反映这种降温。"))};
        lowRepeatInsight = buildInsight(
            QStringLiteral("low_repeat"),
            QStringLiteral("复吃意愿偏低"),
            QStringLiteral("这些菜并不是吃不下，而是吃完后不太想再选"),
            formatDishSignalList(lowRepeatSignals, dishNameById,
                                 QStringLiteral(" 复吃"),
                                 QStringLiteral(" 口味")),
            QStringLiteral("risk"),
            lowRepeatDishSupports,
            gatherMealSupportList(dishIdsFromSignals(lowRepeatSignals),
                                  mealSamplesByDishId, 6),
            lowRepeatWeightHints);
        addSynthesizedHint(lowRepeatWeightHints, lowRepeatDishSupports,
                           lowRepeatInsight.value(QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("复吃意愿偏低"));
    }

    QVariantMap improvingInsight;
    if (!improvingSignals.isEmpty()) {
        const QVariantList improvingWeightHints = {
            buildWeightHint(
                QStringLiteral("preference.preference_score"),
                QStringLiteral("综合偏好回传"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "近期反馈明显变好，而且趋势比较稳定，综合偏好可以稍微更快跟上。"))};
        improvingInsight = buildInsight(
            QStringLiteral("improving_dishes"),
            QStringLiteral("近期转好"),
            QStringLiteral("这些菜在最近反馈里明显变好，可以继续观察是否成为新偏好"),
            formatDishSignalList(improvingSignals, dishNameById,
                                 QStringLiteral(" 改善"),
                                 QStringLiteral(" 近期")),
            QStringLiteral("good"),
            improvingDishSupports,
            gatherMealSupportList(dishIdsFromSignals(improvingSignals),
                                  mealSamplesByDishId, 6),
            improvingWeightHints);
        addSynthesizedHint(improvingWeightHints, improvingDishSupports,
                           improvingInsight.value(QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("近期转好"));
    }

    QVariantMap worseningInsight;
    if (!worseningSignals.isEmpty()) {
        const QVariantList worseningWeightHints = {
            buildWeightHint(
                QStringLiteral("group.preference_fit"),
                QStringLiteral("偏好信号总权重"),
                QStringLiteral("decrease"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "如果旧偏好还在前排，但最近反馈已转差，偏好总权重可以先稍微收一点。")),
            buildWeightHint(
                QStringLiteral("preference.repeat_willingness"),
                QStringLiteral("复吃意愿信号"),
                QStringLiteral("increase"),
                QStringLiteral("slight"),
                QStringLiteral(
                    "最近的复吃意愿已经转弱，当前回传速度还不够快，应该更早压下去。"))};
        worseningInsight = buildInsight(
            QStringLiteral("worsening_dishes"),
            QStringLiteral("近期转差"),
            QStringLiteral("这些菜最近在走弱，可能需要更快体现惩罚"),
            formatDishSignalList(worseningSignals, dishNameById,
                                 QStringLiteral(" 恶化"),
                                 QStringLiteral(" 近期")),
            QStringLiteral("risk"),
            worseningDishSupports,
            gatherMealSupportList(dishIdsFromSignals(worseningSignals),
                                  mealSamplesByDishId, 6),
            worseningWeightHints);
        addSynthesizedHint(worseningWeightHints, worseningDishSupports,
                           worseningInsight.value(QStringLiteral("supportingMeals"))
                               .toList(),
                           QStringLiteral("近期转差"));
    }

    if (!coverageInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, coverageInsight);
    }
    if (!recommendationHitsInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, recommendationHitsInsight);
    }
    if (!synthesizedWeightHints.isEmpty()) {
        QStringList topWeightKeys;
        for (const QVariant &hintVariant : synthesizedWeightHints) {
            const QString key =
                hintVariant.toMap().value(QStringLiteral("displayLabel")).toString();
            if (!key.isEmpty()) {
                topWeightKeys.append(key);
            }
            if (topWeightKeys.size() >= 3) {
                break;
            }
        }

        appendInsight(
            &m_feedbackInsights,
            buildInsight(
                QStringLiteral("weight_adjustment_suggestions"),
                QStringLiteral("调权方向建议"),
                QStringLiteral("目前已有 %1 类稳定模式，足以支持小步手动调权")
                    .arg(synthesizedPatternTitles.size()),
                QStringLiteral("建议先从 %1 这几项做小步调整；当前仍不会自动改权重。")
                    .arg(topWeightKeys.join(QStringLiteral("、"))),
                QStringLiteral("watch"),
                sortSupportDishesV2(synthesizedDishSamples, 5),
                sortSupportMealsV2(synthesizedMealSamples, 6),
                synthesizedWeightHints));
    }
    if (!contextInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, contextInsight);
    }
    if (!rankingMomentumInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, rankingMomentumInsight);
    }
    if (!stableFavoritesInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, stableFavoritesInsight);
    }
    if (!sleepinessWatchInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, sleepinessWatchInsight);
    }
    if (!lowRepeatInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, lowRepeatInsight);
    }
    if (!improvingInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, improvingInsight);
    }
    if (!worseningInsight.isEmpty()) {
        appendInsight(&m_feedbackInsights, worseningInsight);
    }

    refreshFilteredState();
}

void MealLogManager::refreshFilteredState()
{
    m_filteredAvailableDishes.clear();
    for (const QVariant &dishVariant : m_availableDishes) {
        const QVariantMap dish = dishVariant.toMap();
        if (matchesSearch(dish,
                          m_dishSearch,
                          {QStringLiteral("name"), QStringLiteral("merchantName"),
                           QStringLiteral("category"), QStringLiteral("notes"),
                           QStringLiteral("sleepinessRiskLevel")})) {
            m_filteredAvailableDishes.append(dish);
        }
    }
    sortAvailableDishMatches(&m_filteredAvailableDishes, m_dishSearch);
}

bool MealLogManager::matchesSearch(const QVariantMap &item,
                                   const QString &searchText,
                                   const QStringList &keys)
{
    if (searchText.trimmed().isEmpty()) {
        return true;
    }

    const QString loweredSearch = searchText.trimmed().toLower();
    for (const QString &key : keys) {
        if (item.value(key).toString().toLower().contains(loweredSearch)) {
            return true;
        }
    }

    return false;
}
