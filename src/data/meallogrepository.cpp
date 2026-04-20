#include "meallogrepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MealLogRepository::MealLogRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

MealLog MealLogRepository::loadMealLog(int mealLogId) const
{
    MealLog mealLog;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, meal_type, eaten_at, weekday, has_class_after_meal, "
        "minutes_until_next_class, location_type, dining_mode, total_price, "
        "total_eat_time_minutes, pre_meal_hunger_level, pre_meal_energy_level, "
        "mood_tag, notes, created_at "
        "FROM meal_logs WHERE id = ?"));
    query.addBindValue(mealLogId);

    if (!query.exec() || !query.next()) {
        return mealLog;
    }

    mealLog.id = query.value(0).toInt();
    mealLog.mealType = query.value(1).toString();
    mealLog.eatenAt = query.value(2).toDateTime();
    mealLog.weekday = query.value(3).toInt();
    mealLog.hasClassAfterMeal = query.value(4).toBool();
    mealLog.minutesUntilNextClass = query.value(5).toInt();
    mealLog.locationType = query.value(6).toString();
    mealLog.diningMode = query.value(7).toString();
    mealLog.totalPrice = query.value(8).toDouble();
    mealLog.totalEatTimeMinutes = query.value(9).toInt();
    mealLog.preMealHungerLevel = query.value(10).toInt();
    mealLog.preMealEnergyLevel = query.value(11).toInt();
    mealLog.moodTag = query.value(12).toString();
    mealLog.notes = query.value(13).toString();
    mealLog.createdAt = query.value(14).toDateTime();
    return mealLog;
}

QList<MealLogDishItem> MealLogRepository::loadMealLogDishItems(int mealLogId) const
{
    QList<MealLogDishItem> items;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT dish_id, portion_ratio, custom_notes "
        "FROM meal_log_dishes WHERE meal_log_id = ? ORDER BY id"));
    query.addBindValue(mealLogId);

    if (!query.exec()) {
        return items;
    }

    while (query.next()) {
        MealLogDishItem item;
        item.dishId = query.value(0).toInt();
        item.portionRatio = query.value(1).toDouble();
        item.customNotes = query.value(2).toString();
        items.append(item);
    }

    return items;
}

bool MealLogRepository::addMealLog(const MealLog &mealLog,
                                   const QList<MealLogDishItem> &dishItems,
                                   int *mealLogId,
                                   QString *errorMessage) const
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    if (!database.transaction()) {
        if (errorMessage != nullptr) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    QSqlQuery mealQuery(database);
    mealQuery.prepare(QStringLiteral(
        "INSERT INTO meal_logs "
        "(meal_type, eaten_at, weekday, has_class_after_meal, "
        "minutes_until_next_class, location_type, dining_mode, total_price, "
        "total_eat_time_minutes, pre_meal_hunger_level, pre_meal_energy_level, "
        "mood_tag, notes, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, datetime('now'))"));
    mealQuery.addBindValue(mealLog.mealType);
    mealQuery.addBindValue(mealLog.eatenAt.toString(Qt::ISODate));
    mealQuery.addBindValue(mealLog.weekday);
    mealQuery.addBindValue(mealLog.hasClassAfterMeal);
    mealQuery.addBindValue(mealLog.minutesUntilNextClass);
    mealQuery.addBindValue(mealLog.locationType);
    mealQuery.addBindValue(mealLog.diningMode);
    mealQuery.addBindValue(mealLog.totalPrice);
    mealQuery.addBindValue(mealLog.totalEatTimeMinutes);
    mealQuery.addBindValue(mealLog.preMealHungerLevel);
    mealQuery.addBindValue(mealLog.preMealEnergyLevel);
    mealQuery.addBindValue(mealLog.moodTag);
    mealQuery.addBindValue(mealLog.notes);

    if (!mealQuery.exec()) {
        database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = mealQuery.lastError().text();
        }
        return false;
    }

    const int insertedMealLogId = mealQuery.lastInsertId().toInt();
    QSqlQuery dishQuery(database);
    dishQuery.prepare(QStringLiteral(
        "INSERT INTO meal_log_dishes (meal_log_id, dish_id, portion_ratio, custom_notes) "
        "VALUES (?, ?, ?, ?)"));

    for (const MealLogDishItem &dishItem : dishItems) {
        dishQuery.addBindValue(insertedMealLogId);
        dishQuery.addBindValue(dishItem.dishId);
        dishQuery.addBindValue(dishItem.portionRatio);
        dishQuery.addBindValue(dishItem.customNotes.isEmpty()
                                   ? QVariant()
                                   : QVariant(dishItem.customNotes));

        if (!dishQuery.exec()) {
            database.rollback();
            if (errorMessage != nullptr) {
                *errorMessage = dishQuery.lastError().text();
            }
            return false;
        }

        dishQuery.finish();
    }

    if (!database.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    if (mealLogId != nullptr) {
        *mealLogId = insertedMealLogId;
    }

    return true;
}

