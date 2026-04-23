#include "foodmanager.h"

#include "../data/dishrepository.h"
#include "../data/meallogrepository.h"
#include "../data/merchantrepository.h"

#include <algorithm>
#include <QHash>
#include <QSet>
#include <QVariantMap>

namespace
{
QString normalizedKey(const QString &value)
{
    return value.trimmed().toLower();
}

bool isOneOf(const QString &value, const QSet<QString> &allowedValues)
{
    return allowedValues.contains(normalizedKey(value));
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

void sortMerchantMatches(QVariantList *items, const QString &searchText)
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
                          {QStringLiteral("name"), QStringLiteral("campusArea"),
                           QStringLiteral("notes"), QStringLiteral("priceLevel")});
                      const int rightPriority = searchPriority(
                          right, loweredSearch,
                          {QStringLiteral("name"), QStringLiteral("campusArea"),
                           QStringLiteral("notes"), QStringLiteral("priceLevel")});
                      if (leftPriority != rightPriority) {
                          return leftPriority < rightPriority;
                      }
                  }

                  const QString leftName =
                      left.value(QStringLiteral("name")).toString().trimmed().toLower();
                  const QString rightName =
                      right.value(QStringLiteral("name")).toString().trimmed().toLower();
                  if (leftName != rightName) {
                      return leftName < rightName;
                  }

                  return left.value(QStringLiteral("id")).toInt() <
                         right.value(QStringLiteral("id")).toInt();
              });
}

void sortDishMatches(QVariantList *items, const QString &searchText)
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
                           QStringLiteral("defaultDiningMode"),
                           QStringLiteral("digestiveBurdenLevel"),
                           QStringLiteral("sleepinessRiskLevel"),
                           QStringLiteral("notes")});
                      const int rightPriority = searchPriority(
                          right, loweredSearch,
                          {QStringLiteral("name"), QStringLiteral("merchantName"),
                           QStringLiteral("category"),
                           QStringLiteral("defaultDiningMode"),
                           QStringLiteral("digestiveBurdenLevel"),
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

                  const QString leftMerchant =
                      left.value(QStringLiteral("merchantName")).toString().trimmed().toLower();
                  const QString rightMerchant =
                      right.value(QStringLiteral("merchantName")).toString().trimmed().toLower();
                  if (leftMerchant != rightMerchant) {
                      return leftMerchant < rightMerchant;
                  }

                  const QString leftName =
                      left.value(QStringLiteral("name")).toString().trimmed().toLower();
                  const QString rightName =
                      right.value(QStringLiteral("name")).toString().trimmed().toLower();
                  if (leftName != rightName) {
                      return leftName < rightName;
                  }

                  return left.value(QStringLiteral("id")).toInt() <
                         right.value(QStringLiteral("id")).toInt();
              });
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

        if (topItems.size() >= 5) {
            break;
        }
    }

    return topItems;
}
}

FoodManager::FoodManager(const DatabaseManager &databaseManager, QObject *parent)
    : QObject(parent),
      m_databaseManager(databaseManager)
{
    refreshState();
}

QVariantList FoodManager::merchants() const
{
    return m_merchants;
}

QVariantList FoodManager::dishes() const
{
    return m_dishes;
}

QVariantList FoodManager::filteredMerchants() const
{
    return m_filteredMerchants;
}

QVariantList FoodManager::filteredDishes() const
{
    return m_filteredDishes;
}

QVariantList FoodManager::frequentMerchants() const
{
    return m_frequentMerchants;
}

QVariantList FoodManager::frequentDishes() const
{
    return m_frequentDishes;
}

QString FoodManager::merchantSearch() const
{
    return m_merchantSearch;
}

QString FoodManager::dishSearch() const
{
    return m_dishSearch;
}

int FoodManager::dishMerchantFilterId() const
{
    return m_dishMerchantFilterId;
}

int FoodManager::merchantCount() const
{
    return m_merchantCount;
}

int FoodManager::dishCount() const
{
    return m_dishCount;
}

QString FoodManager::lastError() const
{
    return m_lastError;
}

void FoodManager::reload()
{
    refreshState();
    emit foodChanged();
}

