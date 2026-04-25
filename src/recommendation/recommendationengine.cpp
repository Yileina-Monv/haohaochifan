#include "recommendationengine.h"

#include "../core/appconfig.h"
#include "../core/domainmodels.h"
#include "../data/dishrepository.h"
#include "../data/mealfeedbackrepository.h"
#include "../data/meallogrepository.h"
#include "../data/merchantrepository.h"
#include "../data/planningrepository.h"
#include "../data/recommendationrecordrepository.h"
#include "../data/schedulerepository.h"

#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QTimer>
#include <QTime>
#include <QUrl>
#include <QVariantMap>

#include <algorithm>
#include <cmath>

namespace
{
using WeightMap = QHash<QString, double>;

constexpr double kValueCompareEpsilon = 0.0001;
constexpr int kSupplementTimeoutMs = 15000;

const QStringList kSupplementTopLevelKeys = {
    QStringLiteral("version"),
    QStringLiteral("result")
};

const QStringList kSupplementResultKeys = {
    QStringLiteral("hungerIntent"),
    QStringLiteral("carbIntent"),
    QStringLiteral("drinkIntent"),
    QStringLiteral("budgetFlexIntent"),
    QStringLiteral("classConstraintWeight"),
    QStringLiteral("postMealSleepPlan"),
    QStringLiteral("plannedNapMinutes"),
    QStringLiteral("sleepNeedLevel"),
    QStringLiteral("sleepPlanConfidence"),
    QStringLiteral("proteinIntent"),
    QStringLiteral("colaIntent"),
    QStringLiteral("flavorIntent"),
    QStringLiteral("relaxedTimePreference")
};

const QList<double> kWeakIntentValues = {
    0.75, 0.85, 0.95, 1.0, 1.1, 1.2, 1.35
};

const QList<double> kStrongIntentValues = {
    0.4, 0.5, 0.65, 0.8, 1.0, 1.25, 1.6, 2.0, 2.5
};

const QList<double> kGovernanceValues = {
    0.0, 0.25, 0.5, 0.75, 1.0
};

const QList<int> kNapMinuteValues = {
    0, 10, 15, 20, 30, 40, 45, 60, 90
};

const QStringList kSleepPlanValues = {
    QStringLiteral("stay_awake"),
    QStringLiteral("nap_before_class"),
    QStringLiteral("no_class"),
    QStringLiteral("unknown")
};

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) <= kValueCompareEpsilon;
}

bool containsAllowedValue(double value, const QList<double> &allowedValues)
{
    for (const double allowedValue : allowedValues) {
        if (nearlyEqual(value, allowedValue)) {
            return true;
        }
    }
    return false;
}

bool containsAllowedInt(int value, const QList<int> &allowedValues)
{
    return allowedValues.contains(value);
}

double weakIntentDelta(double multiplier)
{
    return multiplier - 1.0;
}

double strongIntentDelta(double multiplier)
{
    return multiplier - 1.0;
}

bool isNeutralValue(double value, double neutralValue)
{
    return nearlyEqual(value, neutralValue);
}

