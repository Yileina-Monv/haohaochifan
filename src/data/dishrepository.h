#pragma once

#include "../core/domainmodels.h"

#include <QString>

class DishRepository
{
public:
    explicit DishRepository(QString connectionName);

    QList<Dish> loadActiveDishes() const;
    QList<Dish> loadAllDishes() const;
    int activeDishCount() const;
    bool addDish(const Dish &dish, QString *errorMessage = nullptr) const;
    bool updateDish(const Dish &dish, QString *errorMessage = nullptr) const;
    bool archiveDish(int dishId, QString *errorMessage = nullptr) const;

private:
    QString m_connectionName;
};
