#include "schedulemanager.h"

#include "domainmodels.h"

#include "../data/schedulerepository.h"

#include <QHash>
#include <QSet>
#include <QVariantMap>

namespace
{
QList<ScheduleEntry> providedScheduleEntries()
{
    return {
        {0, 3, 3, 4, QStringLiteral("Imported Physics A"), QStringLiteral("Building A-110"), QStringLiteral("North Campus"), QStringLiteral("medium"), QString()},
        {0, 3, 7, 8, QStringLiteral("Imported Analysis"), QStringLiteral("Building B-208"), QStringLiteral("North Campus"), QStringLiteral("medium"), QString()},
        {0, 3, 9, 10, QStringLiteral("Imported Statistics"), QStringLiteral("Building C-107"), QStringLiteral("North Campus"), QStringLiteral("medium"), QString()},
        {0, 3, 11, 12, QStringLiteral("Imported Ideology"), QStringLiteral("Building TBD"), QStringLiteral("North Campus"), QStringLiteral("low"), QStringLiteral("Imported from the original screenshot. Replace with the confirmed classroom when available.")},
        {0, 4, 3, 5, QStringLiteral("Imported C Programming Lab"), QStringLiteral("Building D-207"), QStringLiteral("South Campus"), QStringLiteral("high"), QString()},
        {0, 4, 9, 10, QStringLiteral("Imported C Programming Lab"), QStringLiteral("Building D-207"), QStringLiteral("South Campus"), QStringLiteral("high"), QString()},
        {0, 5, 1, 2, QStringLiteral("Imported Physics A"), QStringLiteral("Building A-110"), QStringLiteral("North Campus"), QStringLiteral("medium"), QString()},
        {0, 5, 7, 9, QStringLiteral("Imported Signals"), QStringLiteral("Building B-417"), QStringLiteral("North Campus"), QStringLiteral("high"), QString()}
    };
}

QString sessionLabel(const QString &sessionGroup)
{
    if (sessionGroup == QStringLiteral("morning")) {
        return QStringLiteral("Morning");
    }
    if (sessionGroup == QStringLiteral("afternoon")) {
        return QStringLiteral("Afternoon");
    }
    if (sessionGroup == QStringLiteral("evening")) {
        return QStringLiteral("Evening");
    }
    return sessionGroup;
}
}

ScheduleManager::ScheduleManager(const DatabaseManager &databaseManager, QObject *parent)
    : QObject(parent),
      m_databaseManager(databaseManager)
{
    refreshState();

    if (m_totalEntryCount == 0) {
        resetToProvidedSchedule();
    }
}

QVariantList ScheduleManager::classPeriods() const
{
    return m_classPeriods;
}

QVariantList ScheduleManager::weekSchedule() const
{
    return m_weekSchedule;
}

int ScheduleManager::totalEntryCount() const
{
    return m_totalEntryCount;
}

QString ScheduleManager::lastError() const
{
    return m_lastError;
}

void ScheduleManager::reload()
{
    refreshState();
    emit scheduleChanged();
}

bool ScheduleManager::resetToProvidedSchedule()
{
    ScheduleRepository repository(m_databaseManager.connectionName());
    QString errorMessage;

    if (!repository.clearAllEntries(&errorMessage)) {
        m_lastError = errorMessage;
        emit scheduleChanged();
        return false;
    }

    for (const ScheduleEntry &entry : providedScheduleEntries()) {
        if (!repository.addEntry(entry, &errorMessage)) {
            m_lastError = errorMessage;
            emit scheduleChanged();
            return false;
        }
    }

    m_lastError.clear();
    reload();
    return true;
}

bool ScheduleManager::addCustomEntry(int weekday,
                                     int periodStart,
                                     int periodEnd,
                                     const QString &courseName,
                                     const QString &location,
                                     const QString &campusZone,
                                     const QString &intensityLevel,
                                     const QString &notes)
{
    return saveEntry(0,
                     weekday,
                     periodStart,
                     periodEnd,
                     courseName,
                     location,
                     campusZone,
                     intensityLevel,
                     notes,
                     false);
}

bool ScheduleManager::updateEntry(int entryId,
                                  int weekday,
                                  int periodStart,
                                  int periodEnd,
                                  const QString &courseName,
                                  const QString &location,
                                  const QString &campusZone,
                                  const QString &intensityLevel,
                                  const QString &notes)
{
    return saveEntry(entryId,
                     weekday,
                     periodStart,
                     periodEnd,
                     courseName,
                     location,
                     campusZone,
                     intensityLevel,
                     notes,
                     true);
}

