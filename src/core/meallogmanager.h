#pragma once

#include "../data/databasemanager.h"

#include <QObject>
#include <QVariantList>

class MealLogManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList availableDishes READ availableDishes NOTIFY stateChanged)
    Q_PROPERTY(QVariantList filteredAvailableDishes READ filteredAvailableDishes NOTIFY stateChanged)
    Q_PROPERTY(QVariantList frequentDishes READ frequentDishes NOTIFY stateChanged)
    Q_PROPERTY(QVariantList selectedDishes READ selectedDishes NOTIFY stateChanged)
    Q_PROPERTY(QVariantList recentMeals READ recentMeals NOTIFY stateChanged)
    Q_PROPERTY(QVariantList feedbackInsights READ feedbackInsights NOTIFY stateChanged)
    Q_PROPERTY(QString dishSearch READ dishSearch NOTIFY stateChanged)
    Q_PROPERTY(int editingMealLogId READ editingMealLogId NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)

public:
    explicit MealLogManager(const DatabaseManager &databaseManager,
                            QObject *parent = nullptr);

    QVariantList availableDishes() const;
    QVariantList filteredAvailableDishes() const;
    QVariantList frequentDishes() const;
    QVariantList selectedDishes() const;
    QVariantList recentMeals() const;
    QVariantList feedbackInsights() const;
    QString dishSearch() const;
    int editingMealLogId() const;
    QString lastError() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setDishSearch(const QString &searchText);
    Q_INVOKABLE bool addSelectedDish(int dishId,
                                     double portionRatio,
                                     const QString &customNotes);
    Q_INVOKABLE void removeSelectedDish(int index);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QVariantMap loadMealLogForEdit(int mealLogId);
    Q_INVOKABLE void cancelEditingMealLog();
    Q_INVOKABLE bool deleteMealLog(int mealLogId);
    Q_INVOKABLE QVariantMap loadMealFeedback(int mealLogId);
    Q_INVOKABLE bool saveMealFeedback(int mealLogId,
                                      int fullnessLevel,
                                      int sleepinessLevel,
                                      int comfortLevel,
                                      int focusImpactLevel,
                                      bool wouldEatAgain,
                                      int tasteRating,
                                      int repeatWillingness,
                                      const QString &freeTextFeedback);
    Q_INVOKABLE bool saveMealLog(const QString &mealType,
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
                                 const QString &notes);

signals:
    void stateChanged();

private:
    void refreshState();
    void refreshFilteredState();
    static bool matchesSearch(const QVariantMap &item, const QString &searchText,
                              const QStringList &keys);

    const DatabaseManager &m_databaseManager;
    QVariantList m_availableDishes;
    QVariantList m_filteredAvailableDishes;
    QVariantList m_frequentDishes;
    QVariantList m_selectedDishes;
    QVariantList m_recentMeals;
    QVariantList m_feedbackInsights;
    QString m_dishSearch;
    int m_editingMealLogId = 0;
    QString m_lastError;
};