QDateTime effectiveCurrentDateTime()
{
    const QString overrideText =
        qEnvironmentVariable("MEALADVISOR_FIXED_NOW").trimmed();
    if (overrideText.isEmpty()) {
        return QDateTime::currentDateTime();
    }

    const QDateTime parsedIso = QDateTime::fromString(overrideText, Qt::ISODate);
    if (parsedIso.isValid()) {
        return parsedIso;
    }

    const QDateTime parsedSql =
        QDateTime::fromString(overrideText, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (parsedSql.isValid()) {
        return parsedSql;
    }

    return QDateTime::currentDateTime();
}

struct MealContext
{
    QDate targetDate;
    int weekday = 0;
    QString mealType;
    QString mealLabel;
    QDateTime mealDateTime;
    bool hasClassAfterMeal = false;
    int minutesUntilNextClass = 0;
    int nextClassPeriodStart = 0;
    PlanningPolicy policy;
    RecommendationProfile profile;
};

struct WeightProfile
{
    QString key;
    QString label;
    double sceneFit = 0.0;
    double nutritionFit = 0.0;
    double preferenceFit = 0.0;
    double diversityFit = 0.0;
    double budgetFit = 0.0;
    double intentFit = 0.0;
};

struct SleepPlanModifier
{
    QString label;
    double carbPenaltyMultiplier = 1.0;
    double sleepinessPenaltyMultiplier = 1.0;
    double totalTimePenaltyMultiplier = 1.0;
    int wakeBufferMinutes = 12;
};

struct MealSnapshot
{
    MealLog mealLog;
    QList<MealLogDishItem> dishItems;
};

struct MealAggregate
{
    bool valid = false;
    MealLog mealLog;
    QSet<int> dishIds;
    QSet<int> merchantIds;
    double carbLevel = 0.0;
    double fatLevel = 0.0;
    double proteinLevel = 0.0;
    double fiberLevel = 0.0;
    double vitaminLevel = 0.0;
    double vegetableLevel = 0.0;
    double satietyLevel = 0.0;
    double digestiveBurdenLevel = 0.0;
    double sleepinessRiskLevel = 0.0;
    bool highCarbBias = false;
    bool heavyBias = false;
    bool lightButUnsatisfiedBias = false;
};

struct HistoryInsights
{
    int sameDishCount = 0;
    int sameMerchantCount = 0;
    int consecutiveSameMerchantCount = 0;
    int consecutiveSameDishCount = 0;
    int highCarbMealCount = 0;
    int heavyMealCount = 0;
    int lightButUnsatisfiedMealCount = 0;
    int mealsConsidered = 0;
};

struct CandidateSignals
{
    double timeFit = 0.0;
    double classFit = 0.0;
    double diningModeFit = 0.0;
    double mealTypeFit = 0.0;

    double carbFit = 0.0;
    double fatFit = 0.0;
    double proteinSupport = 0.0;
    double fiberSupport = 0.0;
    double vitaminSupport = 0.0;
    double satietySupport = 0.0;
    double digestiveBurdenFit = 0.0;
    double sleepinessRiskFit = 0.0;
    double giFit = 0.0;

    double tasteRatingFit = 0.0;
    double repeatWillingnessFit = 0.0;
    double preferenceScoreFit = 0.0;

    double sameDishFit = 0.0;
    double sameMerchantFit = 0.0;
    double nutritionCompensationFit = 0.0;

    double priceFit = 0.0;
    double budgetPressureFit = 0.0;
    double acquireCostFit = 0.0;

    double hungerIntentFit = 0.0;
    double carbIntentFit = 0.0;
    double drinkIntentFit = 0.0;
    double budgetFlexIntentFit = 0.0;
    double skipClassConstraintFit = 0.0;

    double sceneFit = 0.0;
    double nutritionFit = 0.0;
    double preferenceFit = 0.0;
    double diversityFit = 0.0;
    double budgetFit = 0.0;
    double intentFit = 0.0;
    double totalScore = 0.0;

    double acquisitionMinutes = 0.0;
    double totalOccupiedMinutes = 0.0;
};

struct CandidateResult
{
    Dish dish;
    Merchant merchant;
    CandidateSignals metrics;
    QStringList reasons;
    QStringList warnings;
    QVariantList breakdown;
};

double clamp01(double value)
{
    return std::clamp(value, 0.0, 1.0);
}

double clampRange(double value, double minimumValue, double maximumValue)
{
    return std::clamp(value, minimumValue, maximumValue);
}

double positiveLevelScore(const QString &level)
{
    if (level == QStringLiteral("low")) {
        return 0.0;
    }
    if (level == QStringLiteral("medium")) {
        return 0.6;
    }
    if (level == QStringLiteral("high")) {
        return 1.0;
    }
    return 0.45;
}

double lowRiskScore(const QString &level)
{
    if (level == QStringLiteral("low")) {
        return 1.0;
    }
    if (level == QStringLiteral("medium")) {
        return 0.5;
    }
    if (level == QStringLiteral("high")) {
        return 0.0;
    }
    return 0.45;
}

double closenessScore(double actual, double target)
{
    return clamp01(1.0 - std::abs(actual - target));
}

double normalizedPreferenceSeed(int rawScore)
{
    if (rawScore <= 0) {
        return 0.55;
    }
    if (rawScore <= 5) {
        return clamp01(rawScore / 5.0);
    }
    if (rawScore <= 10) {
        return clamp01(rawScore / 10.0);
    }
    return clamp01(rawScore / 100.0);
}

double smoothedAverage(double baseline, double observed, int sampleCount, int baselineWeight = 2)
{
    if (sampleCount <= 0 || observed <= 0.0) {
        return clamp01(baseline);
    }

    const double boundedObserved = clamp01(observed);
    return clamp01((baseline * baselineWeight + boundedObserved * sampleCount) /
                   (baselineWeight + sampleCount));
}

double smoothedAverageWeighted(double baseline,
                               double observed,
                               double sampleWeight,
                               double baselineWeight = 2.0)
{
    if (sampleWeight <= 0.0 || observed <= 0.0) {
        return clamp01(baseline);
    }

    const double boundedObserved = clamp01(observed);
    return clamp01((baseline * baselineWeight + boundedObserved * sampleWeight) /
                   (baselineWeight + sampleWeight));
}

bool isActivePlanningDay(const PlanningPolicy &policy, int weekday)
{
    return policy.enabledWeekdays.contains(weekday);
}

QDate nextActiveDate(const PlanningPolicy &policy, const QDate &startDate)
{
    QDate cursor = startDate;
    for (int i = 0; i < 14; ++i) {
        if (isActivePlanningDay(policy, cursor.dayOfWeek())) {
            return cursor;
        }
        cursor = cursor.addDays(1);
    }
    return startDate;
}

QString weekdayLabel(int weekday)
{
    switch (weekday) {
    case 1:
        return QStringLiteral("周一");
    case 2:
        return QStringLiteral("周二");
    case 3:
        return QStringLiteral("周三");
    case 4:
        return QStringLiteral("周四");
    case 5:
        return QStringLiteral("周五");
    case 6:
        return QStringLiteral("周六");
    case 7:
        return QStringLiteral("周日");
    default:
        return QStringLiteral("第 %1 天").arg(weekday);
    }
}

QString mealLabel(const QString &mealType)
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

QString boolLabel(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString supplementParserSystemPrompt()
{
    return QStringLiteral(
        "You are MealAdvisor's supplement parser.\n\n"
        "Your only job is to convert one user's meal-related natural-language supplement into a strictly valid JSON object.\n\n"
        "You are NOT a chatbot.\n"
        "You are NOT a recommender.\n"
        "You do NOT explain your reasoning.\n"
        "You do NOT output markdown.\n"
        "You do NOT output code fences.\n"
        "You do NOT output natural language.\n"
        "You do NOT output extra keys.\n"
        "You do NOT omit required keys.\n"
        "You do NOT guess facts that are not supported by the input.\n\n"
        "You must always return exactly one JSON object with this shape:\n\n"
        "{\n"
        "  \"version\": \"supplement_parser_v1\",\n"
        "  \"result\": {\n"
        "    \"hungerIntent\": 1.0,\n"
        "    \"carbIntent\": 1.0,\n"
        "    \"drinkIntent\": 1.0,\n"
        "    \"budgetFlexIntent\": 1.0,\n"
        "    \"classConstraintWeight\": 1.0,\n"
        "    \"postMealSleepPlan\": \"unknown\",\n"
        "    \"plannedNapMinutes\": 0,\n"
        "    \"sleepNeedLevel\": 1.0,\n"
        "    \"sleepPlanConfidence\": 0.0,\n"
        "    \"proteinIntent\": 1.0,\n"
        "    \"colaIntent\": 1.0,\n"
        "    \"flavorIntent\": 1.0,\n"
        "    \"relaxedTimePreference\": 1.0\n"
        "  }\n"
        "}\n\n"
        "Rules:\n"
        "1. All keys are required.\n"
        "2. No extra keys are allowed.\n"
        "3. Only output JSON.\n"
        "4. No explanation, no comments, no markdown.\n\n"
        "Field constraints:\n\n"
        "Weak-judgment fields:\n"
        "- hungerIntent\n"
        "- carbIntent\n"
        "- drinkIntent\n"
        "- budgetFlexIntent\n"
        "- proteinIntent\n"
        "- colaIntent\n"
        "- flavorIntent\n\n"
        "Allowed values for weak-judgment fields:\n"
        "0.75, 0.85, 0.95, 1.0, 1.1, 1.2, 1.35\n\n"
        "Strong-judgment fields:\n"
        "- classConstraintWeight\n"
        "- sleepNeedLevel\n"
        "- relaxedTimePreference\n\n"
        "Allowed values for strong-judgment fields:\n"
        "0.4, 0.5, 0.65, 0.8, 1.0, 1.25, 1.6, 2.0, 2.5\n\n"
        "Governance field:\n"
        "- sleepPlanConfidence\n"
        "Allowed values:\n"
        "0.0, 0.25, 0.5, 0.75, 1.0\n\n"
        "Enum field:\n"
        "- postMealSleepPlan\n"
        "Allowed values:\n"
        "\"stay_awake\", \"nap_before_class\", \"no_class\", \"unknown\"\n\n"
        "Integer field:\n"
        "- plannedNapMinutes\n"
        "Allowed values:\n"
        "0, 10, 15, 20, 30, 40, 45, 60, 90\n\n"
        "Interpretation rules:\n"
        "- Strong-judgment fields are only for explicit scenario changes or explicit after-meal state changes.\n"
        "- Weak-judgment fields are only for ordinary preference signals.\n"
        "- Ordinary preference must not override hard scenario constraints.\n"
        "- If input is vague, use neutral defaults.\n"
        "- If the user explicitly says the situation changed, you may use strong-judgment values, including extreme values.\n"
        "- If there is not enough evidence, stay conservative.\n"
        "- If the user explicitly mentions class soon, rushing to class, or needing a steady meal before class, raise classConstraintWeight above 1.0 instead of leaving it neutral.\n"
        "- If the user explicitly says they need to stay awake or avoid drowsiness, set postMealSleepPlan to stay_awake and usually raise sleepNeedLevel above 1.0 when the wording is strong.\n"
        "- If the user explicitly says they will nap before class and gives a duration, set postMealSleepPlan to nap_before_class, fill plannedNapMinutes, and use high sleepPlanConfidence.\n"
        "- If the user explicitly says budget can be relaxed, raise budgetFlexIntent above 1.0; if the user explicitly wants cola, raise colaIntent above 1.0.\n\n"
        "Special priority rules:\n\n"
        "1. Explicit scenario change > ordinary preference\n"
        "2. Explicit stay-awake requirement > ordinary carb preference\n"
        "3. Explicit nap/rest plan > default class pressure\n"
        "4. If conflicting signals remain unresolved, stay conservative\n\n"
        "You must be stable.\n"
        "Do not invent unsupported urgency, class changes, nap plans, or budget changes.");
}

QString contextSummary(const MealContext &context)
{
    QStringList parts;
    parts.append(QStringLiteral("%1 %2")
                     .arg(context.targetDate.toString(QStringLiteral("yyyy-MM-dd")),
                          weekdayLabel(context.weekday)));
    parts.append(context.mealLabel);
    if (context.hasClassAfterMeal && context.minutesUntilNextClass > 0) {
        parts.append(QStringLiteral("距离下节课 %1 分钟")
                         .arg(context.minutesUntilNextClass));
    } else {
        parts.append(QStringLiteral("近期没有上课约束"));
    }
    return parts.join(QStringLiteral(", "));
}

QString supplementParserUserPrompt(const MealContext &context, const QString &userText)
{
    return QStringLiteral(
               "Parse the following meal supplement into the required JSON format.\n\n"
               "Current context:\n"
               "- mealType: %1\n"
               "- targetDate: %2\n"
               "- weekday: %3\n"
               "- hasClassAfterMeal: %4\n"
               "- minutesUntilNextClass: %5\n"
               "- currentContextSummary: %6\n\n"
               "User supplement text:\n"
               "%7\n\n"
               "Return JSON only.")
        .arg(context.mealType,
             context.targetDate.toString(QStringLiteral("yyyy-MM-dd")),
             weekdayLabel(context.weekday),
             boolLabel(context.hasClassAfterMeal),
             QString::number(context.minutesUntilNextClass),
             contextSummary(context),
             userText);
}

bool containsKeyword(const QString &text, const QStringList &keywords)
{
    const QString lower = text.toLower();
    for (const QString &keyword : keywords) {
        if (lower.contains(keyword)) {
            return true;
        }
    }
    return false;
}

bool isColaDish(const Dish &dish)
{
    const QString combinedText =
        dish.name + QStringLiteral(" ") + dish.notes + QStringLiteral(" ") + dish.category;
    return containsKeyword(combinedText,
                           {QStringLiteral("cola"),
                            QStringLiteral("coke"),
                            QStringLiteral("可乐")});
}

bool isBreakfastLikeDish(const Dish &dish)
{
    const QString combinedText =
        dish.name + QStringLiteral(" ") + dish.category + QStringLiteral(" ") + dish.notes;
    return containsKeyword(combinedText,
                           {QStringLiteral("早餐"),
                            QStringLiteral("粥"),
                            QStringLiteral("豆浆"),
                            QStringLiteral("包子"),
                            QStringLiteral("油条"),
                            QStringLiteral("麦满分"),
                            QStringLiteral("banana"),
                            QStringLiteral("breakfast")});
}

bool isSnackLikeDish(const Dish &dish)
{
    const QString combinedText =
        dish.name + QStringLiteral(" ") + dish.category + QStringLiteral(" ") + dish.notes;
    return dish.isBeverage ||
           containsKeyword(combinedText,
                           {QStringLiteral("snack"),
                            QStringLiteral("零食"),
                            QStringLiteral("奶茶"),
                            QStringLiteral("饮料"),
                            QStringLiteral("果茶"),
                            QStringLiteral("dessert"),
                            QStringLiteral("甜点")});
}

bool supportsDiningMode(const Dish &dish, const Merchant &merchant)
{
    if (dish.defaultDiningMode == QStringLiteral("delivery")) {
        return merchant.supportsDelivery;
    }
    if (dish.defaultDiningMode == QStringLiteral("takeaway")) {
        return merchant.supportsTakeaway;
    }
    return merchant.supportsDineIn;
}

QTime targetMealTime(const QString &mealType, const QList<ScheduleEntry> &entries)
{
    bool hasAfternoonClass = false;
    bool hasEveningClass = false;

    for (const ScheduleEntry &entry : entries) {
        if (entry.periodEnd >= 6 && entry.periodStart <= 10) {
            hasAfternoonClass = true;
        }
        if (entry.periodEnd >= 11) {
            hasEveningClass = true;
        }
    }

    if (mealType == QStringLiteral("breakfast")) {
        return QTime(8, 0);
    }
    if (mealType == QStringLiteral("lunch")) {
        return hasAfternoonClass ? QTime(12, 10) : QTime(13, 10);
    }
    if (mealType == QStringLiteral("dinner")) {
        return hasEveningClass ? QTime(18, 0) : QTime(19, 0);
    }

    return QTime(12, 0);
}

PlanningPolicy defaultPolicy(const QList<PlanningPolicy> &policies)
{
    if (!policies.isEmpty()) {
        return policies.first();
    }

    PlanningPolicy policy;
    policy.name = QStringLiteral("默认校园规划");
    policy.enabledWeekdays = {2, 3, 4, 5};
    policy.includeCommuteDays = true;
    policy.skipNonEnabledDays = true;
    policy.defaultDailyBudget = 80.0;
    policy.flexibleBudgetCap = 120.0;
    policy.breakfastRecommendationEnabled = false;
    return policy;
}

RecommendationProfile selectProfile(const QList<RecommendationProfile> &profiles,
                                    bool hasClassAfterMeal)
{
    if (hasClassAfterMeal) {
        for (const RecommendationProfile &profile : profiles) {
            if (profile.planningScope == QStringLiteral("class_day")) {
                return profile;
            }
        }
    }

    for (const RecommendationProfile &profile : profiles) {
        if (profile.isDefault) {
            return profile;
        }
    }

    if (!profiles.isEmpty()) {
        return profiles.first();
    }

    RecommendationProfile fallback;
    fallback.name = QStringLiteral("默认推荐策略");
    fallback.healthPriority = 8;
    fallback.budgetPriority = 4;
    fallback.timeEffortPriority = 7;
    fallback.satietyPriority = 6;
    fallback.carbControlPriority = 8;
    fallback.fatControlPriority = 6;
    fallback.proteinPriority = 6;
    fallback.sleepinessAvoidPriority = 8;
    fallback.hasClassPriority = 1;
    return fallback;
}

MealContext buildMealContext(const QList<PlanningPolicy> &policies,
                             const QList<RecommendationProfile> &profiles,
                             const ScheduleRepository &scheduleRepository)
{
    MealContext context;
    context.policy = defaultPolicy(policies);

    if (context.policy.enabledWeekdays.isEmpty()) {
        context.policy.enabledWeekdays = {2, 3, 4, 5};
    }

    const QDateTime now = effectiveCurrentDateTime();
    QDate targetDate = now.date();
    QString targetMealType;

    if (!isActivePlanningDay(context.policy, targetDate.dayOfWeek())) {
        targetDate = nextActiveDate(context.policy, targetDate.addDays(1));
        targetMealType = context.policy.breakfastRecommendationEnabled
                             ? QStringLiteral("breakfast")
                             : QStringLiteral("lunch");
    } else if (now.time() < QTime(10, 30)) {
        targetMealType = context.policy.breakfastRecommendationEnabled
                             ? QStringLiteral("breakfast")
                             : QStringLiteral("lunch");
    } else if (now.time() < QTime(16, 0)) {
        targetMealType = QStringLiteral("lunch");
    } else if (now.time() < QTime(21, 30)) {
        targetMealType = QStringLiteral("dinner");
    } else {
        targetDate = nextActiveDate(context.policy, targetDate.addDays(1));
        targetMealType = context.policy.breakfastRecommendationEnabled
                             ? QStringLiteral("breakfast")
                             : QStringLiteral("lunch");
    }

    const QList<ClassPeriod> periods = scheduleRepository.loadClassPeriods();
    QHash<int, ClassPeriod> periodByIndex;
    for (const ClassPeriod &period : periods) {
        periodByIndex.insert(period.periodIndex, period);
    }

    const QList<ScheduleEntry> dayEntries =
        scheduleRepository.loadEntriesForWeekday(targetDate.dayOfWeek());
    const QTime plannedTime = targetMealTime(targetMealType, dayEntries);
    const QDateTime plannedDateTime(targetDate, plannedTime);

    int minutesUntilNextClass = 0;
    bool hasClassAfterMeal = false;
    int nextClassPeriodStart = 0;

    for (const ScheduleEntry &entry : dayEntries) {
        const ClassPeriod startPeriod = periodByIndex.value(entry.periodStart);
        const QTime classStartTime =
            QTime::fromString(startPeriod.startTime, QStringLiteral("HH:mm"));

        if (!classStartTime.isValid()) {
            continue;
        }

        const QDateTime classStart(targetDate, classStartTime);
        const qint64 diffMinutes = plannedDateTime.secsTo(classStart) / 60;

        if (diffMinutes > 0 &&
            (!hasClassAfterMeal || diffMinutes < minutesUntilNextClass)) {
            hasClassAfterMeal = true;
            minutesUntilNextClass = static_cast<int>(diffMinutes);
            nextClassPeriodStart = entry.periodStart;
        }
    }

    context.targetDate = targetDate;
    context.weekday = targetDate.dayOfWeek();
    context.mealType = targetMealType;
    context.mealLabel = mealLabel(targetMealType);
    context.mealDateTime = plannedDateTime;
    context.hasClassAfterMeal = hasClassAfterMeal;
    context.minutesUntilNextClass = minutesUntilNextClass;
    context.nextClassPeriodStart = nextClassPeriodStart;
    context.profile = selectProfile(profiles, hasClassAfterMeal);
    return context;
}

double acquisitionMinutes(const Dish &dish, const Merchant &merchant)
{
    const int baseEatTime = std::max(8, dish.eatTimeMinutes);
    const int effortBuffer = std::max(0, dish.acquireEffortScore) * 4;

    if (dish.defaultDiningMode == QStringLiteral("delivery") &&
        merchant.supportsDelivery) {
        return baseEatTime + std::max(0, merchant.deliveryEtaMinutes);
    }

    if (dish.defaultDiningMode == QStringLiteral("takeaway") &&
        merchant.supportsTakeaway) {
        return baseEatTime + std::max(0, merchant.queueTimeMinutes) +
               std::max(0, merchant.distanceMinutes) * 0.6 + effortBuffer;
    }

    return baseEatTime + std::max(0, merchant.queueTimeMinutes) +
           std::max(0, merchant.distanceMinutes) + effortBuffer;
}

double safeMealWeight(const Dish &dish, const MealLogDishItem &dishItem)
{
    return std::max(0.15, dish.mealImpactWeight * std::max(0.2, dishItem.portionRatio));
}

QList<MealSnapshot> buildRecentMealSnapshots(const QList<MealLog> &recentMealLogs,
                                             const MealLogRepository &mealLogRepository,
                                             int mealWindow,
                                             bool excludeBreakfast)
{
    QList<MealSnapshot> snapshots;
    for (const MealLog &mealLog : recentMealLogs) {
        if (excludeBreakfast &&
            mealLog.mealType == QStringLiteral("breakfast")) {
            continue;
        }

        MealSnapshot snapshot;
        snapshot.mealLog = mealLog;
        snapshot.dishItems = mealLogRepository.loadMealLogDishItems(mealLog.id);
        if (snapshot.dishItems.isEmpty()) {
            continue;
        }

        snapshots.append(snapshot);
        if (snapshots.size() >= mealWindow) {
            break;
        }
    }
    return snapshots;
}

MealAggregate aggregateMeal(const MealSnapshot &snapshot,
                            const QHash<int, Dish> &dishById)
{
    MealAggregate aggregate;
    aggregate.mealLog = snapshot.mealLog;

    double totalWeight = 0.0;
    for (const MealLogDishItem &dishItem : snapshot.dishItems) {
        if (!dishById.contains(dishItem.dishId)) {
            continue;
        }

        const Dish dish = dishById.value(dishItem.dishId);
        const double itemWeight = safeMealWeight(dish, dishItem);
        totalWeight += itemWeight;

        aggregate.dishIds.insert(dish.id);
        if (dish.merchantId > 0) {
            aggregate.merchantIds.insert(dish.merchantId);
        }

        aggregate.carbLevel += positiveLevelScore(dish.carbLevel) * itemWeight;
        aggregate.fatLevel += positiveLevelScore(dish.fatLevel) * itemWeight;
        aggregate.proteinLevel += positiveLevelScore(dish.proteinLevel) * itemWeight;
        aggregate.fiberLevel += positiveLevelScore(dish.fiberLevel) * itemWeight;
        aggregate.vitaminLevel += positiveLevelScore(dish.vitaminLevel) * itemWeight;
        aggregate.vegetableLevel += positiveLevelScore(dish.vegetableLevel) * itemWeight;
        aggregate.satietyLevel += positiveLevelScore(dish.satietyLevel) * itemWeight;
        aggregate.digestiveBurdenLevel += positiveLevelScore(dish.digestiveBurdenLevel) * itemWeight;
        aggregate.sleepinessRiskLevel += positiveLevelScore(dish.sleepinessRiskLevel) * itemWeight;
    }

    if (totalWeight <= 0.0) {
        return aggregate;
    }

    aggregate.valid = true;
    aggregate.carbLevel /= totalWeight;
    aggregate.fatLevel /= totalWeight;
    aggregate.proteinLevel /= totalWeight;
    aggregate.fiberLevel /= totalWeight;
    aggregate.vitaminLevel /= totalWeight;
    aggregate.vegetableLevel /= totalWeight;
    aggregate.satietyLevel /= totalWeight;
    aggregate.digestiveBurdenLevel /= totalWeight;
    aggregate.sleepinessRiskLevel /= totalWeight;
    aggregate.highCarbBias = aggregate.carbLevel >= 0.65;
    aggregate.heavyBias = aggregate.fatLevel >= 0.65 ||
                          aggregate.digestiveBurdenLevel >= 0.65;
    aggregate.lightButUnsatisfiedBias = aggregate.satietyLevel <= 0.35 ||
                                        aggregate.proteinLevel <= 0.35;
    return aggregate;
}

HistoryInsights analyzeHistory(const Dish &candidateDish,
                               const QList<MealAggregate> &recentMeals)
{
    HistoryInsights insights;
    insights.mealsConsidered = recentMeals.size();

    for (int i = 0; i < recentMeals.size(); ++i) {
        const MealAggregate &meal = recentMeals.at(i);
        if (!meal.valid) {
            continue;
        }

        if (meal.dishIds.contains(candidateDish.id)) {
            insights.sameDishCount += 1;
            if (i == insights.consecutiveSameDishCount) {
                insights.consecutiveSameDishCount += 1;
            }
        }

        if (candidateDish.merchantId > 0 &&
            meal.merchantIds.contains(candidateDish.merchantId)) {
            insights.sameMerchantCount += 1;
            if (i == insights.consecutiveSameMerchantCount) {
                insights.consecutiveSameMerchantCount += 1;
            }
        }

        if (meal.highCarbBias) {
            insights.highCarbMealCount += 1;
        }
        if (meal.heavyBias) {
            insights.heavyMealCount += 1;
        }
        if (meal.lightButUnsatisfiedBias) {
            insights.lightButUnsatisfiedMealCount += 1;
        }
    }

    return insights;
}

double readOverride(const QVariantMap &overrides,
                    const QString &key,
                    double defaultValue)
{
    const QVariant value = overrides.value(key);
    return value.canConvert<double>() ? value.toDouble() : defaultValue;
}

WeightProfile chooseWeightProfile(const MealContext &context, bool classPressure)
{
    WeightProfile profile;
    if (context.mealType == QStringLiteral("dinner")) {
        profile.key = QStringLiteral("dinner");
        profile.label = QStringLiteral("晚餐");
        profile.sceneFit = 20.0;
        profile.nutritionFit = 20.0;
        profile.preferenceFit = 25.0;
        profile.diversityFit = 20.0;
        profile.budgetFit = 10.0;
        profile.intentFit = 5.0;
        return profile;
    }

    if (context.mealType == QStringLiteral("lunch") && classPressure) {
        profile.key = QStringLiteral("class_lunch");
        profile.label = QStringLiteral("120 分钟内有课午餐");
        profile.sceneFit = 30.0;
        profile.nutritionFit = 25.0;
        profile.preferenceFit = 15.0;
        profile.diversityFit = 15.0;
        profile.budgetFit = 10.0;
        profile.intentFit = 5.0;
        return profile;
    }

    profile.key = QStringLiteral("normal_daytime");
    profile.label = QStringLiteral("普通白天");
    profile.sceneFit = 25.0;
    profile.nutritionFit = 25.0;
    profile.preferenceFit = 20.0;
    profile.diversityFit = 15.0;
    profile.budgetFit = 10.0;
    profile.intentFit = 5.0;
    return profile;
}

WeightMap buildResolvedWeights(const MealContext &context,
                               bool classPressure,
                               const QVariantMap &overrides)
{
    const WeightProfile profile = chooseWeightProfile(context, classPressure);

    WeightMap weights;
    weights.insert(QStringLiteral("group.scene_fit"),
                   readOverride(overrides, QStringLiteral("group.scene_fit"),
                                profile.sceneFit));
    weights.insert(QStringLiteral("group.nutrition_fit"),
                   readOverride(overrides, QStringLiteral("group.nutrition_fit"),
                                profile.nutritionFit));
    weights.insert(QStringLiteral("group.preference_fit"),
                   readOverride(overrides, QStringLiteral("group.preference_fit"),
                                profile.preferenceFit));
    weights.insert(QStringLiteral("group.diversity_fit"),
                   readOverride(overrides, QStringLiteral("group.diversity_fit"),
                                profile.diversityFit));
    weights.insert(QStringLiteral("group.budget_fit"),
                   readOverride(overrides, QStringLiteral("group.budget_fit"),
                                profile.budgetFit));
    weights.insert(QStringLiteral("group.intent_fit"),
                   readOverride(overrides, QStringLiteral("group.intent_fit"),
                                profile.intentFit));

    weights.insert(QStringLiteral("scene.time_fit"),
                   readOverride(overrides, QStringLiteral("scene.time_fit"), 0.36));
    weights.insert(QStringLiteral("scene.class_fit"),
                   readOverride(overrides, QStringLiteral("scene.class_fit"), 0.34));
    weights.insert(QStringLiteral("scene.dining_mode_fit"),
                   readOverride(overrides, QStringLiteral("scene.dining_mode_fit"), 0.15));
    weights.insert(QStringLiteral("scene.meal_type_fit"),
                   readOverride(overrides, QStringLiteral("scene.meal_type_fit"), 0.15));

    weights.insert(QStringLiteral("nutrition.carb_fit"),
                   readOverride(overrides, QStringLiteral("nutrition.carb_fit"), 0.19));
    weights.insert(QStringLiteral("nutrition.fat_fit"),
                   readOverride(overrides, QStringLiteral("nutrition.fat_fit"), 0.09));
    weights.insert(QStringLiteral("nutrition.protein_support"),
                   readOverride(overrides, QStringLiteral("nutrition.protein_support"), 0.12));
    weights.insert(QStringLiteral("nutrition.fiber_support"),
                   readOverride(overrides, QStringLiteral("nutrition.fiber_support"), 0.12));
    weights.insert(QStringLiteral("nutrition.vitamin_support"),
                   readOverride(overrides, QStringLiteral("nutrition.vitamin_support"), 0.09));
    weights.insert(QStringLiteral("nutrition.satiety_support"),
                   readOverride(overrides, QStringLiteral("nutrition.satiety_support"), 0.11));
    weights.insert(QStringLiteral("nutrition.digestive_burden_fit"),
                   readOverride(overrides, QStringLiteral("nutrition.digestive_burden_fit"), 0.13));
    weights.insert(QStringLiteral("nutrition.sleepiness_risk_fit"),
                   readOverride(overrides, QStringLiteral("nutrition.sleepiness_risk_fit"), 0.10));
    weights.insert(QStringLiteral("nutrition.gi_fit"),
                   readOverride(overrides, QStringLiteral("nutrition.gi_fit"), 0.05));

    weights.insert(QStringLiteral("preference.taste_rating"),
                   readOverride(overrides, QStringLiteral("preference.taste_rating"), 0.45));
    weights.insert(QStringLiteral("preference.repeat_willingness"),
                   readOverride(overrides, QStringLiteral("preference.repeat_willingness"), 0.20));
    weights.insert(QStringLiteral("preference.preference_score"),
                   readOverride(overrides, QStringLiteral("preference.preference_score"), 0.35));

    weights.insert(QStringLiteral("diversity.same_dish_penalty"),
                   readOverride(overrides, QStringLiteral("diversity.same_dish_penalty"), 0.42));
    weights.insert(QStringLiteral("diversity.same_merchant_penalty"),
                   readOverride(overrides, QStringLiteral("diversity.same_merchant_penalty"), 0.20));
    weights.insert(QStringLiteral("diversity.nutrition_compensation"),
                   readOverride(overrides, QStringLiteral("diversity.nutrition_compensation"), 0.38));

    weights.insert(QStringLiteral("budget.price_fit"),
                   readOverride(overrides, QStringLiteral("budget.price_fit"), 0.45));
    weights.insert(QStringLiteral("budget.budget_pressure"),
                   readOverride(overrides, QStringLiteral("budget.budget_pressure"), 0.20));
    weights.insert(QStringLiteral("budget.acquire_cost_fit"),
                   readOverride(overrides, QStringLiteral("budget.acquire_cost_fit"), 0.35));

    weights.insert(QStringLiteral("intent.hunger_intent"),
                   readOverride(overrides, QStringLiteral("intent.hunger_intent"), 0.30));
    weights.insert(QStringLiteral("intent.carb_intent"),
                   readOverride(overrides, QStringLiteral("intent.carb_intent"), 0.22));
    weights.insert(QStringLiteral("intent.drink_intent"),
                   readOverride(overrides, QStringLiteral("intent.drink_intent"), 0.18));
    weights.insert(QStringLiteral("intent.budget_flex_intent"),
                   readOverride(overrides, QStringLiteral("intent.budget_flex_intent"), 0.12));
    weights.insert(QStringLiteral("intent.skip_class_constraint"),
                   readOverride(overrides, QStringLiteral("intent.skip_class_constraint"), 0.18));

    weights.insert(QStringLiteral("config.recent_non_breakfast_meal_window"),
                   readOverride(overrides,
                                QStringLiteral("config.recent_non_breakfast_meal_window"),
                                6.0));
    weights.insert(QStringLiteral("config.nutrition_compensation_window"),
                   readOverride(overrides,
                                QStringLiteral("config.nutrition_compensation_window"),
                                4.0));
    weights.insert(QStringLiteral("config.exclude_breakfast"),
                   readOverride(overrides, QStringLiteral("config.exclude_breakfast"), 1.0));
    return weights;
}

QVariantList buildWeightConfigList(const MealContext &context,
                                   bool classPressure,
                                   const WeightMap &weights,
                                   const SleepPlanModifier &sleepModifier)
{
    QVariantList config;

    const auto appendItem = [&config](const QString &section,
                                      const QString &key,
                                      const QString &label,
                                      const QVariant &value) {
        QVariantMap item;
        item.insert(QStringLiteral("section"), section);
        item.insert(QStringLiteral("key"), key);
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("value"), value);
        config.append(item);
    };

    appendItem(QStringLiteral("profile"), QStringLiteral("profile.meal_type"),
               QStringLiteral("当前餐次"), mealLabel(context.mealType));
    appendItem(QStringLiteral("profile"), QStringLiteral("profile.weight_profile"),
               QStringLiteral("权重场景"),
               chooseWeightProfile(context, classPressure).label);
    appendItem(QStringLiteral("group"), QStringLiteral("group.scene_fit"),
               QStringLiteral("场景适配"), weights.value(QStringLiteral("group.scene_fit")));
    appendItem(QStringLiteral("group"), QStringLiteral("group.nutrition_fit"),
               QStringLiteral("营养平衡"), weights.value(QStringLiteral("group.nutrition_fit")));
    appendItem(QStringLiteral("group"), QStringLiteral("group.preference_fit"),
               QStringLiteral("个人偏好"), weights.value(QStringLiteral("group.preference_fit")));
    appendItem(QStringLiteral("group"), QStringLiteral("group.diversity_fit"),
               QStringLiteral("多餐补偿"), weights.value(QStringLiteral("group.diversity_fit")));
    appendItem(QStringLiteral("group"), QStringLiteral("group.budget_fit"),
               QStringLiteral("预算适配"), weights.value(QStringLiteral("group.budget_fit")));
    appendItem(QStringLiteral("group"), QStringLiteral("group.intent_fit"),
               QStringLiteral("当前需求"), weights.value(QStringLiteral("group.intent_fit")));
    appendItem(QStringLiteral("scene"), QStringLiteral("scene.time_fit"),
               QStringLiteral("时间可行性"), weights.value(QStringLiteral("scene.time_fit")));
    appendItem(QStringLiteral("scene"), QStringLiteral("scene.class_fit"),
               QStringLiteral("课前适配"), weights.value(QStringLiteral("scene.class_fit")));
    appendItem(QStringLiteral("nutrition"), QStringLiteral("nutrition.carb_fit"),
               QStringLiteral("碳水适配"), weights.value(QStringLiteral("nutrition.carb_fit")));
    appendItem(QStringLiteral("nutrition"), QStringLiteral("nutrition.sleepiness_risk_fit"),
               QStringLiteral("困倦风险"), weights.value(QStringLiteral("nutrition.sleepiness_risk_fit")));
    appendItem(QStringLiteral("diversity"), QStringLiteral("config.recent_non_breakfast_meal_window"),
               QStringLiteral("多餐窗口"), weights.value(QStringLiteral("config.recent_non_breakfast_meal_window")));
    appendItem(QStringLiteral("sleep_plan"), QStringLiteral("sleep_plan.mode"),
               QStringLiteral("饭后安排"), sleepModifier.label);
    appendItem(QStringLiteral("sleep_plan"), QStringLiteral("sleep_plan.carb_multiplier"),
               QStringLiteral("碳水惩罚系数"), sleepModifier.carbPenaltyMultiplier);
    appendItem(QStringLiteral("sleep_plan"), QStringLiteral("sleep_plan.sleepiness_multiplier"),
               QStringLiteral("困倦惩罚系数"), sleepModifier.sleepinessPenaltyMultiplier);
    appendItem(QStringLiteral("sleep_plan"), QStringLiteral("sleep_plan.total_time_multiplier"),
               QStringLiteral("总耗时惩罚系数"), sleepModifier.totalTimePenaltyMultiplier);
    appendItem(QStringLiteral("sleep_plan"), QStringLiteral("sleep_plan.wake_buffer_minutes"),
               QStringLiteral("起床缓冲分钟"), sleepModifier.wakeBufferMinutes);

    return config;
}

SleepPlanModifier resolveSleepPlanModifier(
    const RecommendationEngine::SupplementAdjustment &adjustment)
{
    SleepPlanModifier modifier;
    const double sleepNeedStrength =
        clamp01(0.5 + strongIntentDelta(adjustment.sleepNeedLevel) / 1.5);
    const double signalStrength =
        clamp01((sleepNeedStrength + adjustment.sleepPlanConfidence) / 2.0);

    if (adjustment.postMealSleepPlan == QStringLiteral("stay_awake")) {
        modifier.label = QStringLiteral("饭后保持清醒");
        modifier.carbPenaltyMultiplier = 1.10 + signalStrength * 0.70;
        modifier.sleepinessPenaltyMultiplier = 1.15 + signalStrength * 0.85;
        modifier.totalTimePenaltyMultiplier = 1.00 + signalStrength * 0.20;
        modifier.wakeBufferMinutes = 10;
        return modifier;
    }

    if (adjustment.postMealSleepPlan == QStringLiteral("nap_before_class")) {
        modifier.label = QStringLiteral("饭后先小睡再上课");
        modifier.carbPenaltyMultiplier = 0.95 - signalStrength * 0.25;
        modifier.sleepinessPenaltyMultiplier = 0.95 - signalStrength * 0.20;
        modifier.totalTimePenaltyMultiplier = 1.15 + signalStrength * 0.75;
        modifier.wakeBufferMinutes = 15;
        return modifier;
    }

    if (adjustment.postMealSleepPlan == QStringLiteral("no_class")) {
        modifier.label = QStringLiteral("饭后无课");
        modifier.carbPenaltyMultiplier = 0.85;
        modifier.sleepinessPenaltyMultiplier = 0.80;
        modifier.totalTimePenaltyMultiplier = 0.92;
        modifier.wakeBufferMinutes = 12;
        return modifier;
    }

    modifier.label = QStringLiteral("未知");
    return modifier;
}

double weightedScore(const QList<QPair<double, double>> &values)
{
    double totalWeight = 0.0;
    double totalScore = 0.0;
    for (const auto &pair : values) {
        totalScore += pair.first * pair.second;
        totalWeight += pair.second;
    }

    if (totalWeight <= 0.0) {
        return 55.0;
    }

    return (totalScore / totalWeight) * 100.0;
}

void appendUnique(QStringList *list, const QString &text)
{
    if (list != nullptr && !text.isEmpty() && !list->contains(text)) {
        list->append(text);
    }
}

QVariantList buildBreakdownList(const CandidateSignals &metrics)
{
    QVariantList breakdown;

    const auto appendItem = [&breakdown](const QString &label, double score) {
        QVariantMap item;
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("score"), QString::number(score, 'f', 1));
        breakdown.append(item);
    };

    appendItem(QStringLiteral("场景适配"), metrics.sceneFit);
    appendItem(QStringLiteral("营养平衡"), metrics.nutritionFit);
    appendItem(QStringLiteral("个人偏好"), metrics.preferenceFit);
    appendItem(QStringLiteral("多餐补偿"), metrics.diversityFit);
    appendItem(QStringLiteral("预算适配"), metrics.budgetFit);
    appendItem(QStringLiteral("当前需求"), metrics.intentFit);
    appendItem(QStringLiteral("综合得分"), metrics.totalScore);

    return breakdown;
}