bool MealLogRepository::updateMealLog(int mealLogId,
                                      const MealLog &mealLog,
                                      const QList<MealLogDishItem> &dishItems,
                                      QString *errorMessage) const
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    if (!database.transaction()) {
        if (errorMessage != nullptr) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    QSqlQuery mealQuery(database);
    mealQuery.prepare(QStringLiteral(
        "UPDATE meal_logs SET "
        "meal_type = ?, eaten_at = ?, weekday = ?, has_class_after_meal = ?, "
        "minutes_until_next_class = ?, location_type = ?, dining_mode = ?, "
        "total_price = ?, total_eat_time_minutes = ?, pre_meal_hunger_level = ?, "
        "pre_meal_energy_level = ?, mood_tag = ?, notes = ? "
        "WHERE id = ?"));
    mealQuery.addBindValue(mealLog.mealType);
    mealQuery.addBindValue(mealLog.eatenAt.toString(Qt::ISODate));
    mealQuery.addBindValue(mealLog.weekday);
    mealQuery.addBindValue(mealLog.hasClassAfterMeal);
    mealQuery.addBindValue(mealLog.minutesUntilNextClass);
    mealQuery.addBindValue(mealLog.locationType);
    mealQuery.addBindValue(mealLog.diningMode);
    mealQuery.addBindValue(mealLog.totalPrice);
    mealQuery.addBindValue(mealLog.totalEatTimeMinutes);
    mealQuery.addBindValue(mealLog.preMealHungerLevel);
    mealQuery.addBindValue(mealLog.preMealEnergyLevel);
    mealQuery.addBindValue(mealLog.moodTag);
    mealQuery.addBindValue(mealLog.notes);
    mealQuery.addBindValue(mealLogId);

    if (!mealQuery.exec()) {
        database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = mealQuery.lastError().text();
        }
        return false;
    }

    QSqlQuery clearItemsQuery(database);
    clearItemsQuery.prepare(QStringLiteral(
        "DELETE FROM meal_log_dishes WHERE meal_log_id = ?"));
    clearItemsQuery.addBindValue(mealLogId);
    if (!clearItemsQuery.exec()) {
        database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = clearItemsQuery.lastError().text();
        }
        return false;
    }

    QSqlQuery dishQuery(database);
    dishQuery.prepare(QStringLiteral(
        "INSERT INTO meal_log_dishes (meal_log_id, dish_id, portion_ratio, custom_notes) "
        "VALUES (?, ?, ?, ?)"));

    for (const MealLogDishItem &dishItem : dishItems) {
        dishQuery.addBindValue(mealLogId);
        dishQuery.addBindValue(dishItem.dishId);
        dishQuery.addBindValue(dishItem.portionRatio);
        dishQuery.addBindValue(dishItem.customNotes.isEmpty()
                                   ? QVariant()
                                   : QVariant(dishItem.customNotes));

        if (!dishQuery.exec()) {
            database.rollback();
            if (errorMessage != nullptr) {
                *errorMessage = dishQuery.lastError().text();
            }
            return false;
        }

        dishQuery.finish();
    }

    if (!database.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    return true;
}

bool MealLogRepository::deleteMealLog(int mealLogId, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("DELETE FROM meal_logs WHERE id = ?"));
    query.addBindValue(mealLogId);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<MealLog> MealLogRepository::loadRecentMealLogs(int limit) const
{
    QList<MealLog> mealLogs;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, meal_type, eaten_at, weekday, has_class_after_meal, "
        "minutes_until_next_class, location_type, dining_mode, total_price, "
        "total_eat_time_minutes, pre_meal_hunger_level, pre_meal_energy_level, "
        "mood_tag, notes, created_at "
        "FROM meal_logs ORDER BY eaten_at DESC LIMIT ?"));
    query.addBindValue(limit);

    if (!query.exec()) {
        return mealLogs;
    }

    while (query.next()) {
        MealLog mealLog;
        mealLog.id = query.value(0).toInt();
        mealLog.mealType = query.value(1).toString();
        mealLog.eatenAt = query.value(2).toDateTime();
        mealLog.weekday = query.value(3).toInt();
        mealLog.hasClassAfterMeal = query.value(4).toBool();
        mealLog.minutesUntilNextClass = query.value(5).toInt();
        mealLog.locationType = query.value(6).toString();
        mealLog.diningMode = query.value(7).toString();
        mealLog.totalPrice = query.value(8).toDouble();
        mealLog.totalEatTimeMinutes = query.value(9).toInt();
        mealLog.preMealHungerLevel = query.value(10).toInt();
        mealLog.preMealEnergyLevel = query.value(11).toInt();
        mealLog.moodTag = query.value(12).toString();
        mealLog.notes = query.value(13).toString();
        mealLog.createdAt = query.value(14).toDateTime();
        mealLogs.append(mealLog);
    }

    return mealLogs;
}

QList<int> MealLogRepository::loadRecentDishIds(int limit) const
{
    QList<int> dishIds;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT meal_log_dishes.dish_id "
        "FROM meal_log_dishes "
        "INNER JOIN meal_logs ON meal_logs.id = meal_log_dishes.meal_log_id "
        "ORDER BY meal_logs.eaten_at DESC, meal_log_dishes.id DESC LIMIT ?"));
    query.addBindValue(limit);

    if (!query.exec()) {
        return dishIds;
    }

    while (query.next()) {
        dishIds.append(query.value(0).toInt());
    }

    return dishIds;
}

int MealLogRepository::mealLogCount() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM meal_logs"));
    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}
