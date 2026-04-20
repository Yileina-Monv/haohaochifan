#include "dishrepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DishRepository::DishRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

namespace
{
QList<Dish> loadDishesByQuery(const QString &connectionName, const QString &sql)
{
    QList<Dish> dishes;
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(sql);

    if (!query.exec()) {
        return dishes;
    }

    while (query.next()) {
        Dish dish;
        dish.id = query.value(0).toInt();
        dish.name = query.value(1).toString();
        dish.merchantId = query.value(2).toInt();
        dish.category = query.value(3).toString();
        dish.isCombo = query.value(4).toBool();
        dish.isBeverage = query.value(5).toBool();
        dish.price = query.value(6).toDouble();
        dish.defaultDiningMode = query.value(7).toString();
        dish.eatTimeMinutes = query.value(8).toInt();
        dish.acquireEffortScore = query.value(9).toInt();
        dish.carbLevel = query.value(10).toString();
        dish.fatLevel = query.value(11).toString();
        dish.proteinLevel = query.value(12).toString();
        dish.vitaminLevel = query.value(13).toString();
        dish.fiberLevel = query.value(14).toString();
        dish.vegetableLevel = query.value(15).toString();
        dish.satietyLevel = query.value(16).toString();
        dish.digestiveBurdenLevel = query.value(17).toString();
        dish.sleepinessRiskLevel = query.value(18).toString();
        dish.flavorLevel = query.value(19).toString();
        dish.odorLevel = query.value(20).toString();
        dish.mealImpactWeight = query.value(21).toDouble();
        dish.notes = query.value(22).toString();
        dish.healthScore = query.value(23).toInt();
        dish.favoriteScore = query.value(24).toInt();
        dish.isActive = query.value(25).toBool();
        dish.sourceType = query.value(26).toString();
        dish.createdAt = query.value(27).toDateTime();
        dish.updatedAt = query.value(28).toDateTime();
        dishes.append(dish);
    }

    return dishes;
}
}

QList<Dish> DishRepository::loadActiveDishes() const
{
    return loadDishesByQuery(
        m_connectionName,
        QStringLiteral(
            "SELECT id, name, merchant_id, category, is_combo, is_beverage, price, "
            "default_dining_mode, eat_time_minutes, acquire_effort_score, carb_level, "
            "fat_level, protein_level, vitamin_level, fiber_level, vegetable_level, "
            "satiety_level, digestive_burden_level, sleepiness_risk_level, "
            "flavor_level, odor_level, meal_impact_weight, notes, health_score, "
            "favorite_score, is_active, source_type, created_at, updated_at "
            "FROM dishes WHERE is_active = 1 ORDER BY name"));
}

QList<Dish> DishRepository::loadAllDishes() const
{
    return loadDishesByQuery(
        m_connectionName,
        QStringLiteral(
            "SELECT id, name, merchant_id, category, is_combo, is_beverage, price, "
            "default_dining_mode, eat_time_minutes, acquire_effort_score, carb_level, "
            "fat_level, protein_level, vitamin_level, fiber_level, vegetable_level, "
            "satiety_level, digestive_burden_level, sleepiness_risk_level, "
            "flavor_level, odor_level, meal_impact_weight, notes, health_score, "
            "favorite_score, is_active, source_type, created_at, updated_at "
            "FROM dishes ORDER BY name"));
}

int DishRepository::activeDishCount() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM dishes WHERE is_active = 1"));
    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

bool DishRepository::addDish(const Dish &dish, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "INSERT INTO dishes "
        "(name, merchant_id, category, is_combo, is_beverage, price, "
        "default_dining_mode, eat_time_minutes, acquire_effort_score, carb_level, "
        "fat_level, protein_level, vitamin_level, fiber_level, vegetable_level, "
        "satiety_level, digestive_burden_level, sleepiness_risk_level, "
        "flavor_level, odor_level, meal_impact_weight, notes, health_score, "
        "favorite_score, is_active, source_type, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, "
        "datetime('now'), datetime('now'))"));
    query.addBindValue(dish.name);
    query.addBindValue(dish.merchantId > 0 ? QVariant(dish.merchantId) : QVariant());
    query.addBindValue(dish.category);
    query.addBindValue(dish.isCombo);
    query.addBindValue(dish.isBeverage);
    query.addBindValue(dish.price);
    query.addBindValue(dish.defaultDiningMode);
    query.addBindValue(dish.eatTimeMinutes);
    query.addBindValue(dish.acquireEffortScore);
    query.addBindValue(dish.carbLevel);
    query.addBindValue(dish.fatLevel);
    query.addBindValue(dish.proteinLevel);
    query.addBindValue(dish.vitaminLevel);
    query.addBindValue(dish.fiberLevel);
    query.addBindValue(dish.vegetableLevel);
    query.addBindValue(dish.satietyLevel);
    query.addBindValue(dish.digestiveBurdenLevel);
    query.addBindValue(dish.sleepinessRiskLevel);
    query.addBindValue(dish.flavorLevel);
    query.addBindValue(dish.odorLevel);
    query.addBindValue(dish.mealImpactWeight);
    query.addBindValue(dish.notes);
    query.addBindValue(dish.healthScore);
    query.addBindValue(dish.favoriteScore);
    query.addBindValue(dish.isActive);
    query.addBindValue(dish.sourceType);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

bool DishRepository::updateDish(const Dish &dish, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "UPDATE dishes SET "
        "name = ?, merchant_id = ?, category = ?, is_combo = ?, is_beverage = ?, "
        "price = ?, default_dining_mode = ?, eat_time_minutes = ?, "
        "acquire_effort_score = ?, carb_level = ?, fat_level = ?, "
        "protein_level = ?, vitamin_level = ?, fiber_level = ?, vegetable_level = ?, "
        "satiety_level = ?, digestive_burden_level = ?, sleepiness_risk_level = ?, "
        "flavor_level = ?, odor_level = ?, meal_impact_weight = ?, notes = ?, "
        "health_score = ?, favorite_score = ?, is_active = ?, source_type = ?, "
        "updated_at = datetime('now') "
        "WHERE id = ?"));
    query.addBindValue(dish.name);
    query.addBindValue(dish.merchantId > 0 ? QVariant(dish.merchantId) : QVariant());
    query.addBindValue(dish.category);
    query.addBindValue(dish.isCombo);
    query.addBindValue(dish.isBeverage);
    query.addBindValue(dish.price);
    query.addBindValue(dish.defaultDiningMode);
    query.addBindValue(dish.eatTimeMinutes);
    query.addBindValue(dish.acquireEffortScore);
    query.addBindValue(dish.carbLevel);
    query.addBindValue(dish.fatLevel);
    query.addBindValue(dish.proteinLevel);
    query.addBindValue(dish.vitaminLevel);
    query.addBindValue(dish.fiberLevel);
    query.addBindValue(dish.vegetableLevel);
    query.addBindValue(dish.satietyLevel);
    query.addBindValue(dish.digestiveBurdenLevel);
    query.addBindValue(dish.sleepinessRiskLevel);
    query.addBindValue(dish.flavorLevel);
    query.addBindValue(dish.odorLevel);
    query.addBindValue(dish.mealImpactWeight);
    query.addBindValue(dish.notes);
    query.addBindValue(dish.healthScore);
    query.addBindValue(dish.favoriteScore);
    query.addBindValue(dish.isActive);
    query.addBindValue(dish.sourceType);
    query.addBindValue(dish.id);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool DishRepository::archiveDish(int dishId, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "UPDATE dishes SET is_active = 0, updated_at = datetime('now') WHERE id = ?"));
    query.addBindValue(dishId);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}
