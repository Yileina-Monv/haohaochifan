#pragma once

#include "../data/databasemanager.h"

#include <QObject>
#include <QVariantList>

class FoodManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList merchants READ merchants NOTIFY foodChanged)
    Q_PROPERTY(QVariantList dishes READ dishes NOTIFY foodChanged)
    Q_PROPERTY(QVariantList filteredMerchants READ filteredMerchants NOTIFY foodChanged)
    Q_PROPERTY(QVariantList filteredDishes READ filteredDishes NOTIFY foodChanged)
    Q_PROPERTY(QVariantList frequentMerchants READ frequentMerchants NOTIFY foodChanged)
    Q_PROPERTY(QVariantList frequentDishes READ frequentDishes NOTIFY foodChanged)
    Q_PROPERTY(QString merchantSearch READ merchantSearch NOTIFY foodChanged)
    Q_PROPERTY(QString dishSearch READ dishSearch NOTIFY foodChanged)
    Q_PROPERTY(int merchantCount READ merchantCount NOTIFY foodChanged)
    Q_PROPERTY(int dishCount READ dishCount NOTIFY foodChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY foodChanged)

public:
    explicit FoodManager(const DatabaseManager &databaseManager,
                         QObject *parent = nullptr);

    QVariantList merchants() const;
    QVariantList dishes() const;
    QVariantList filteredMerchants() const;
    QVariantList filteredDishes() const;
    QVariantList frequentMerchants() const;
    QVariantList frequentDishes() const;
    QString merchantSearch() const;
    QString dishSearch() const;
    int merchantCount() const;
    int dishCount() const;
    QString lastError() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setMerchantSearch(const QString &searchText);
    Q_INVOKABLE void setDishSearch(const QString &searchText);
    Q_INVOKABLE bool addMerchant(const QString &name,
                                 const QString &campusArea,
                                 const QString &priceLevel,
                                 bool supportsDineIn,
                                 bool supportsTakeaway,
                                 bool supportsDelivery,
                                 int deliveryEtaMinutes,
                                 int distanceMinutes,
                                 int queueTimeMinutes,
                                 const QString &notes);
    Q_INVOKABLE bool updateMerchant(int merchantId,
                                    const QString &name,
                                    const QString &campusArea,
                                    const QString &priceLevel,
                                    bool supportsDineIn,
                                    bool supportsTakeaway,
                                    bool supportsDelivery,
                                    int deliveryEtaMinutes,
                                    int distanceMinutes,
                                    int queueTimeMinutes,
                                    const QString &notes);
    Q_INVOKABLE bool deleteMerchant(int merchantId);
    Q_INVOKABLE bool addDish(const QString &name,
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
                             const QString &notes);
    Q_INVOKABLE bool updateDish(int dishId,
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
                                const QString &notes);
    Q_INVOKABLE bool deleteDish(int dishId);

signals:
    void foodChanged();

private:
    void refreshState();
    void refreshFilteredState();
    static bool matchesSearch(const QVariantMap &item, const QString &searchText,
                              const QStringList &keys);

    const DatabaseManager &m_databaseManager;
    QVariantList m_merchants;
    QVariantList m_dishes;
    QVariantList m_filteredMerchants;
    QVariantList m_filteredDishes;
    QVariantList m_frequentMerchants;
    QVariantList m_frequentDishes;
    QString m_merchantSearch;
    QString m_dishSearch;
    int m_merchantCount = 0;
    int m_dishCount = 0;
    QString m_lastError;
};