QString extractTextFromMessageContent(const QJsonValue &contentValue)
{
    if (contentValue.isString()) {
        return contentValue.toString().trimmed();
    }

    if (contentValue.isArray()) {
        QStringList parts;
        const QJsonArray partsArray = contentValue.toArray();
        for (const QJsonValue &partValue : partsArray) {
            if (!partValue.isObject()) {
                continue;
            }
            const QJsonObject partObject = partValue.toObject();
            const QString type = partObject.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("text") ||
                type == QStringLiteral("output_text")) {
                parts.append(partObject.value(QStringLiteral("text")).toString());
            }
        }
        return parts.join(QString()).trimmed();
    }

    return QString();
}

bool parseStrictJsonObject(const QByteArray &payload, QJsonObject *object)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return false;
    }

    if (object != nullptr) {
        *object = document.object();
    }
    return true;
}

bool validateExactKeys(const QJsonObject &object,
                       const QStringList &expectedKeys,
                       QString *error)
{
    if (object.keys().size() != expectedKeys.size()) {
        if (error != nullptr) {
            *error = QStringLiteral("字段数量不符合约定");
        }
        return false;
    }

    for (const QString &expectedKey : expectedKeys) {
        if (!object.contains(expectedKey)) {
            if (error != nullptr) {
                *error = QStringLiteral("缺少字段：%1").arg(expectedKey);
            }
            return false;
        }
    }

    for (const QString &actualKey : object.keys()) {
        if (!expectedKeys.contains(actualKey)) {
            if (error != nullptr) {
                *error = QStringLiteral("存在额外字段：%1").arg(actualKey);
            }
            return false;
        }
    }

    return true;
}

