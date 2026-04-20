#pragma once

#include "../data/databasemanager.h"

#include <QObject>
#include <QVariantList>

class ScheduleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList classPeriods READ classPeriods NOTIFY scheduleChanged)
    Q_PROPERTY(QVariantList weekSchedule READ weekSchedule NOTIFY scheduleChanged)
    Q_PROPERTY(int totalEntryCount READ totalEntryCount NOTIFY scheduleChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY scheduleChanged)

public:
    explicit ScheduleManager(const DatabaseManager &databaseManager,
                             QObject *parent = nullptr);

    QVariantList classPeriods() const;
    QVariantList weekSchedule() const;
    int totalEntryCount() const;
    QString lastError() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool resetToProvidedSchedule();
    Q_INVOKABLE bool addCustomEntry(int weekday,
                                    int periodStart,
                                    int periodEnd,
                                    const QString &courseName,
                                    const QString &location,
                                    const QString &campusZone,
                                    const QString &intensityLevel,
                                    const QString &notes);
    Q_INVOKABLE bool updateEntry(int entryId,
                                 int weekday,
                                 int periodStart,
                                 int periodEnd,
                                 const QString &courseName,
                                 const QString &location,
                                 const QString &campusZone,
                                 const QString &intensityLevel,
                                 const QString &notes);
    Q_INVOKABLE bool deleteEntry(int entryId);

signals:
    void scheduleChanged();

private:
    void refreshState();
    bool saveEntry(int entryId,
                   int weekday,
                   int periodStart,
                   int periodEnd,
                   const QString &courseName,
                   const QString &location,
                   const QString &campusZone,
                   const QString &intensityLevel,
                   const QString &notes,
                   bool isUpdate);
    static QString weekdayLabel(int weekday);
    static QList<int> managedWeekdays();

    const DatabaseManager &m_databaseManager;
    QVariantList m_classPeriods;
    QVariantList m_weekSchedule;
    int m_totalEntryCount = 0;
    QString m_lastError;
};