void FoodManager::setMerchantSearch(const QString &searchText)
{
    m_merchantSearch = searchText.trimmed();
    refreshFilteredState();
    emit foodChanged();
}

void FoodManager::setDishSearch(const QString &searchText)
{
    m_dishSearch = searchText.trimmed();
    refreshFilteredState();
    emit foodChanged();
}

void FoodManager::setDishMerchantFilterId(int merchantId)
{
    m_dishMerchantFilterId = merchantId > 0 ? merchantId : 0;
    refreshFilteredState();
    emit foodChanged();
}

bool FoodManager::addMerchant(const QString &name,
                              const QString &campusArea,
                              const QString &priceLevel,
                              bool supportsDineIn,
                              bool supportsTakeaway,
                              bool supportsDelivery,
                              int deliveryEtaMinutes,
                              int distanceMinutes,
                              int queueTimeMinutes,
                              const QString &notes)
{
    const QString normalizedPriceLevel = normalizedKey(priceLevel);

    if (name.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Merchant name is required.");
        emit foodChanged();
        return false;
    }

    if (!isOneOf(normalizedPriceLevel,
                 {QStringLiteral("budget"), QStringLiteral("mid"),
                  QStringLiteral("high")})) {
        m_lastError = QStringLiteral("Merchant price level must be budget, mid, or high.");
        emit foodChanged();
        return false;
    }

    if (deliveryEtaMinutes < 0 || distanceMinutes < 0 || queueTimeMinutes < 0) {
        m_lastError = QStringLiteral("Merchant timing fields cannot be negative.");
        emit foodChanged();
        return false;
    }

    if (!supportsDineIn && !supportsTakeaway && !supportsDelivery) {
        m_lastError = QStringLiteral("Choose at least one supported dining mode.");
        emit foodChanged();
        return false;
    }

    Merchant merchant;
    merchant.name = name.trimmed();
    merchant.campusArea = campusArea.trimmed();
    merchant.priceLevel = normalizedPriceLevel;
    merchant.supportsDineIn = supportsDineIn;
    merchant.supportsTakeaway = supportsTakeaway;
    merchant.supportsDelivery = supportsDelivery;
    merchant.deliveryEtaMinutes = deliveryEtaMinutes;
    merchant.distanceMinutes = distanceMinutes;
    merchant.queueTimeMinutes = queueTimeMinutes;
    merchant.notes = notes.trimmed();

    MerchantRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.addMerchant(merchant, &errorMessage)) {
        m_lastError = errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool FoodManager::updateMerchant(int merchantId,
                                 const QString &name,
                                 const QString &campusArea,
                                 const QString &priceLevel,
                                 bool supportsDineIn,
                                 bool supportsTakeaway,
                                 bool supportsDelivery,
                                 int deliveryEtaMinutes,
                                 int distanceMinutes,
                                 int queueTimeMinutes,
                                 const QString &notes)
{
    const QString normalizedPriceLevel = normalizedKey(priceLevel);

    if (merchantId <= 0 || name.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Valid merchant data is required.");
        emit foodChanged();
        return false;
    }

    if (!isOneOf(normalizedPriceLevel,
                 {QStringLiteral("budget"), QStringLiteral("mid"),
                  QStringLiteral("high")})) {
        m_lastError = QStringLiteral("Merchant price level must be budget, mid, or high.");
        emit foodChanged();
        return false;
    }

    if (deliveryEtaMinutes < 0 || distanceMinutes < 0 || queueTimeMinutes < 0) {
        m_lastError = QStringLiteral("Merchant timing fields cannot be negative.");
        emit foodChanged();
        return false;
    }

    if (!supportsDineIn && !supportsTakeaway && !supportsDelivery) {
        m_lastError = QStringLiteral("Choose at least one supported dining mode.");
        emit foodChanged();
        return false;
    }

    Merchant merchant;
    merchant.id = merchantId;
    merchant.name = name.trimmed();
    merchant.campusArea = campusArea.trimmed();
    merchant.priceLevel = normalizedPriceLevel;
    merchant.supportsDineIn = supportsDineIn;
    merchant.supportsTakeaway = supportsTakeaway;
    merchant.supportsDelivery = supportsDelivery;
    merchant.deliveryEtaMinutes = deliveryEtaMinutes;
    merchant.distanceMinutes = distanceMinutes;
    merchant.queueTimeMinutes = queueTimeMinutes;
    merchant.notes = notes.trimmed();

    MerchantRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.updateMerchant(merchant, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to update merchant.")
                          : errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool FoodManager::deleteMerchant(int merchantId)
{
    MerchantRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.deleteMerchant(merchantId, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to delete merchant.")
                          : errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool FoodManager::addDish(const QString &name,
                          int merchantId,
                          const QString &category,
                          double price,
                          const QString &defaultDiningMode,
                          int eatTimeMinutes,
                          int acquireEffortScore,
                          const QString &carbLevel,
                          const QString &fatLevel,
                          const QString &proteinLevel,
                          const QString &vitaminLevel,
                          const QString &fiberLevel,
                          const QString &satietyLevel,
                          const QString &digestiveBurdenLevel,
                          const QString &sleepinessRiskLevel,
                          const QString &flavorLevel,
                          const QString &odorLevel,
                          bool isCombo,
                          bool isBeverage,
                          double mealImpactWeight,
                          const QString &notes)
{
    const QString normalizedDiningMode = normalizedKey(defaultDiningMode);
    const QString normalizedCarbLevel = normalizedKey(carbLevel);
    const QString normalizedFatLevel = normalizedKey(fatLevel);
    const QString normalizedProteinLevel = normalizedKey(proteinLevel);
    const QString normalizedVitaminLevel = normalizedKey(vitaminLevel);
    const QString normalizedFiberLevel = normalizedKey(fiberLevel);
    const QString normalizedSatietyLevel = normalizedKey(satietyLevel);
    const QString normalizedDigestiveBurdenLevel =
        normalizedKey(digestiveBurdenLevel);
    const QString normalizedSleepinessRiskLevel =
        normalizedKey(sleepinessRiskLevel);
    const QString normalizedFlavorLevel = normalizedKey(flavorLevel);
    const QString normalizedOdorLevel = normalizedKey(odorLevel);
    const QSet<QString> allowedLevels = {QStringLiteral("low"),
                                         QStringLiteral("medium"),
                                         QStringLiteral("high")};

    if (name.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Dish name is required.");
        emit foodChanged();
        return false;
    }

    if (merchantId <= 0) {
        m_lastError = QStringLiteral("Please choose one merchant.");
        emit foodChanged();
        return false;
    }

    if (!isOneOf(normalizedDiningMode,
                 {QStringLiteral("dine_in"), QStringLiteral("takeaway"),
                  QStringLiteral("delivery")})) {
        m_lastError = QStringLiteral("Dish dining mode must be dine_in, takeaway, or delivery.");
        emit foodChanged();
        return false;
    }

    if (!allowedLevels.contains(normalizedCarbLevel) ||
        !allowedLevels.contains(normalizedFatLevel) ||
        !allowedLevels.contains(normalizedProteinLevel) ||
        !allowedLevels.contains(normalizedVitaminLevel) ||
        !allowedLevels.contains(normalizedFiberLevel) ||
        !allowedLevels.contains(normalizedSatietyLevel) ||
        !allowedLevels.contains(normalizedDigestiveBurdenLevel) ||
        !allowedLevels.contains(normalizedSleepinessRiskLevel) ||
        !allowedLevels.contains(normalizedFlavorLevel) ||
        !allowedLevels.contains(normalizedOdorLevel)) {
        m_lastError = QStringLiteral("Dish nutrition and tag levels must stay within low, medium, or high.");
        emit foodChanged();
        return false;
    }

    if (price < 0.0 || eatTimeMinutes <= 0 || eatTimeMinutes > 240 ||
        acquireEffortScore < 1 || acquireEffortScore > 3 ||
        mealImpactWeight <= 0.0 || mealImpactWeight > 3.0) {
        m_lastError = QStringLiteral("Dish numeric fields are out of the supported range.");
        emit foodChanged();
        return false;
    }

    Dish dish;
    dish.name = name.trimmed();
    dish.merchantId = merchantId;
    dish.category = category.trimmed();
    dish.price = price;
    dish.defaultDiningMode = normalizedDiningMode;
    dish.eatTimeMinutes = eatTimeMinutes;
    dish.acquireEffortScore = acquireEffortScore;
    dish.carbLevel = normalizedCarbLevel;
    dish.fatLevel = normalizedFatLevel;
    dish.proteinLevel = normalizedProteinLevel;
    dish.vitaminLevel = normalizedVitaminLevel;
    dish.fiberLevel = normalizedFiberLevel;
    dish.satietyLevel = normalizedSatietyLevel;
    dish.digestiveBurdenLevel = normalizedDigestiveBurdenLevel;
    dish.sleepinessRiskLevel = normalizedSleepinessRiskLevel;
    dish.flavorLevel = normalizedFlavorLevel;
    dish.odorLevel = normalizedOdorLevel;
    dish.isCombo = isCombo;
    dish.isBeverage = isBeverage;
    dish.mealImpactWeight = mealImpactWeight;
    dish.notes = notes.trimmed();
    dish.sourceType = QStringLiteral("manual");

    DishRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.addDish(dish, &errorMessage)) {
        m_lastError = errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool FoodManager::updateDish(int dishId,
                             const QString &name,
                             int merchantId,
                             const QString &category,
                             double price,
                             const QString &defaultDiningMode,
                             int eatTimeMinutes,
                             int acquireEffortScore,
                             const QString &carbLevel,
                             const QString &fatLevel,
                             const QString &proteinLevel,
                             const QString &vitaminLevel,
                             const QString &fiberLevel,
                             const QString &satietyLevel,
                             const QString &digestiveBurdenLevel,
                             const QString &sleepinessRiskLevel,
                             const QString &flavorLevel,
                             const QString &odorLevel,
                             bool isCombo,
                             bool isBeverage,
                             double mealImpactWeight,
                             const QString &notes)
{
    const QString normalizedDiningMode = normalizedKey(defaultDiningMode);
    const QString normalizedCarbLevel = normalizedKey(carbLevel);
    const QString normalizedFatLevel = normalizedKey(fatLevel);
    const QString normalizedProteinLevel = normalizedKey(proteinLevel);
    const QString normalizedVitaminLevel = normalizedKey(vitaminLevel);
    const QString normalizedFiberLevel = normalizedKey(fiberLevel);
    const QString normalizedSatietyLevel = normalizedKey(satietyLevel);
    const QString normalizedDigestiveBurdenLevel =
        normalizedKey(digestiveBurdenLevel);
    const QString normalizedSleepinessRiskLevel =
        normalizedKey(sleepinessRiskLevel);
    const QString normalizedFlavorLevel = normalizedKey(flavorLevel);
    const QString normalizedOdorLevel = normalizedKey(odorLevel);
    const QSet<QString> allowedLevels = {QStringLiteral("low"),
                                         QStringLiteral("medium"),
                                         QStringLiteral("high")};

    if (dishId <= 0 || name.trimmed().isEmpty() || merchantId <= 0) {
        m_lastError = QStringLiteral("Valid dish data is required.");
        emit foodChanged();
        return false;
    }

    if (!isOneOf(normalizedDiningMode,
                 {QStringLiteral("dine_in"), QStringLiteral("takeaway"),
                  QStringLiteral("delivery")})) {
        m_lastError = QStringLiteral("Dish dining mode must be dine_in, takeaway, or delivery.");
        emit foodChanged();
        return false;
    }

    if (!allowedLevels.contains(normalizedCarbLevel) ||
        !allowedLevels.contains(normalizedFatLevel) ||
        !allowedLevels.contains(normalizedProteinLevel) ||
        !allowedLevels.contains(normalizedVitaminLevel) ||
        !allowedLevels.contains(normalizedFiberLevel) ||
        !allowedLevels.contains(normalizedSatietyLevel) ||
        !allowedLevels.contains(normalizedDigestiveBurdenLevel) ||
        !allowedLevels.contains(normalizedSleepinessRiskLevel) ||
        !allowedLevels.contains(normalizedFlavorLevel) ||
        !allowedLevels.contains(normalizedOdorLevel)) {
        m_lastError = QStringLiteral("Dish nutrition and tag levels must stay within low, medium, or high.");
        emit foodChanged();
        return false;
    }

    if (price < 0.0 || eatTimeMinutes <= 0 || eatTimeMinutes > 240 ||
        acquireEffortScore < 1 || acquireEffortScore > 3 ||
        mealImpactWeight <= 0.0 || mealImpactWeight > 3.0) {
        m_lastError = QStringLiteral("Dish numeric fields are out of the supported range.");
        emit foodChanged();
        return false;
    }

    Dish dish;
    dish.id = dishId;
    dish.name = name.trimmed();
    dish.merchantId = merchantId;
    dish.category = category.trimmed();
    dish.price = price;
    dish.defaultDiningMode = normalizedDiningMode;
    dish.eatTimeMinutes = eatTimeMinutes;
    dish.acquireEffortScore = acquireEffortScore;
    dish.carbLevel = normalizedCarbLevel;
    dish.fatLevel = normalizedFatLevel;
    dish.proteinLevel = normalizedProteinLevel;
    dish.vitaminLevel = normalizedVitaminLevel;
    dish.fiberLevel = normalizedFiberLevel;
    dish.satietyLevel = normalizedSatietyLevel;
    dish.digestiveBurdenLevel = normalizedDigestiveBurdenLevel;
    dish.sleepinessRiskLevel = normalizedSleepinessRiskLevel;
    dish.flavorLevel = normalizedFlavorLevel;
    dish.odorLevel = normalizedOdorLevel;
    dish.isCombo = isCombo;
    dish.isBeverage = isBeverage;
    dish.mealImpactWeight = mealImpactWeight;
    dish.notes = notes.trimmed();
    dish.isActive = true;
    dish.sourceType = QStringLiteral("manual");

    DishRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.updateDish(dish, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to update dish.")
                          : errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

bool FoodManager::deleteDish(int dishId)
{
    DishRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.archiveDish(dishId, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to archive dish.")
                          : errorMessage;
        emit foodChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

void FoodManager::refreshState()
{
    MerchantRepository merchantRepository(m_databaseManager.connectionName());
    DishRepository dishRepository(m_databaseManager.connectionName());
    MealLogRepository mealLogRepository(m_databaseManager.connectionName());

    const QList<Merchant> merchants = merchantRepository.loadAllMerchants();
    const QList<Dish> dishes = dishRepository.loadActiveDishes();
    const QList<int> recentDishIds = mealLogRepository.loadRecentDishIds(20);

    m_merchants.clear();
    m_dishes.clear();
    m_merchantCount = merchants.size();
    m_dishCount = dishes.size();

    QHash<int, QString> merchantNameById;
    for (const Merchant &merchant : merchants) {
        merchantNameById.insert(merchant.id, merchant.name);

        QVariantMap merchantMap;
        merchantMap.insert(QStringLiteral("id"), merchant.id);
        merchantMap.insert(QStringLiteral("name"), merchant.name);
        merchantMap.insert(QStringLiteral("campusArea"), merchant.campusArea);
        merchantMap.insert(QStringLiteral("priceLevel"), merchant.priceLevel);
        merchantMap.insert(QStringLiteral("supportsDineIn"),
                           merchant.supportsDineIn);
        merchantMap.insert(QStringLiteral("supportsTakeaway"),
                           merchant.supportsTakeaway);
        merchantMap.insert(QStringLiteral("supportsDelivery"),
                           merchant.supportsDelivery);
        merchantMap.insert(QStringLiteral("deliveryEtaMinutes"),
                           merchant.deliveryEtaMinutes);
        merchantMap.insert(QStringLiteral("distanceMinutes"),
                           merchant.distanceMinutes);
        merchantMap.insert(QStringLiteral("queueTimeMinutes"),
                           merchant.queueTimeMinutes);
        merchantMap.insert(QStringLiteral("notes"), merchant.notes);
        m_merchants.append(merchantMap);
    }

    QHash<int, int> dishFrequency;
    QHash<int, int> merchantFrequency;
    for (const int dishId : recentDishIds) {
        dishFrequency[dishId] += 1;
    }

    for (const Dish &dish : dishes) {
        QVariantMap dishMap;
        dishMap.insert(QStringLiteral("id"), dish.id);
        dishMap.insert(QStringLiteral("name"), dish.name);
        dishMap.insert(QStringLiteral("merchantId"), dish.merchantId);
        dishMap.insert(QStringLiteral("merchantName"),
                       merchantNameById.value(dish.merchantId));
        dishMap.insert(QStringLiteral("category"), dish.category);
        dishMap.insert(QStringLiteral("price"), dish.price);
        dishMap.insert(QStringLiteral("defaultDiningMode"), dish.defaultDiningMode);
        dishMap.insert(QStringLiteral("eatTimeMinutes"), dish.eatTimeMinutes);
        dishMap.insert(QStringLiteral("acquireEffortScore"), dish.acquireEffortScore);
        dishMap.insert(QStringLiteral("carbLevel"), dish.carbLevel);
        dishMap.insert(QStringLiteral("fatLevel"), dish.fatLevel);
        dishMap.insert(QStringLiteral("proteinLevel"), dish.proteinLevel);
        dishMap.insert(QStringLiteral("vitaminLevel"), dish.vitaminLevel);
        dishMap.insert(QStringLiteral("fiberLevel"), dish.fiberLevel);
        dishMap.insert(QStringLiteral("satietyLevel"), dish.satietyLevel);
        dishMap.insert(QStringLiteral("digestiveBurdenLevel"),
                       dish.digestiveBurdenLevel);
        dishMap.insert(QStringLiteral("sleepinessRiskLevel"),
                       dish.sleepinessRiskLevel);
        dishMap.insert(QStringLiteral("flavorLevel"), dish.flavorLevel);
        dishMap.insert(QStringLiteral("odorLevel"), dish.odorLevel);
        dishMap.insert(QStringLiteral("mealImpactWeight"),
                       dish.mealImpactWeight);
        dishMap.insert(QStringLiteral("recentUseCount"),
                       dishFrequency.value(dish.id));
        dishMap.insert(QStringLiteral("notes"), dish.notes);
        dishMap.insert(QStringLiteral("isCombo"), dish.isCombo);
        dishMap.insert(QStringLiteral("isBeverage"), dish.isBeverage);
        m_dishes.append(dishMap);
        if (dish.merchantId > 0) {
            merchantFrequency[dish.merchantId] += dishFrequency.value(dish.id);
        }
    }

    m_frequentMerchants =
        topItemsFromCounts(m_merchants, merchantFrequency, QStringLiteral("id"));
    m_frequentDishes =
        topItemsFromCounts(m_dishes, dishFrequency, QStringLiteral("id"));
    refreshFilteredState();
}

void FoodManager::refreshFilteredState()
{
    m_filteredMerchants.clear();
    m_filteredDishes.clear();

    for (const QVariant &merchantVariant : m_merchants) {
        const QVariantMap merchant = merchantVariant.toMap();
        if (matchesSearch(merchant, m_merchantSearch,
                          {QStringLiteral("name"), QStringLiteral("campusArea"),
                           QStringLiteral("priceLevel"), QStringLiteral("notes")})) {
            m_filteredMerchants.append(merchant);
        }
    }
    sortMerchantMatches(&m_filteredMerchants, m_merchantSearch);

    for (const QVariant &dishVariant : m_dishes) {
        const QVariantMap dish = dishVariant.toMap();
        if (m_dishMerchantFilterId > 0 &&
            dish.value(QStringLiteral("merchantId")).toInt() != m_dishMerchantFilterId) {
            continue;
        }

        if (matchesSearch(dish, m_dishSearch,
                          {QStringLiteral("name"), QStringLiteral("merchantName"),
                           QStringLiteral("category"), QStringLiteral("notes"),
                           QStringLiteral("carbLevel"), QStringLiteral("fatLevel"),
                           QStringLiteral("proteinLevel"), QStringLiteral("vitaminLevel"),
                           QStringLiteral("fiberLevel"), QStringLiteral("satietyLevel"),
                           QStringLiteral("digestiveBurdenLevel"),
                           QStringLiteral("sleepinessRiskLevel"),
                           QStringLiteral("defaultDiningMode"),
                           QStringLiteral("flavorLevel"),
                           QStringLiteral("odorLevel")})) {
            m_filteredDishes.append(dish);
        }
    }
    sortDishMatches(&m_filteredDishes, m_dishSearch);
}

bool FoodManager::matchesSearch(const QVariantMap &item,
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