bool extractContractObjectFromApiResponse(const QByteArray &payload,
                                          QJsonObject *contractObject,
                                          QString *error)
{
    QJsonObject rootObject;
    if (!parseStrictJsonObject(payload, &rootObject)) {
        if (error != nullptr) {
            *error = QStringLiteral("响应正文不是 JSON");
        }
        return false;
    }

    if (rootObject.contains(QStringLiteral("version")) &&
        rootObject.contains(QStringLiteral("result"))) {
        if (contractObject != nullptr) {
            *contractObject = rootObject;
        }
        return true;
    }

    const QJsonArray choices = rootObject.value(QStringLiteral("choices")).toArray();
    if (!choices.isEmpty()) {
        const QJsonObject firstChoice = choices.first().toObject();
        const QJsonObject messageObject =
            firstChoice.value(QStringLiteral("message")).toObject();
        const QJsonValue parsedValue = messageObject.value(QStringLiteral("parsed"));
        if (parsedValue.isObject()) {
            if (contractObject != nullptr) {
                *contractObject = parsedValue.toObject();
            }
            return true;
        }

        const QString content =
            extractTextFromMessageContent(messageObject.value(QStringLiteral("content")));
        QJsonObject parsedContentObject;
        if (parseStrictJsonObject(content.toUtf8(), &parsedContentObject)) {
            if (contractObject != nullptr) {
                *contractObject = parsedContentObject;
            }
            return true;
        }

        if (error != nullptr) {
            *error = QStringLiteral("消息内容不是严格 JSON");
        }
        return false;
    }

    const QJsonArray outputArray = rootObject.value(QStringLiteral("output")).toArray();
    if (!outputArray.isEmpty()) {
        const QJsonObject firstOutput = outputArray.first().toObject();
        const QJsonArray contentArray =
            firstOutput.value(QStringLiteral("content")).toArray();
        for (const QJsonValue &contentValue : contentArray) {
            const QJsonObject contentObject = contentValue.toObject();
            const QString text = contentObject.value(QStringLiteral("text")).toString().trimmed();
            QJsonObject parsedContentObject;
            if (!text.isEmpty() &&
                parseStrictJsonObject(text.toUtf8(), &parsedContentObject)) {
                if (contractObject != nullptr) {
                    *contractObject = parsedContentObject;
                }
                return true;
            }
        }
        if (error != nullptr) {
            *error = QStringLiteral("输出内容不是严格 JSON");
        }
        return false;
    }

    if (error != nullptr) {
        *error = QStringLiteral("响应中没有可用的 chat completion 消息");
    }
    return false;
}

bool readAllowedDouble(const QJsonObject &object,
                       const QString &key,
                       const QList<double> &allowedValues,
                       double *value,
                       QString *error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble()) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 必须是数字").arg(key);
        }
        return false;
    }

    const double parsedValue = jsonValue.toDouble();
    if (!containsAllowedValue(parsedValue, allowedValues)) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 的取值不受支持").arg(key);
        }
        return false;
    }

    if (value != nullptr) {
        *value = parsedValue;
    }
    return true;
}

bool readAllowedInt(const QJsonObject &object,
                    const QString &key,
                    const QList<int> &allowedValues,
                    int *value,
                    QString *error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble()) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 必须是整数").arg(key);
        }
        return false;
    }

    const double parsedDouble = jsonValue.toDouble();
    const int parsedValue = static_cast<int>(std::round(parsedDouble));
    if (!nearlyEqual(parsedDouble, parsedValue)) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 必须是整数").arg(key);
        }
        return false;
    }

    if (!containsAllowedInt(parsedValue, allowedValues)) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 的取值不受支持").arg(key);
        }
        return false;
    }

    if (value != nullptr) {
        *value = parsedValue;
    }
    return true;
}

bool readAllowedString(const QJsonObject &object,
                       const QString &key,
                       const QStringList &allowedValues,
                       QString *value,
                       QString *error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isString()) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 必须是字符串").arg(key);
        }
        return false;
    }

    const QString parsedValue = jsonValue.toString().trimmed();
    if (!allowedValues.contains(parsedValue)) {
        if (error != nullptr) {
        *error = QStringLiteral("%1 的取值不受支持").arg(key);
        }
        return false;
    }

    if (value != nullptr) {
        *value = parsedValue;
    }
    return true;
}

QStringList buildReasons(const Dish &dish,
                         const MealContext &context,
                         const RecommendationEngine::SupplementAdjustment &adjustment,
                         const CandidateSignals &metrics,
                         const DishFeedbackAggregate &feedback,
                         const HistoryInsights &history,
                         const SleepPlanModifier &sleepModifier,
                         bool classPressureWasRelaxed,
                         double mealBudget,
                         double flexibleMealBudget)
{
    QStringList reasons;
    const bool feedbackStrong =
        feedback.effectiveSampleWeight >= 1.6 || feedback.recentSampleCount >= 2;

    if (context.hasClassAfterMeal && metrics.classFit >= 78.0) {
        appendUnique(&reasons, QStringLiteral("课前场景下更稳，饭后犯困和负担风险更低。"));
    }

    if (feedbackStrong && feedback.avgTasteRating >= 4.0) {
        appendUnique(&reasons, QStringLiteral("历史反馈里这道菜的口味接受度较高。"));
    }
    if (feedbackStrong && feedback.avgRepeatWillingness >= 4.0) {
        appendUnique(&reasons, QStringLiteral("从历史反馈看，你近期更愿意再次吃它。"));
    }

    if (adjustment.postMealSleepPlan == QStringLiteral("nap_before_class") &&
        metrics.timeFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("按饭后小睡计划计算，总占用时间仍在可接受范围内。"));
    } else if (adjustment.postMealSleepPlan == QStringLiteral("stay_awake") &&
               metrics.timeFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("饭后不睡直接继续安排时，整体耗时更可控。"));
    }

    if (metrics.carbFit >= 72.0 &&
        metrics.proteinSupport >= 68.0 &&
        metrics.fiberSupport >= 68.0) {
        appendUnique(&reasons, QStringLiteral("碳水更稳，蛋白质和纤维支撑也更好。"));
    } else if (metrics.nutritionFit >= 74.0) {
        appendUnique(&reasons, QStringLiteral("营养结构更均衡，整体更适合当前这一餐。"));
    }

    if (history.sameDishCount == 0 && history.sameMerchantCount <= 1 &&
        metrics.diversityFit >= 68.0) {
        appendUnique(&reasons, QStringLiteral("能避开最近几餐的重复，整体更有变化。"));
    } else if (metrics.nutritionCompensationFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("最近几餐如果偏重，这一项更像是在做补偿。"));
    }

    if (dish.price <= mealBudget && metrics.budgetFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("价格落在本餐预算内，获取成本也比较顺手。"));
    } else if (dish.price <= flexibleMealBudget && metrics.budgetFit >= 64.0) {
        appendUnique(&reasons, QStringLiteral("价格略高但仍在弹性预算范围内。"));
    }

    if (adjustment.hungerIntent >= 0.35 && metrics.hungerIntentFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("更贴合你当前偏饿、想吃得更顶一点的需求。"));
    }
    if (adjustment.drinkIntent >= 0.35 && metrics.drinkIntentFit >= 78.0) {
        appendUnique(&reasons, QStringLiteral("和你这次补充说明里的饮品倾向比较匹配。"));
    }
    if (adjustment.carbIntent >= 0.35 && metrics.carbIntentFit >= 72.0) {
        appendUnique(&reasons, QStringLiteral("对这次想吃点碳水的需求响应更自然。"));
    }

    if (classPressureWasRelaxed) {
        appendUnique(&reasons, QStringLiteral("已按补充说明放宽上课约束，不再把课前压力拉满。"));
    }

    if (metrics.sceneFit >= 78.0 && reasons.size() < 3) {
        appendUnique(&reasons, QStringLiteral("当前时段下，时间、就餐方式和餐次适配度都更稳。"));
    }

    if (sleepModifier.label != QStringLiteral("未知") && reasons.size() < 3) {
        appendUnique(&reasons,
                     QStringLiteral("饭后安排按“%1”做了额外修正。").arg(sleepModifier.label));
    }

    if (reasons.isEmpty()) {
        appendUnique(&reasons, QStringLiteral("综合场景、营养、预算和多餐补偿后，这一项更均衡。"));
    }

    return reasons.mid(0, 3);
}

QStringList buildWarnings(const Dish &dish,
                          const MealContext &context,
                          const RecommendationEngine::SupplementAdjustment &adjustment,
                          const CandidateSignals &metrics,
                          const DishFeedbackAggregate &feedback,
                          const SleepPlanModifier &sleepModifier,
                          double flexibleMealBudget)
{
    QStringList warnings;
    const bool feedbackStrong =
        feedback.effectiveSampleWeight >= 1.6 || feedback.recentSampleCount >= 2;

    if (context.mealType == QStringLiteral("dinner") &&
        positiveLevelScore(dish.sleepinessRiskLevel) >= 0.6) {
        appendUnique(&warnings, QStringLiteral("晚餐后可能更容易犯困。"));
    }

    if (positiveLevelScore(dish.digestiveBurdenLevel) >= 0.8) {
        appendUnique(&warnings, QStringLiteral("整体负担偏重，饭后可能更沉。"));
    }

    if (feedbackStrong && feedback.avgSleepinessLevel >= 4.0) {
        appendUnique(&warnings, QStringLiteral("历史反馈显示这道菜吃完后更容易犯困。"));
    }
    if (feedbackStrong && feedback.avgFocusImpactLevel <= 2.5 &&
        feedback.avgFocusImpactLevel > 0.0) {
        appendUnique(&warnings, QStringLiteral("历史反馈里它对后续专注状态不太友好。"));
    }

    if (positiveLevelScore(dish.odorLevel) >= 0.8) {
        appendUnique(&warnings, QStringLiteral("气味偏重，若饭后回宿舍或室内活动会更明显。"));
    }

    if (adjustment.postMealSleepPlan == QStringLiteral("stay_awake") &&
        context.hasClassAfterMeal &&
        positiveLevelScore(dish.carbLevel) >= 0.6 &&
        (positiveLevelScore(dish.sleepinessRiskLevel) >= 0.6 ||
         metrics.acquisitionMinutes >= 35.0)) {
        appendUnique(&warnings,
                     QStringLiteral("饭后不睡直接继续安排时，这类高碳组合更容易拖慢状态。"));
    }

    if (adjustment.postMealSleepPlan == QStringLiteral("nap_before_class") &&
        context.hasClassAfterMeal &&
        metrics.totalOccupiedMinutes > context.minutesUntilNextClass) {
        appendUnique(&warnings, QStringLiteral("按计划先小睡的话，总占用时间可能顶到下节课。"));
    }

    if (metrics.timeFit <= 45.0) {
        appendUnique(&warnings, QStringLiteral("这一项在当前时段的时间可行性一般。"));
    }

    if (dish.price > flexibleMealBudget) {
        appendUnique(&warnings, QStringLiteral("价格已经超过本餐弹性预算。"));
    }

    if (sleepModifier.totalTimePenaltyMultiplier >= 1.5 &&
        adjustment.plannedNapMinutes >= 45 &&
        context.hasClassAfterMeal) {
        appendUnique(&warnings, QStringLiteral("这次饭后小睡计划较长，时间风险会被进一步放大。"));
    }

    return warnings;
}