bool ScheduleManager::deleteEntry(int entryId)
{
    if (entryId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid schedule entry.");
        emit scheduleChanged();
        return false;
    }

    ScheduleRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.deleteEntry(entryId, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to delete schedule entry.")
                          : errorMessage;
        emit scheduleChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

void ScheduleManager::refreshState()
{
    ScheduleRepository repository(m_databaseManager.connectionName());
    const QList<ClassPeriod> periods = repository.loadClassPeriods();
    const QList<ScheduleEntry> entries = repository.loadAllEntries();

    m_classPeriods.clear();
    QHash<int, ClassPeriod> periodIndexMap;
    for (const ClassPeriod &period : periods) {
        periodIndexMap.insert(period.periodIndex, period);

        QVariantMap periodMap;
        periodMap.insert(QStringLiteral("periodIndex"), period.periodIndex);
        periodMap.insert(QStringLiteral("timeRange"),
                         QStringLiteral("%1-%2").arg(period.startTime, period.endTime));
        periodMap.insert(QStringLiteral("sessionLabel"), sessionLabel(period.sessionGroup));
        m_classPeriods.append(periodMap);
    }

    m_weekSchedule.clear();
    m_totalEntryCount = entries.size();
    for (const int weekday : managedWeekdays()) {
        QVariantList dayEntries;

        for (const ScheduleEntry &entry : entries) {
            if (entry.weekday != weekday) {
                continue;
            }

            const ClassPeriod startPeriod = periodIndexMap.value(entry.periodStart);
            const ClassPeriod endPeriod = periodIndexMap.value(entry.periodEnd);

            QVariantMap entryMap;
            entryMap.insert(QStringLiteral("id"), entry.id);
            entryMap.insert(QStringLiteral("weekday"), entry.weekday);
            entryMap.insert(QStringLiteral("courseName"), entry.courseName);
            entryMap.insert(QStringLiteral("periodStart"), entry.periodStart);
            entryMap.insert(QStringLiteral("periodEnd"), entry.periodEnd);
            entryMap.insert(QStringLiteral("periodRange"),
                            QStringLiteral("Period %1-%2").arg(entry.periodStart).arg(entry.periodEnd));
            entryMap.insert(QStringLiteral("timeRange"),
                            QStringLiteral("%1-%2").arg(startPeriod.startTime, endPeriod.endTime));
            entryMap.insert(QStringLiteral("location"), entry.location);
            entryMap.insert(QStringLiteral("campusZone"), entry.campusZone);
            entryMap.insert(QStringLiteral("intensityLevel"), entry.intensityLevel);
            entryMap.insert(QStringLiteral("notes"), entry.notes);
            dayEntries.append(entryMap);
        }

        QVariantMap dayMap;
        dayMap.insert(QStringLiteral("weekday"), weekday);
        dayMap.insert(QStringLiteral("label"), weekdayLabel(weekday));
        dayMap.insert(QStringLiteral("entryCount"), dayEntries.size());
        dayMap.insert(QStringLiteral("entries"), dayEntries);
        m_weekSchedule.append(dayMap);
    }
}

bool ScheduleManager::saveEntry(int entryId,
                                int weekday,
                                int periodStart,
                                int periodEnd,
                                const QString &courseName,
                                const QString &location,
                                const QString &campusZone,
                                const QString &intensityLevel,
                                const QString &notes,
                                bool isUpdate)
{
    const QString trimmedCourseName = courseName.trimmed();
    if (trimmedCourseName.isEmpty()) {
        m_lastError = QStringLiteral("Course name is required.");
        emit scheduleChanged();
        return false;
    }

    if (!managedWeekdays().contains(weekday)) {
        m_lastError = QStringLiteral("Weekday must stay inside the managed planning range.");
        emit scheduleChanged();
        return false;
    }

    if (periodStart <= 0 || periodEnd < periodStart) {
        m_lastError = QStringLiteral("Period range is invalid.");
        emit scheduleChanged();
        return false;
    }

    const QSet<int> validPeriodIndices = [this]() {
        QSet<int> indices;
        for (const QVariant &periodVariant : m_classPeriods) {
            indices.insert(periodVariant.toMap().value(QStringLiteral("periodIndex")).toInt());
        }
        return indices;
    }();

    if (!validPeriodIndices.contains(periodStart) || !validPeriodIndices.contains(periodEnd)) {
        m_lastError = QStringLiteral("Selected periods are out of range.");
        emit scheduleChanged();
        return false;
    }

    if (isUpdate && entryId <= 0) {
        m_lastError = QStringLiteral("Please choose a valid schedule entry.");
        emit scheduleChanged();
        return false;
    }

    ScheduleEntry entry;
    entry.id = entryId;
    entry.weekday = weekday;
    entry.periodStart = periodStart;
    entry.periodEnd = periodEnd;
    entry.courseName = trimmedCourseName;
    entry.location = location.trimmed();
    entry.campusZone = campusZone.trimmed();
    entry.intensityLevel = intensityLevel.trimmed();
    entry.notes = notes.trimmed();

    ScheduleRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    const bool ok = isUpdate
                        ? repository.updateEntry(entry, &errorMessage)
                        : repository.addEntry(entry, &errorMessage);
    if (!ok) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("Failed to save schedule entry.")
                          : errorMessage;
        emit scheduleChanged();
        return false;
    }

    m_lastError.clear();
    reload();
    return true;
}

QString ScheduleManager::weekdayLabel(int weekday)
{
    switch (weekday) {
    case 2:
        return QStringLiteral("Tuesday");
    case 3:
        return QStringLiteral("Wednesday");
    case 4:
        return QStringLiteral("Thursday");
    case 5:
        return QStringLiteral("Friday");
    default:
        return QStringLiteral("Weekday %1").arg(weekday);
    }
}

QList<int> ScheduleManager::managedWeekdays()
{
    return {2, 3, 4, 5};
}
