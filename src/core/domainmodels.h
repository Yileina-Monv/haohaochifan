#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

struct UserProfile
{
    int id = 0;
    QString displayName;
    QString weightStatus;
    QString visceralFatRisk;
    QString carbSensitivity;
    double defaultBudgetMin = 0.0;
    double defaultBudgetMax = 0.0;
    double bmiValue = 0.0;
    int ageYears = 0;
    QString biologicalSex;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct RecommendationProfile
{
    int id = 0;
    QString name;
    int hasClassPriority = 0;
    int healthPriority = 0;
    int budgetPriority = 0;
    int timeEffortPriority = 0;
    int satietyPriority = 0;
    int carbControlPriority = 0;
    int fatControlPriority = 0;
    int proteinPriority = 0;
    int sleepinessAvoidPriority = 0;
    bool breakfastRecommendationEnabled = false;
    QList<int> enabledWeekdays;
    QString planningScope;
    bool isDefault = false;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct PlanningPolicy
{
    int id = 0;
    QString name;
    QList<int> enabledWeekdays;
    bool includeCommuteDays = true;
    bool skipNonEnabledDays = true;
    double defaultDailyBudget = 0.0;
    double flexibleBudgetCap = 0.0;
    bool breakfastRecommendationEnabled = false;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct ClassPeriod
{
    int id = 0;
    int periodIndex = 0;
    QString startTime;
    QString endTime;
    QString sessionGroup;
};

struct Merchant
{
    int id = 0;
    QString name;
    QString campusArea;
    int distanceMinutes = 0;
    int queueTimeMinutes = 0;
    bool supportsDineIn = true;
    bool supportsTakeaway = false;
    bool supportsDelivery = false;
    int deliveryEtaMinutes = 0;
    QString priceLevel;
    QString notes;
};

struct ScheduleEntry
{
    int id = 0;
    int weekday = 0;
    int periodStart = 0;
    int periodEnd = 0;
    QString courseName;
    QString location;
    QString campusZone;
    QString intensityLevel;
    QString notes;
};

struct Dish
{
    int id = 0;
    QString name;
    int merchantId = 0;
    QString category;
    bool isCombo = false;
    bool isBeverage = false;
    double price = 0.0;
    QString defaultDiningMode;
    int eatTimeMinutes = 0;
    int acquireEffortScore = 0;
    QString carbLevel;
    QString fatLevel;
    QString proteinLevel;
    QString vitaminLevel;
    QString fiberLevel;
    QString vegetableLevel;
    QString satietyLevel;
    QString digestiveBurdenLevel;
    QString sleepinessRiskLevel;
    QString flavorLevel;
    QString odorLevel;
    double mealImpactWeight = 1.0;
    QString notes;
    int healthScore = 0;
    int favoriteScore = 0;
    bool isActive = true;
    QString sourceType = QStringLiteral("manual");
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct MealLog
{
    int id = 0;
    QString mealType;
    QDateTime eatenAt;
    int weekday = 0;
    bool hasClassAfterMeal = false;
    int minutesUntilNextClass = 0;
    QString locationType;
    QString diningMode;
    double totalPrice = 0.0;
    int totalEatTimeMinutes = 0;
    int preMealHungerLevel = 0;
    int preMealEnergyLevel = 0;
    QString moodTag;
    QString notes;
    QDateTime createdAt;
};

struct MealLogDishItem
{
    int dishId = 0;
    double portionRatio = 1.0;
    QString customNotes;
};

struct MealFeedback
{
    int id = 0;
    int mealLogId = 0;
    int recommendationRecordId = 0;
    int fullnessLevel = 0;
    int sleepinessLevel = 0;
    int comfortLevel = 0;
    int focusImpactLevel = 0;
    int tasteRating = 0;
    int repeatWillingness = 0;
    bool wouldEatAgain = false;
    QString freeTextFeedback;
    QDateTime createdAt;
};

struct DishFeedbackAggregate
{
    int dishId = 0;
    int sampleCount = 0;
    int recentSampleCount = 0;
    double effectiveSampleWeight = 0.0;
    double avgTasteRating = 0.0;
    double avgRepeatWillingness = 0.0;
    double avgSleepinessLevel = 0.0;
    double avgComfortLevel = 0.0;
    double avgFocusImpactLevel = 0.0;
    double wouldEatAgainRate = 0.0;
};

struct RecommendationRecordCandidate
{
    int dishId = 0;
    double score = 0.0;
    QString reason;
};

struct RecommendationRecord
{
    int id = 0;
    QString recommendedForMealType;
    QDateTime generatedAt;
    QString contextSummary;
    int strategyProfileId = 0;
    RecommendationRecordCandidate candidate1;
    RecommendationRecordCandidate candidate2;
    RecommendationRecordCandidate candidate3;
    int selectedDishId = 0;
    int selectedMealLogId = 0;
    int selectedCandidateRank = 0;
};

struct RecommendationSelectionLink
{
    int recommendationRecordId = 0;
    int selectedCandidateRank = 0;
    int selectedDishId = 0;
    QString selectedDishName;
    QDateTime generatedAt;
};

QList<int> parseWeekdayList(const QString &serializedWeekdays);
QString serializeWeekdayList(const QList<int> &weekdays);