QVariantList buildWeightList(
    const RecommendationEngine::SupplementAdjustment &adjustment)
{
    QVariantList weights;

    const auto appendNormalizedWeight = [&weights](const QString &label,
                                                   const QString &value) {
        QVariantMap item;
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("value"), value);
        weights.append(item);
    };

    if (!isNeutralValue(adjustment.hungerIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("饥饿倾向"),
                               QString::number(adjustment.hungerIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.carbIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("碳水倾向"),
                               QString::number(adjustment.carbIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.drinkIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("饮品倾向"),
                               QString::number(adjustment.drinkIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.budgetFlexIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("预算放宽"),
                               QString::number(adjustment.budgetFlexIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.classConstraintWeight, 1.0)) {
        appendNormalizedWeight(QStringLiteral("课程约束"),
                               QString::number(adjustment.classConstraintWeight, 'f', 2));
    }
    if (adjustment.postMealSleepPlan != QStringLiteral("unknown")) {
        appendNormalizedWeight(QStringLiteral("饭后安排"),
                               adjustment.postMealSleepPlan);
    }
    if (adjustment.plannedNapMinutes > 0) {
        appendNormalizedWeight(QStringLiteral("计划小睡"),
                               QStringLiteral("%1 分钟").arg(adjustment.plannedNapMinutes));
    }
    if (!isNeutralValue(adjustment.sleepNeedLevel, 1.0)) {
        appendNormalizedWeight(QStringLiteral("困意强度"),
                               QString::number(adjustment.sleepNeedLevel, 'f', 2));
    }
    if (adjustment.sleepPlanConfidence > 0.0) {
        appendNormalizedWeight(QStringLiteral("睡眠计划置信度"),
                               QString::number(adjustment.sleepPlanConfidence, 'f', 2));
    }
    if (!isNeutralValue(adjustment.proteinIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("蛋白倾向"),
                               QString::number(adjustment.proteinIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.colaIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("可乐倾向"),
                               QString::number(adjustment.colaIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.flavorIntent, 1.0)) {
        appendNormalizedWeight(QStringLiteral("口味偏好"),
                               QString::number(adjustment.flavorIntent, 'f', 2));
    }
    if (!isNeutralValue(adjustment.relaxedTimePreference, 1.0)) {
        appendNormalizedWeight(QStringLiteral("时间宽松"),
                               QString::number(adjustment.relaxedTimePreference, 'f', 2));
    }

    if (weights.isEmpty()) {
        appendNormalizedWeight(QStringLiteral("补充输入"),
                               adjustment.fallbackUsed ? QStringLiteral("默认回退")
                                                       : QStringLiteral("中性"));
    }

    return weights;

    const auto appendWeight = [&weights](const QString &label, const QString &value) {
        QVariantMap item;
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("value"), value);
        weights.append(item);
    };

    if (adjustment.hungerIntent > 0.0) {
        appendWeight(QStringLiteral("饥饿倾向"),
                     QString::number(adjustment.hungerIntent, 'f', 2));
    }
    if (adjustment.carbIntent != 0.0) {
        appendWeight(QStringLiteral("碳水倾向"),
                     QString::number(adjustment.carbIntent, 'f', 2));
    }
    if (adjustment.drinkIntent > 0.0) {
        appendWeight(QStringLiteral("饮品倾向"),
                     QString::number(adjustment.drinkIntent, 'f', 2));
    }
    if (adjustment.budgetFlexIntent != 0.0) {
        appendWeight(QStringLiteral("预算放宽"),
                     QString::number(adjustment.budgetFlexIntent, 'f', 2));
    }
    if (adjustment.skipClassConstraint) {
        appendWeight(QStringLiteral("跳过上课约束"), QStringLiteral("是"));
    }
    if (adjustment.postMealSleepPlan != QStringLiteral("unknown")) {
        appendWeight(QStringLiteral("饭后安排"), adjustment.postMealSleepPlan);
    }
    if (adjustment.plannedNapMinutes > 0) {
        appendWeight(QStringLiteral("计划小睡"),
                     QStringLiteral("%1 分钟").arg(adjustment.plannedNapMinutes));
    }
    if (adjustment.sleepNeedLevel > 0.0) {
        appendWeight(QStringLiteral("困意需求"),
                     QString::number(adjustment.sleepNeedLevel, 'f', 2));
    }
    if (adjustment.sleepPlanConfidence > 0.0) {
        appendWeight(QStringLiteral("睡眠计划置信度"),
                     QString::number(adjustment.sleepPlanConfidence, 'f', 2));
    }
    if (adjustment.proteinIntent > 0.0) {
        appendWeight(QStringLiteral("蛋白倾向"),
                     QString::number(adjustment.proteinIntent, 'f', 2));
    }
    if (adjustment.colaIntent > 0.0) {
        appendWeight(QStringLiteral("可乐倾向"),
                     QString::number(adjustment.colaIntent, 'f', 2));
    }
    if (adjustment.flavorIntent > 0.0) {
        appendWeight(QStringLiteral("口味偏好"),
                     QString::number(adjustment.flavorIntent, 'f', 2));
    }
    if (adjustment.relaxedTimePreference) {
        appendWeight(QStringLiteral("放宽耗时"), QStringLiteral("是"));
    }

    if (weights.isEmpty()) {
        appendWeight(QStringLiteral("补充输入"), QStringLiteral("中性"));
    }

    return weights;
}
}

RecommendationEngine::RecommendationEngine(const DatabaseManager &databaseManager,
                                           AppSettings *appSettings,
                                           QObject *parent)
    : QObject(parent),
      m_databaseManager(databaseManager),
      m_appSettings(appSettings)
{
    if (m_appSettings != nullptr) {
        connect(m_appSettings, &AppSettings::llmSettingsChanged,
                this, &RecommendationEngine::refreshSupplementConfigState);
    }
    setInitialState();
}

QString RecommendationEngine::summary() const
{
    return m_summary;
}

QVariantList RecommendationEngine::candidates() const
{
    return m_candidates;
}

bool RecommendationEngine::busy() const
{
    return m_busy;
}

bool RecommendationEngine::apiConfigured() const
{
    return AppConfig::llmApiConfigured();
}

QString RecommendationEngine::supplementSummary() const
{
    return m_supplementSummary;
}

QString RecommendationEngine::supplementStatus() const
{
    return m_supplementStatus;
}

QString RecommendationEngine::supplementState() const
{
    return m_supplementState;
}

bool RecommendationEngine::supplementFallbackActive() const
{
    return m_supplementFallbackActive;
}

QVariantList RecommendationEngine::supplementWeights() const
{
    return m_supplementWeights;
}

QVariantList RecommendationEngine::activeWeightConfig() const
{
    return m_activeWeightConfig;
}

void RecommendationEngine::reload()
{
    runDecision();
}

void RecommendationEngine::runDecision()
{
    PlanningRepository planningRepository(m_databaseManager.connectionName());
    ScheduleRepository scheduleRepository(m_databaseManager.connectionName());
    MerchantRepository merchantRepository(m_databaseManager.connectionName());
    DishRepository dishRepository(m_databaseManager.connectionName());
    MealFeedbackRepository mealFeedbackRepository(m_databaseManager.connectionName());
    MealLogRepository mealLogRepository(m_databaseManager.connectionName());
    RecommendationRecordRepository recommendationRecordRepository(
        m_databaseManager.connectionName());

    const QList<PlanningPolicy> policies =
        planningRepository.loadPlanningPolicies();
    const QList<RecommendationProfile> profiles =
        planningRepository.loadRecommendationProfiles();
    const QList<Merchant> merchants = merchantRepository.loadAllMerchants();
    const QList<Dish> dishes = dishRepository.loadActiveDishes();
    const QList<Dish> allDishes = dishRepository.loadAllDishes();
    const QList<DishFeedbackAggregate> dishFeedbackAggregates =
        mealFeedbackRepository.loadDishFeedbackAggregates();
    const QList<MealLog> recentMealLogs =
        mealLogRepository.loadRecentMealLogs(20);

    m_candidates.clear();

    if (dishes.isEmpty()) {
        m_summary = QStringLiteral("当前没有可用菜品，请先在 Food 页面录入启用菜品。");
        emit recommendationsChanged();
        return;
    }

    MealContext context =
        buildMealContext(policies, profiles, scheduleRepository);

    const double classConstraintDelta =
        strongIntentDelta(m_adjustment.classConstraintWeight);
    const double relaxedTimeDelta =
        strongIntentDelta(m_adjustment.relaxedTimePreference);
    const double hungerIntentDelta = weakIntentDelta(m_adjustment.hungerIntent);
    const double carbIntentDelta = weakIntentDelta(m_adjustment.carbIntent);
    const double budgetFlexDelta = weakIntentDelta(m_adjustment.budgetFlexIntent);
    const double drinkIntentDelta = weakIntentDelta(m_adjustment.drinkIntent);
    const double proteinIntentDelta = weakIntentDelta(m_adjustment.proteinIntent);
    const double colaIntentDelta = weakIntentDelta(m_adjustment.colaIntent);
    const double beverageIntentDelta =
        drinkIntentDelta > 0.0
            ? drinkIntentDelta
            : (colaIntentDelta > 0.0 ? colaIntentDelta * 0.75
                                     : drinkIntentDelta);
    const double flavorIntentDelta = weakIntentDelta(m_adjustment.flavorIntent);
    const bool classConstraintFullyRelaxed =
        m_adjustment.hasParsed &&
        m_adjustment.classConstraintWeight <= 0.65;

    bool classPressureWasRelaxed = false;
    if (m_adjustment.hasParsed &&
        classConstraintFullyRelaxed &&
        context.hasClassAfterMeal) {
        context.hasClassAfterMeal = false;
        context.minutesUntilNextClass = 0;
        context.profile = selectProfile(profiles, false);
        classPressureWasRelaxed = true;
    }

    const bool classPressure =
        context.hasClassAfterMeal &&
        context.minutesUntilNextClass > 0 &&
        context.minutesUntilNextClass <= 120;

    const SleepPlanModifier sleepModifier = resolveSleepPlanModifier(m_adjustment);
    const WeightMap weights =
        buildResolvedWeights(context, classPressure, m_weightOverrides);
    m_activeWeightConfig =
        buildWeightConfigList(context, classPressure, weights, sleepModifier);
    emit weightConfigChanged();

    QHash<int, Merchant> merchantById;
    for (const Merchant &merchant : merchants) {
        merchantById.insert(merchant.id, merchant);
    }

    QHash<int, Dish> allDishById;
    for (const Dish &dish : allDishes) {
        allDishById.insert(dish.id, dish);
    }

    QHash<int, DishFeedbackAggregate> feedbackByDishId;
    for (const DishFeedbackAggregate &aggregate : dishFeedbackAggregates) {
        feedbackByDishId.insert(aggregate.dishId, aggregate);
    }

    const int recentMealWindow = std::max(
        1, static_cast<int>(std::round(
               weights.value(QStringLiteral("config.recent_non_breakfast_meal_window"), 6.0))));
    const bool excludeBreakfast =
        weights.value(QStringLiteral("config.exclude_breakfast"), 1.0) >= 0.5;

    const QList<MealSnapshot> recentMealSnapshots =
        buildRecentMealSnapshots(recentMealLogs, mealLogRepository,
                                 recentMealWindow, excludeBreakfast);

    QList<MealAggregate> recentMealAggregates;
    recentMealAggregates.reserve(recentMealSnapshots.size());
    for (const MealSnapshot &snapshot : recentMealSnapshots) {
        recentMealAggregates.append(aggregateMeal(snapshot, allDishById));
    }

    const double baseDailyBudget =
        context.policy.defaultDailyBudget > 0.0 ? context.policy.defaultDailyBudget : 80.0;
    const double baseFlexibleBudget =
        context.policy.flexibleBudgetCap > 0.0 ? context.policy.flexibleBudgetCap : 120.0;
    const double budgetScale = 1.0 + clampRange(budgetFlexDelta, -0.25, 0.35) * 0.25;
    const double budgetRelaxBlend =
        m_adjustment.hasParsed && budgetFlexDelta > 0.0
            ? clamp01(budgetFlexDelta / 0.35)
            : 0.0;
    const bool relaxedBudgetDinnerScene =
        budgetRelaxBlend > 0.0 &&
        !classPressure &&
        context.mealType == QStringLiteral("dinner");
    const double mealBudget =
        (context.mealType == QStringLiteral("breakfast") ? baseDailyBudget * 0.20
                                                         : baseDailyBudget * 0.40) *
        budgetScale;
    double flexibleMealBudget =
        (context.mealType == QStringLiteral("breakfast") ? baseFlexibleBudget * 0.24
                                                         : baseFlexibleBudget * 0.45) *
        std::max(0.85, budgetScale);
    if (relaxedBudgetDinnerScene) {
        const double relaxedDinnerCapShare =
            std::min(1.0, 0.45 + budgetRelaxBlend * 0.70);
        flexibleMealBudget =
            std::max(flexibleMealBudget, baseFlexibleBudget * relaxedDinnerCapShare);
    }

    QList<CandidateResult> rankedCandidates;
    rankedCandidates.reserve(dishes.size());

    for (const Dish &dish : dishes) {
        if (!merchantById.contains(dish.merchantId)) {
            continue;
        }

        const Merchant merchant = merchantById.value(dish.merchantId);
        if (!supportsDiningMode(dish, merchant)) {
            continue;
        }

        CandidateSignals metrics;
        metrics.acquisitionMinutes = acquisitionMinutes(dish, merchant);

        const HistoryInsights history = analyzeHistory(dish, recentMealAggregates);
        const DishFeedbackAggregate feedback =
            feedbackByDishId.value(dish.id, DishFeedbackAggregate());
        const bool snackLike = isSnackLikeDish(dish);
        const bool breakfastLike = isBreakfastLikeDish(dish);
        const bool colaDish = isColaDish(dish);

        const double baseCarbLevel = positiveLevelScore(dish.carbLevel);
        const double baseFatLevel = positiveLevelScore(dish.fatLevel);
        const double baseProteinLevel = positiveLevelScore(dish.proteinLevel);
        const double baseFiberLevel = positiveLevelScore(dish.fiberLevel);
        const double baseVitaminLevel =
            0.65 * positiveLevelScore(dish.vitaminLevel) +
            0.35 * positiveLevelScore(dish.vegetableLevel);
        const double baseSatietyLevel = positiveLevelScore(dish.satietyLevel);
        double baseDigestiveBurdenSafe = lowRiskScore(dish.digestiveBurdenLevel);
        double baseSleepinessSafe = lowRiskScore(dish.sleepinessRiskLevel);
        const double mealImpactFit = clamp01(dish.mealImpactWeight);
        const double effectiveFeedbackWeight =
            std::min(feedback.effectiveSampleWeight, 6.0);

        if (feedback.effectiveSampleWeight > 0.0) {
            const double comfortSignal = clamp01(feedback.avgComfortLevel / 5.0);
            const double sleepinessSignal = clamp01(feedback.avgSleepinessLevel / 5.0);
            const double focusSignal = clamp01(feedback.avgFocusImpactLevel / 5.0);
            baseDigestiveBurdenSafe =
                clamp01(baseDigestiveBurdenSafe * 0.78 + comfortSignal * 0.22);
            baseSleepinessSafe =
                clamp01(baseSleepinessSafe * 0.72 +
                        (1.0 - sleepinessSignal) * 0.18 +
                        focusSignal * 0.10);
        }

        const double carbTargetBase =
            classPressure ? 0.28 : (context.mealType == QStringLiteral("dinner") ? 0.45 : 0.38);
        const double carbTarget =
            clampRange(carbTargetBase + carbIntentDelta * 0.80 -
                           classConstraintDelta * 0.08 -
                           (sleepModifier.carbPenaltyMultiplier - 1.0) * 0.15,
                       0.10, 0.85);
        const double satietyTarget =
            clampRange(0.45 + hungerIntentDelta, 0.30, 0.90);
        const double fatTarget = classPressure ? 0.25 : 0.40;

        metrics.carbFit = closenessScore(baseCarbLevel, carbTarget) * 100.0;
        metrics.fatFit = closenessScore(baseFatLevel, fatTarget) * 100.0;
        metrics.proteinSupport = baseProteinLevel * 100.0;
        metrics.fiberSupport = baseFiberLevel * 100.0;
        metrics.vitaminSupport = baseVitaminLevel * 100.0;
        metrics.satietySupport = closenessScore(baseSatietyLevel, satietyTarget) * 100.0;
        metrics.digestiveBurdenFit = baseDigestiveBurdenSafe * 100.0;
        metrics.sleepinessRiskFit =
            clamp01(baseSleepinessSafe -
                    (sleepModifier.sleepinessPenaltyMultiplier - 1.0) * 0.18 *
                        positiveLevelScore(dish.sleepinessRiskLevel)) * 100.0;
        metrics.giFit =
            clamp01(closenessScore(baseCarbLevel, carbTarget) * 0.75 +
                    baseFiberLevel * 0.25) *
            100.0;

        double availableMinutes = 90.0;
        if (context.hasClassAfterMeal && context.minutesUntilNextClass > 0) {
            availableMinutes = context.minutesUntilNextClass;
        } else if (context.mealType == QStringLiteral("dinner")) {
            availableMinutes = 110.0;
        }
        availableMinutes =
            std::max(35.0, availableMinutes + relaxedTimeDelta * 30.0 - std::max(0.0, classConstraintDelta) * 12.0);

        const bool napPlanned =
            m_adjustment.postMealSleepPlan == QStringLiteral("nap_before_class");
        const double napMinutes = napPlanned ? m_adjustment.plannedNapMinutes : 0.0;
        metrics.totalOccupiedMinutes =
            metrics.acquisitionMinutes +
            (napMinutes + sleepModifier.wakeBufferMinutes) *
                sleepModifier.totalTimePenaltyMultiplier;
        metrics.timeFit =
            clamp01(1.0 - std::max(0.0, metrics.totalOccupiedMinutes - availableMinutes) /
                             std::max(40.0, availableMinutes)) *
            100.0;

        const double diningModeBase =
            dish.defaultDiningMode == QStringLiteral("takeaway")
                ? 0.92
                : dish.defaultDiningMode == QStringLiteral("delivery") ? 0.85 : 0.72;
        const double sceneDiningBoost =
            classPressure || napPlanned
                ? (dish.defaultDiningMode == QStringLiteral("dine_in") ? 0.88 : 1.0)
                : (dish.defaultDiningMode == QStringLiteral("dine_in") ? 1.0 : 0.92);
        metrics.diningModeFit = clamp01(diningModeBase * sceneDiningBoost) * 100.0;

        double mealTypeFit = snackLike ? 0.28 : 0.85;
        if (dish.mealImpactWeight < 0.75) {
            mealTypeFit = std::min(mealTypeFit, 0.62);
        }
        if (context.mealType != QStringLiteral("breakfast") && breakfastLike) {
            mealTypeFit = std::min(mealTypeFit, 0.45);
        }
        if (context.mealType == QStringLiteral("dinner") && !snackLike) {
            mealTypeFit = std::max(mealTypeFit, 0.88);
        }
        metrics.mealTypeFit = mealTypeFit * 100.0;

        const double classComposite =
            clamp01(baseSleepinessSafe * 0.50 +
                    baseDigestiveBurdenSafe * 0.30 +
                    lowRiskScore(dish.carbLevel) * 0.20);
        metrics.classFit =
            (context.hasClassAfterMeal ? classComposite : clamp01(classComposite * 0.6 + 0.4)) *
            100.0;

        metrics.sceneFit = weightedScore(
            {{metrics.timeFit / 100.0, weights.value(QStringLiteral("scene.time_fit"))},
             {metrics.classFit / 100.0, weights.value(QStringLiteral("scene.class_fit"))},
             {metrics.diningModeFit / 100.0, weights.value(QStringLiteral("scene.dining_mode_fit"))},
             {metrics.mealTypeFit / 100.0, weights.value(QStringLiteral("scene.meal_type_fit"))}});

        const double sleepinessMetricWeight =
            weights.value(QStringLiteral("nutrition.sleepiness_risk_fit")) *
            sleepModifier.sleepinessPenaltyMultiplier;
        const double carbMetricWeight =
            weights.value(QStringLiteral("nutrition.carb_fit")) *
            sleepModifier.carbPenaltyMultiplier;

        metrics.nutritionFit = weightedScore(
            {{metrics.carbFit / 100.0, carbMetricWeight},
             {metrics.fatFit / 100.0, weights.value(QStringLiteral("nutrition.fat_fit"))},
             {metrics.proteinSupport / 100.0, weights.value(QStringLiteral("nutrition.protein_support"))},
             {metrics.fiberSupport / 100.0, weights.value(QStringLiteral("nutrition.fiber_support"))},
             {metrics.vitaminSupport / 100.0, weights.value(QStringLiteral("nutrition.vitamin_support"))},
             {metrics.satietySupport / 100.0, weights.value(QStringLiteral("nutrition.satiety_support"))},
             {metrics.digestiveBurdenFit / 100.0, weights.value(QStringLiteral("nutrition.digestive_burden_fit"))},
             {metrics.sleepinessRiskFit / 100.0, sleepinessMetricWeight},
             {metrics.giFit / 100.0, weights.value(QStringLiteral("nutrition.gi_fit"))}});

        const double baselineTasteSeed = normalizedPreferenceSeed(dish.favoriteScore);
        const double tasteSeed = smoothedAverageWeighted(
            baselineTasteSeed,
            feedback.avgTasteRating > 0.0 ? feedback.avgTasteRating / 5.0 : 0.0,
            effectiveFeedbackWeight);
        const double repeatSeed = smoothedAverageWeighted(
            clamp01(baselineTasteSeed * 0.7 + 0.25),
            feedback.avgRepeatWillingness > 0.0
                ? 0.75 * (feedback.avgRepeatWillingness / 5.0) +
                      0.25 * clamp01(feedback.wouldEatAgainRate)
                : 0.0,
            effectiveFeedbackWeight);
        const double comfortSeed =
            smoothedAverageWeighted(0.55,
                                    feedback.avgComfortLevel > 0.0
                                        ? clamp01(feedback.avgComfortLevel / 5.0)
                                        : 0.0,
                                    effectiveFeedbackWeight, 2.4);
        const double focusSeed =
            smoothedAverageWeighted(0.55,
                                    feedback.avgFocusImpactLevel > 0.0
                                        ? clamp01(feedback.avgFocusImpactLevel / 5.0)
                                        : 0.0,
                                    effectiveFeedbackWeight, 2.4);
        metrics.tasteRatingFit = tasteSeed * 100.0;
        metrics.repeatWillingnessFit = repeatSeed * 100.0;
        metrics.preferenceScoreFit =
            clamp01((tasteSeed * 0.45 + repeatSeed * 0.30 + comfortSeed * 0.15 +
                     focusSeed * 0.10)) *
            100.0;
        metrics.preferenceFit = weightedScore(
            {{metrics.tasteRatingFit / 100.0, weights.value(QStringLiteral("preference.taste_rating"))},
             {metrics.repeatWillingnessFit / 100.0, weights.value(QStringLiteral("preference.repeat_willingness"))},
             {metrics.preferenceScoreFit / 100.0, weights.value(QStringLiteral("preference.preference_score"))}});

        double sameDishFit = 1.0;
        if (history.sameDishCount == 1) {
            sameDishFit = 0.72;
        } else if (history.sameDishCount == 2) {
            sameDishFit = 0.42;
        } else if (history.sameDishCount >= 3) {
            sameDishFit = 0.08;
        }
        sameDishFit -= history.consecutiveSameDishCount * 0.10;
        metrics.sameDishFit = clamp01(sameDishFit) * 100.0;

        double sameMerchantFit = 1.0;
        if (history.sameMerchantCount >= 2) {
            sameMerchantFit -= 0.18;
        }
        sameMerchantFit -= history.consecutiveSameMerchantCount * 0.12;
        metrics.sameMerchantFit = clamp01(sameMerchantFit) * 100.0;

        double nutritionCompensation = 0.55;
        const int compensationWindow = std::max(
            1, static_cast<int>(std::round(
                   weights.value(QStringLiteral("config.nutrition_compensation_window"), 4.0))));
        const int effectiveHighCarbMeals =
            std::min(history.highCarbMealCount, compensationWindow);
        const int effectiveHeavyMeals =
            std::min(history.heavyMealCount, compensationWindow);
        const int effectiveLightMeals =
            std::min(history.lightButUnsatisfiedMealCount, compensationWindow);

        if (effectiveHighCarbMeals >= 2) {
            nutritionCompensation += lowRiskScore(dish.carbLevel) * 0.18;
            nutritionCompensation += baseProteinLevel * 0.08;
            nutritionCompensation += baseFiberLevel * 0.10;
            nutritionCompensation -= positiveLevelScore(dish.carbLevel) * 0.18;
        }
        if (effectiveHeavyMeals >= 2) {
            nutritionCompensation += baseDigestiveBurdenSafe * 0.14;
            nutritionCompensation -= positiveLevelScore(dish.digestiveBurdenLevel) * 0.18;
            nutritionCompensation -= positiveLevelScore(dish.fatLevel) * 0.10;
        }
        if (effectiveLightMeals >= 2) {
            nutritionCompensation += baseSatietyLevel * 0.15;
            nutritionCompensation += baseProteinLevel * 0.10;
        }
        metrics.nutritionCompensationFit = clamp01(nutritionCompensation) * 100.0;

        metrics.diversityFit = weightedScore(
            {{metrics.sameDishFit / 100.0, weights.value(QStringLiteral("diversity.same_dish_penalty"))},
             {metrics.sameMerchantFit / 100.0, weights.value(QStringLiteral("diversity.same_merchant_penalty"))},
             {metrics.nutritionCompensationFit / 100.0, weights.value(QStringLiteral("diversity.nutrition_compensation"))}});

        double priceFit = 0.0;
        double budgetPressureFit = 0.0;
        if (dish.price <= mealBudget) {
            priceFit = 1.0;
            budgetPressureFit = 1.0;
        } else if (dish.price <= flexibleMealBudget) {
            const double distanceToCap =
                (dish.price - mealBudget) / std::max(1.0, flexibleMealBudget - mealBudget);
            priceFit = 0.78 - distanceToCap * 0.28;
            budgetPressureFit = 0.72 - distanceToCap * 0.32;
        } else {
            const double overflow =
                (dish.price - flexibleMealBudget) / std::max(10.0, flexibleMealBudget);
            priceFit = 0.35 - overflow * 0.40;
            budgetPressureFit = 0.22 - overflow * 0.35;
        }
        metrics.priceFit = clamp01(priceFit) * 100.0;
        metrics.budgetPressureFit = clamp01(budgetPressureFit) * 100.0;

        const double acquireTarget =
            classPressure ? 38.0 : (context.mealType == QStringLiteral("dinner") ? 58.0 : 48.0);
        metrics.acquireCostFit =
            clamp01(1.0 - std::max(0.0, metrics.acquisitionMinutes - acquireTarget) /
                             std::max(20.0, acquireTarget)) *
            100.0;
        metrics.budgetFit = weightedScore(
            {{metrics.priceFit / 100.0, weights.value(QStringLiteral("budget.price_fit"))},
             {metrics.budgetPressureFit / 100.0, weights.value(QStringLiteral("budget.budget_pressure"))},
             {metrics.acquireCostFit / 100.0, weights.value(QStringLiteral("budget.acquire_cost_fit"))}});

        metrics.hungerIntentFit =
            closenessScore(baseSatietyLevel,
                           clampRange(0.45 + hungerIntentDelta * 0.90, 0.25, 0.92)) *
            100.0;
        const double carbIntentTarget =
            clampRange(carbTargetBase + carbIntentDelta * 0.90, 0.10, 0.90);
        metrics.carbIntentFit = closenessScore(baseCarbLevel, carbIntentTarget) * 100.0;

        double drinkIntentFit = 0.68;
        if (beverageIntentDelta > 0.0) {
            drinkIntentFit = snackLike ? 0.85 : 0.48;
            if (colaIntentDelta > 0.0 && colaDish) {
                drinkIntentFit = 0.95;
            }
        } else {
            drinkIntentFit = beverageIntentDelta < 0.0
                                 ? (snackLike ? 0.18 : 0.84)
                                 : (snackLike ? 0.24 : 0.78);
        }
        metrics.drinkIntentFit = drinkIntentFit * 100.0;

        const double budgetIntentTarget =
            mealBudget + (flexibleMealBudget - mealBudget) *
                             clamp01((m_adjustment.budgetFlexIntent - 0.75) / 0.60);
        metrics.budgetFlexIntentFit =
            clamp01(1.0 - std::abs(dish.price - budgetIntentTarget) /
                             std::max(20.0, budgetIntentTarget)) *
            100.0;

        double skipConstraintFit = 0.55;
        if (context.hasClassAfterMeal &&
            m_adjustment.classConstraintWeight < 1.0) {
            const double relaxedBlend =
                clamp01((1.0 - m_adjustment.classConstraintWeight) / 0.60);
            skipConstraintFit =
                clamp01((0.55 * (1.0 - relaxedBlend)) +
                        (0.60 + positiveLevelScore(dish.flavorLevel) * 0.20 +
                         positiveLevelScore(dish.satietyLevel) * 0.20) * relaxedBlend) *
                100.0;
        } else if (!context.hasClassAfterMeal) {
            skipConstraintFit = 60.0;
        } else if (m_adjustment.classConstraintWeight > 1.0) {
            skipConstraintFit =
                clamp01(0.55 - std::max(0.0, classConstraintDelta) * 0.18) * 100.0;
        }
        metrics.skipClassConstraintFit = skipConstraintFit;

        metrics.intentFit = weightedScore(
            {{metrics.hungerIntentFit / 100.0, weights.value(QStringLiteral("intent.hunger_intent"))},
             {metrics.carbIntentFit / 100.0, weights.value(QStringLiteral("intent.carb_intent"))},
             {metrics.drinkIntentFit / 100.0, weights.value(QStringLiteral("intent.drink_intent"))},
             {metrics.budgetFlexIntentFit / 100.0, weights.value(QStringLiteral("intent.budget_flex_intent"))},
             {metrics.skipClassConstraintFit / 100.0, weights.value(QStringLiteral("intent.skip_class_constraint"))}});

        metrics.totalScore =
            metrics.sceneFit * weights.value(QStringLiteral("group.scene_fit")) / 100.0 +
            metrics.nutritionFit * weights.value(QStringLiteral("group.nutrition_fit")) / 100.0 +
            metrics.preferenceFit * weights.value(QStringLiteral("group.preference_fit")) / 100.0 +
            metrics.diversityFit * weights.value(QStringLiteral("group.diversity_fit")) / 100.0 +
            metrics.budgetFit * weights.value(QStringLiteral("group.budget_fit")) / 100.0 +
            metrics.intentFit * weights.value(QStringLiteral("group.intent_fit")) / 100.0;

        metrics.totalScore += mealImpactFit * 4.5;
        metrics.totalScore += positiveLevelScore(dish.flavorLevel) *
                              (2.0 + std::max(0.0, flavorIntentDelta) * 8.0);
        metrics.totalScore += positiveLevelScore(dish.proteinLevel) *
                              (1.5 + std::max(0.0, proteinIntentDelta) * 7.0);
        if (relaxedBudgetDinnerScene &&
            dish.price > mealBudget &&
            dish.price <= flexibleMealBudget) {
            const double relaxedSpendMatch =
                clamp01((dish.price - mealBudget) /
                        std::max(1.0, flexibleMealBudget - mealBudget));
            metrics.totalScore += budgetRelaxBlend * relaxedSpendMatch * 24.0;
        }
        if (colaDish && colaIntentDelta > 0.0) {
            metrics.totalScore += colaIntentDelta * 18.0;
        }
        if (snackLike && beverageIntentDelta > 0.0) {
            metrics.totalScore += beverageIntentDelta * 8.0;
        }
        if (snackLike && beverageIntentDelta < 0.0) {
            metrics.totalScore -= 5.0;
        }

        CandidateResult candidate;
        candidate.dish = dish;
        candidate.merchant = merchant;
        candidate.metrics = metrics;
        candidate.reasons = buildReasons(dish, context, m_adjustment, metrics, feedback, history,
                                         sleepModifier, classPressureWasRelaxed,
                                         mealBudget, flexibleMealBudget);
        candidate.warnings = buildWarnings(dish, context, m_adjustment, metrics, feedback,
                                           sleepModifier, flexibleMealBudget);
        candidate.breakdown = buildBreakdownList(metrics);
        rankedCandidates.append(candidate);
    }

    std::sort(rankedCandidates.begin(), rankedCandidates.end(),
              [](const CandidateResult &left, const CandidateResult &right) {
                  if (std::abs(left.metrics.totalScore - right.metrics.totalScore) > 0.01) {
                      return left.metrics.totalScore > right.metrics.totalScore;
                  }
                  if (std::abs(left.dish.price - right.dish.price) > 0.01) {
                      return left.dish.price < right.dish.price;
                  }

                  const QString leftDishName = left.dish.name.trimmed().toLower();
                  const QString rightDishName = right.dish.name.trimmed().toLower();
                  if (leftDishName != rightDishName) {
                      return leftDishName < rightDishName;
                  }

                  const QString leftMerchantName =
                      left.merchant.name.trimmed().toLower();
                  const QString rightMerchantName =
                      right.merchant.name.trimmed().toLower();
                  if (leftMerchantName != rightMerchantName) {
                      return leftMerchantName < rightMerchantName;
                  }

                  return left.dish.id < right.dish.id;
              });

    const int candidateCount =
        std::min(3, static_cast<int>(rankedCandidates.size()));
    for (int i = 0; i < candidateCount; ++i) {
        const CandidateResult &candidate = rankedCandidates.at(i);
        QVariantMap candidateMap;
        candidateMap.insert(QStringLiteral("rank"), i + 1);
        candidateMap.insert(QStringLiteral("dishName"), candidate.dish.name);
        candidateMap.insert(QStringLiteral("merchantName"),
                            candidate.merchant.name);
        candidateMap.insert(QStringLiteral("price"), candidate.dish.price);
        candidateMap.insert(QStringLiteral("score"),
                            QString::number(candidate.metrics.totalScore, 'f', 1));
        candidateMap.insert(QStringLiteral("reason"),
                            candidate.reasons.join(QStringLiteral(" ")));
        candidateMap.insert(QStringLiteral("reasons"), candidate.reasons);
        candidateMap.insert(QStringLiteral("warnings"), candidate.warnings);
        candidateMap.insert(QStringLiteral("warningText"),
                            candidate.warnings.join(QStringLiteral("；")));
        candidateMap.insert(QStringLiteral("breakdown"), candidate.breakdown);
        m_candidates.append(candidateMap);
    }

    QString contextLine = QStringLiteral("决策目标：%1 %2（%3）。")
                              .arg(context.targetDate.toString(QStringLiteral("yyyy-MM-dd")))
                              .arg(weekdayLabel(context.weekday))
                              .arg(context.mealLabel);

    if (classPressure) {
        contextLine += QStringLiteral(" 当前餐后约 %1 分钟内有课，场景适配和营养稳态权重被抬高。")
                           .arg(context.minutesUntilNextClass);
    } else {
        contextLine += QStringLiteral(" 当前没有把 120 分钟内上课当作硬约束，整体更偏向均衡、预算和多样性。");
    }

    if (classPressureWasRelaxed) {
        contextLine += QStringLiteral(" 已根据补充说明放宽上课约束。");
    }

    if (m_adjustment.hasParsed && !m_adjustment.summary.isEmpty()) {
        contextLine += QStringLiteral(" 补充输入：%1").arg(m_adjustment.summary);
    }

    if (recentMealAggregates.isEmpty()) {
        contextLine += QStringLiteral(" 最近餐次不足，多餐补偿当前保持中性。");
    } else {
        contextLine += QStringLiteral(" 多餐补偿默认按最近 %1 餐、且排除早餐来计算。")
                           .arg(recentMealAggregates.size());
    }

    if (sleepModifier.label != QStringLiteral("未知")) {
        contextLine += QStringLiteral(" 饭后安排按“%1”修正了碳水、困倦和总耗时惩罚。")
                           .arg(sleepModifier.label);
    }

    RecommendationRecord record;
    record.recommendedForMealType = context.mealType;
    record.generatedAt = effectiveCurrentDateTime();
    record.contextSummary = contextLine;
    record.strategyProfileId = context.profile.id;
    if (candidateCount >= 1) {
        const CandidateResult &candidate = rankedCandidates.at(0);
        record.candidate1.dishId = candidate.dish.id;
        record.candidate1.score = candidate.metrics.totalScore;
        record.candidate1.reason = candidate.reasons.join(QStringLiteral(" "));
    }
    if (candidateCount >= 2) {
        const CandidateResult &candidate = rankedCandidates.at(1);
        record.candidate2.dishId = candidate.dish.id;
        record.candidate2.score = candidate.metrics.totalScore;
        record.candidate2.reason = candidate.reasons.join(QStringLiteral(" "));
    }
    if (candidateCount >= 3) {
        const CandidateResult &candidate = rankedCandidates.at(2);
        record.candidate3.dishId = candidate.dish.id;
        record.candidate3.score = candidate.metrics.totalScore;
        record.candidate3.reason = candidate.reasons.join(QStringLiteral(" "));
    }
    recommendationRecordRepository.addRecord(record);

    m_summary = contextLine;
    emit recommendationsChanged();
}

QString RecommendationEngine::previewRecommendation() const
{
    if (m_candidates.isEmpty()) {
        return m_summary;
    }

    const QVariantMap topCandidate = m_candidates.first().toMap();
    return QStringLiteral("#%1 %2：%3")
        .arg(topCandidate.value(QStringLiteral("rank")).toInt())
        .arg(topCandidate.value(QStringLiteral("dishName")).toString())
        .arg(topCandidate.value(QStringLiteral("reason")).toString());
}

#if 0
void RecommendationEngine::parseSupplement(const QString &text)
{
    const QString trimmedText = text.trimmed();
    if (trimmedText.isEmpty()) {
        m_supplementStatus = QStringLiteral("请先输入补充说明，再调用解析。");
        emit supplementChanged();
        return;
    }

    if (!apiConfigured()) {
        m_supplementStatus = QStringLiteral(
            "API 尚未配置。请设置 MEALADVISOR_LLM_API_KEY，并按需补充 MEALADVISOR_LLM_API_URL / MEALADVISOR_LLM_MODEL。");
        emit supplementChanged();
        return;
    }

    setBusy(true);
    m_supplementStatus = QStringLiteral("正在解析补充说明...");
    emit supplementChanged();

    QNetworkRequest request{QUrl(AppConfig::llmApiUrl())};
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1")
                             .arg(AppConfig::llmApiKey())
                             .toUtf8());

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"),
         QStringLiteral(
             "You convert natural-language meal context into structured inputs for a local rule engine. "
             "Return JSON only with these keys: "
             "summary (string), "
             "hunger_intent (number 0..1), "
             "carb_intent (number 0..1), "
             "drink_intent (number 0..1), "
             "budget_flex_intent (number 0..1), "
             "skip_class_constraint (boolean), "
             "post_meal_sleep_plan (string enum: stay_awake, nap_before_class, no_class, unknown), "
             "planned_nap_minutes (integer 0..180), "
             "sleep_need_level (number 0..1), "
             "sleep_plan_confidence (number 0..1), "
             "protein_intent (number 0..1), "
             "cola_intent (number 0..1), "
             "flavor_intent (number 0..1), "
             "relaxed_time_preference (boolean). "
             "Use 0 or false or unknown when not mentioned. Infer lightly and stay conservative.")}});
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), trimmedText}});

    QJsonObject payload{
        {QStringLiteral("model"), AppConfig::llmModel()},
        {QStringLiteral("messages"), messages},
        {QStringLiteral("temperature"), 0.1}
    };

    QNetworkReply *reply = m_networkAccessManager.post(
        request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedText]() {
        const QByteArray responseBody = reply->readAll();
        const QString networkError = reply->error() == QNetworkReply::NoError
                                         ? QString()
                                         : reply->errorString();
        reply->deleteLater();
        setBusy(false);

        if (!networkError.isEmpty()) {
            m_supplementStatus =
                QStringLiteral("补充说明解析失败：%1").arg(networkError);
            emit supplementChanged();
            return;
        }

        const QString rawText = extractJsonTextFromApiResponse(responseBody);
        const QJsonObject parsedObject = extractStructuredObject(rawText);
        if (parsedObject.isEmpty()) {
            m_supplementStatus = QStringLiteral(
                "API 返回内容无法识别，请缩短输入或检查接口兼容性。");
            emit supplementChanged();
            return;
        }

        SupplementAdjustment adjustment;
        adjustment.hasParsed = true;
        adjustment.sourceText = trimmedText;
        adjustment.summary =
            parsedObject.value(QStringLiteral("summary")).toString().trimmed();
        adjustment.hungerIntent =
            jsonDouble(parsedObject, QStringLiteral("hunger_intent"), 0.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("hunger_boost"), 0.0, 3.0) /
                           3.0);
        adjustment.carbIntent =
            jsonDouble(parsedObject, QStringLiteral("carb_intent"), -1.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("carb_preference"), -3.0, 3.0) /
                           3.0);
        adjustment.drinkIntent =
            jsonDouble(parsedObject, QStringLiteral("drink_intent"), 0.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("beverage_preference"), 0.0, 3.0) /
                           3.0);
        adjustment.budgetFlexIntent =
            jsonDouble(parsedObject, QStringLiteral("budget_flex_intent"), -1.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("budget_flexibility"), -2.0, 3.0) /
                           3.0);
        adjustment.skipClassConstraint =
            parsedObject.value(QStringLiteral("skip_class_constraint")).toBool() ||
            parsedObject.value(QStringLiteral("skip_afternoon_class")).toBool() ||
            parsedObject.value(QStringLiteral("skip_evening_class")).toBool();
        adjustment.postMealSleepPlan =
            parsedObject.value(QStringLiteral("post_meal_sleep_plan"))
                .toString(QStringLiteral("unknown"))
                .trimmed();
        adjustment.plannedNapMinutes =
            static_cast<int>(std::round(jsonDouble(
                parsedObject, QStringLiteral("planned_nap_minutes"), 0.0, 180.0)));
        adjustment.sleepNeedLevel =
            jsonDouble(parsedObject, QStringLiteral("sleep_need_level"), 0.0, 1.0);
        adjustment.sleepPlanConfidence =
            jsonDouble(parsedObject, QStringLiteral("sleep_plan_confidence"), 0.0, 1.0);
        adjustment.proteinIntent =
            jsonDouble(parsedObject, QStringLiteral("protein_intent"), 0.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("protein_preference"), 0.0, 3.0) /
                           3.0);
        adjustment.colaIntent =
            jsonDouble(parsedObject, QStringLiteral("cola_intent"), 0.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("cola_preference"), 0.0, 3.0) /
                           3.0);
        adjustment.flavorIntent =
            jsonDouble(parsedObject, QStringLiteral("flavor_intent"), 0.0, 1.0,
                       jsonDouble(parsedObject, QStringLiteral("flavor_preference"), 0.0, 3.0) /
                           3.0);
        adjustment.relaxedTimePreference =
            parsedObject.value(QStringLiteral("relaxed_time_preference")).toBool();

        const QStringList validSleepPlans = {QStringLiteral("stay_awake"),
                                             QStringLiteral("nap_before_class"),
                                             QStringLiteral("no_class"),
                                             QStringLiteral("unknown")};
        if (!validSleepPlans.contains(adjustment.postMealSleepPlan)) {
            adjustment.postMealSleepPlan = QStringLiteral("unknown");
        }

        applyParsedSupplement(adjustment);
    });
}

void RecommendationEngine::clearSupplement()
{
    m_adjustment = SupplementAdjustment();
    m_supplementSummary = QStringLiteral("当前没有补充说明。");
    m_supplementStatus = QStringLiteral("已清空补充说明。");
    m_supplementWeights.clear();
    emit supplementChanged();
}

void RecommendationEngine::setWeightOverrides(const QVariantMap &overrides)
{
    m_weightOverrides = overrides;
    runDecision();
}

void RecommendationEngine::clearWeightOverrides()
{
    m_weightOverrides.clear();
    runDecision();
}

void RecommendationEngine::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void RecommendationEngine::setInitialState()
{
    m_summary = QStringLiteral("点击“生成推荐”后会按当前时间窗口运行 V2 推荐。");
    m_supplementSummary = QStringLiteral("当前没有补充说明。");
    m_supplementStatus = apiConfigured()
                             ? QStringLiteral("补充说明解析器已就绪。")
                             : QStringLiteral(
                                   "补充说明解析器未启用，请先配置 API 环境变量。");
    m_supplementWeights.clear();
    m_activeWeightConfig.clear();
}

void RecommendationEngine::applyParsedSupplement(
    const SupplementAdjustment &adjustment)
{
    m_adjustment = adjustment;
    m_supplementSummary = adjustment.summary.isEmpty()
                              ? QStringLiteral("补充说明已解析，但没有返回额外摘要。")
                              : adjustment.summary;
    m_supplementStatus = QStringLiteral("补充说明解析成功。");
    m_supplementWeights = buildWeightList(adjustment);
    emit supplementChanged();
}
#endif

RecommendationEngine::SupplementAdjustment
RecommendationEngine::neutralSupplementAdjustment()
{
    return SupplementAdjustment();
}

RecommendationEngine::SupplementParseOutcome
RecommendationEngine::evaluateSupplementResponse(const QString &sourceText,
                                                 const QByteArray &responseBody,
                                                 const QString &networkError,
                                                 bool timedOut)
{
    SupplementParseOutcome outcome;
    outcome.adjustment = neutralSupplementAdjustment();
    outcome.adjustment.hasParsed = true;
    outcome.adjustment.sourceText = sourceText.trimmed();

    if (!networkError.trimmed().isEmpty()) {
        outcome.state = QStringLiteral("network_error");
        outcome.status = timedOut
                             ? QStringLiteral("补充说明解析请求超时，已回退到默认参数。")
                             : QStringLiteral("网络或接口失败：%1。已回退到默认参数。")
                                   .arg(networkError.trimmed());
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    QJsonObject contractObject;
    QString validationError;
    if (!extractContractObjectFromApiResponse(responseBody, &contractObject,
                                              &validationError)) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回内容不是严格 JSON：%1。已回退到默认参数。")
                             .arg(validationError);
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    if (!validateExactKeys(contractObject, kSupplementTopLevelKeys,
                           &validationError)) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回 JSON 顶层结构非法：%1。已回退到默认参数。")
                             .arg(validationError);
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    const QString version =
        contractObject.value(QStringLiteral("version")).toString().trimmed();
    if (version != QStringLiteral("supplement_parser_v1")) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回 JSON version 非法，已回退到默认参数。");
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    if (!contractObject.value(QStringLiteral("result")).isObject()) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回 JSON result 结构非法，已回退到默认参数。");
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    const QJsonObject resultObject =
        contractObject.value(QStringLiteral("result")).toObject();
    if (!validateExactKeys(resultObject, kSupplementResultKeys,
                           &validationError)) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回 JSON result 字段非法：%1。已回退到默认参数。")
                             .arg(validationError);
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    SupplementAdjustment parsedAdjustment = neutralSupplementAdjustment();
    parsedAdjustment.hasParsed = true;
    parsedAdjustment.sourceText = sourceText.trimmed();

    if (!readAllowedDouble(resultObject, QStringLiteral("hungerIntent"),
                           kWeakIntentValues, &parsedAdjustment.hungerIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("carbIntent"),
                           kWeakIntentValues, &parsedAdjustment.carbIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("drinkIntent"),
                           kWeakIntentValues, &parsedAdjustment.drinkIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("budgetFlexIntent"),
                           kWeakIntentValues,
                           &parsedAdjustment.budgetFlexIntent, &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("classConstraintWeight"),
                           kStrongIntentValues,
                           &parsedAdjustment.classConstraintWeight,
                           &validationError) ||
        !readAllowedString(resultObject, QStringLiteral("postMealSleepPlan"),
                           kSleepPlanValues,
                           &parsedAdjustment.postMealSleepPlan,
                           &validationError) ||
        !readAllowedInt(resultObject, QStringLiteral("plannedNapMinutes"),
                        kNapMinuteValues,
                        &parsedAdjustment.plannedNapMinutes, &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("sleepNeedLevel"),
                           kStrongIntentValues,
                           &parsedAdjustment.sleepNeedLevel, &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("sleepPlanConfidence"),
                           kGovernanceValues,
                           &parsedAdjustment.sleepPlanConfidence,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("proteinIntent"),
                           kWeakIntentValues, &parsedAdjustment.proteinIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("colaIntent"),
                           kWeakIntentValues, &parsedAdjustment.colaIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("flavorIntent"),
                           kWeakIntentValues, &parsedAdjustment.flavorIntent,
                           &validationError) ||
        !readAllowedDouble(resultObject, QStringLiteral("relaxedTimePreference"),
                           kStrongIntentValues,
                           &parsedAdjustment.relaxedTimePreference,
                           &validationError)) {
        outcome.state = QStringLiteral("invalid_response");
        outcome.status = QStringLiteral("返回 JSON 值非法：%1。已回退到默认参数。")
                             .arg(validationError);
        outcome.fallbackUsed = true;
        outcome.adjustment.fallbackUsed = true;
        return outcome;
    }

    outcome.adjustment = parsedAdjustment;
    outcome.state = QStringLiteral("success");
    outcome.status = QStringLiteral("补充说明解析成功。");
    outcome.accepted = true;
    return outcome;
}

void RecommendationEngine::parseSupplement(const QString &text)
{
    const QString trimmedText = text.trimmed();
    if (trimmedText.isEmpty()) {
        m_supplementState = apiConfigured() ? QStringLiteral("ready")
                                            : QStringLiteral("unconfigured");
        m_supplementFallbackActive = false;
        m_supplementStatus = QStringLiteral("请先输入补充说明。");
        emit supplementChanged();
        return;
    }

    if (!apiConfigured()) {
        SupplementParseOutcome outcome;
        outcome.adjustment = neutralSupplementAdjustment();
        outcome.adjustment.hasParsed = true;
        outcome.adjustment.fallbackUsed = true;
        outcome.adjustment.sourceText = trimmedText;
        outcome.state = QStringLiteral("unconfigured");
        outcome.status = QStringLiteral(
            "LLM API 未配置，已回退到默认参数。请先设置 API Key，并按需补充 API URL / Model。");
        outcome.fallbackUsed = true;
        applySupplementOutcome(outcome);
        return;
    }

    PlanningRepository planningRepository(m_databaseManager.connectionName());
    ScheduleRepository scheduleRepository(m_databaseManager.connectionName());
    const MealContext context =
        buildMealContext(planningRepository.loadPlanningPolicies(),
                         planningRepository.loadRecommendationProfiles(),
                         scheduleRepository);

    setBusy(true);
    m_supplementState = QStringLiteral("parsing");
    m_supplementFallbackActive = false;
    m_supplementStatus = QStringLiteral("正在解析补充说明...");
    m_supplementSummary = QStringLiteral("正在等待模型返回严格 JSON。");
    emit supplementChanged();

    QNetworkRequest request{QUrl(AppConfig::llmApiUrl())};
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1")
                             .arg(AppConfig::llmApiKey())
                             .toUtf8());

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"), supplementParserSystemPrompt()}
    });
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), supplementParserUserPrompt(context, trimmedText)}
    });

    QJsonObject payload{
        {QStringLiteral("model"), AppConfig::llmModel()},
        {QStringLiteral("messages"), messages},
        {QStringLiteral("temperature"), 0},
        {QStringLiteral("response_format"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("json_object")}}}
    };

    QNetworkReply *reply = m_networkAccessManager.post(
        request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (!reply->isFinished()) {
            reply->setProperty("mealadvisorTimedOut", true);
            reply->abort();
        }
    });
    timeoutTimer->start(kSupplementTimeoutMs);

    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedText]() {
        const QByteArray responseBody = reply->readAll();
        const QString networkError = reply->error() == QNetworkReply::NoError
                                         ? QString()
                                         : reply->errorString();
        const bool timedOut = reply->property("mealadvisorTimedOut").toBool();
        reply->deleteLater();
        setBusy(false);

        applySupplementOutcome(
            evaluateSupplementResponse(trimmedText, responseBody, networkError,
                                       timedOut));
    });
}

void RecommendationEngine::clearSupplement()
{
    m_adjustment = neutralSupplementAdjustment();
    m_adjustment.hasParsed = false;
    m_supplementFallbackActive = false;
    m_supplementSummary = QStringLiteral("当前没有补充说明。");
    m_supplementStatus = QStringLiteral("已清空补充说明。");
    m_supplementState = apiConfigured() ? QStringLiteral("ready")
                                        : QStringLiteral("unconfigured");
    m_supplementWeights.clear();
    emit supplementChanged();
}

void RecommendationEngine::setWeightOverrides(const QVariantMap &overrides)
{
    m_weightOverrides = overrides;
    runDecision();
}

void RecommendationEngine::clearWeightOverrides()
{
    m_weightOverrides.clear();
    runDecision();
}

void RecommendationEngine::refreshSupplementConfigState()
{
    if (m_busy) {
        emit supplementChanged();
        return;
    }

    if (!m_adjustment.hasParsed || m_supplementState == QStringLiteral("unconfigured")) {
        m_supplementState = apiConfigured() ? QStringLiteral("ready")
                                            : QStringLiteral("unconfigured");
        m_supplementStatus = apiConfigured()
                                 ? QStringLiteral("补充说明解析器已就绪。")
                                 : QStringLiteral("补充说明解析器未配置，当前会回退到默认参数。");
        if (!m_adjustment.hasParsed) {
            m_supplementSummary = QStringLiteral("当前没有补充说明。");
        }
        m_supplementFallbackActive = false;
    }

    emit supplementChanged();
}

void RecommendationEngine::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void RecommendationEngine::setInitialState()
{
    m_adjustment = neutralSupplementAdjustment();
    m_adjustment.hasParsed = false;
    m_summary = QStringLiteral("点击“生成推荐”后会按当前时间窗口运行 V2 推荐。");
    m_supplementSummary = QStringLiteral("当前没有补充说明。");
    m_supplementStatus = apiConfigured()
                             ? QStringLiteral("补充说明解析器已就绪。")
                             : QStringLiteral("补充说明解析器未配置，当前会回退到默认参数。");
    m_supplementState = apiConfigured() ? QStringLiteral("ready")
                                        : QStringLiteral("unconfigured");
    m_supplementFallbackActive = false;
    m_supplementWeights.clear();
    m_activeWeightConfig.clear();
}

void RecommendationEngine::applySupplementOutcome(
    const SupplementParseOutcome &outcome)
{
    m_adjustment = outcome.adjustment;
    m_supplementState = outcome.state;
    m_supplementStatus = outcome.status;
    m_supplementFallbackActive = outcome.fallbackUsed;
    m_supplementWeights = buildWeightList(outcome.adjustment);

    if (!outcome.adjustment.hasParsed) {
        m_supplementSummary = QStringLiteral("当前没有补充说明。");
    } else if (m_supplementWeights.size() == 1 &&
               m_supplementWeights.first().toMap().value(QStringLiteral("label")).toString() ==
                   QStringLiteral("补充输入")) {
        m_supplementSummary = outcome.fallbackUsed
                                  ? QStringLiteral("当前已回退到中性默认补充参数。")
                                  : QStringLiteral("解析结果为中性默认值。");
    } else {
        QStringList labels;
        for (const QVariant &weightVariant : m_supplementWeights) {
            const QString label =
                weightVariant.toMap().value(QStringLiteral("label")).toString();
            if (!label.isEmpty()) {
                labels.append(label);
            }
        }
        m_supplementSummary = QStringLiteral("已解析 %1 项临时调整：%2")
                                  .arg(labels.size())
                                  .arg(labels.join(QStringLiteral("、")));
    }

    m_adjustment.summary = m_supplementSummary;
    emit supplementChanged();
}
