#include "schedulerepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

ScheduleRepository::ScheduleRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

QList<ClassPeriod> ScheduleRepository::loadClassPeriods() const
{
    QList<ClassPeriod> periods;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, period_index, start_time, end_time, session_group "
        "FROM class_periods ORDER BY period_index"));

    if (!query.exec()) {
        return periods;
    }

    while (query.next()) {
        ClassPeriod period;
        period.id = query.value(0).toInt();
        period.periodIndex = query.value(1).toInt();
        period.startTime = query.value(2).toString();
        period.endTime = query.value(3).toString();
        period.sessionGroup = query.value(4).toString();
        periods.append(period);
    }

    return periods;
}

QList<ScheduleEntry> ScheduleRepository::loadEntriesForWeekday(int weekday) const
{
    QList<ScheduleEntry> entries;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, weekday, period_start, period_end, course_name, location, "
        "campus_zone, intensity_level, notes "
        "FROM schedule_entries WHERE weekday = ? ORDER BY period_start"));
    query.addBindValue(weekday);

    if (!query.exec()) {
        return entries;
    }

    while (query.next()) {
        ScheduleEntry entry;
        entry.id = query.value(0).toInt();
        entry.weekday = query.value(1).toInt();
        entry.periodStart = query.value(2).toInt();
        entry.periodEnd = query.value(3).toInt();
        entry.courseName = query.value(4).toString();
        entry.location = query.value(5).toString();
        entry.campusZone = query.value(6).toString();
        entry.intensityLevel = query.value(7).toString();
        entry.notes = query.value(8).toString();
        entries.append(entry);
    }

    return entries;
}

QList<ScheduleEntry> ScheduleRepository::loadAllEntries() const
{
    QList<ScheduleEntry> entries;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, weekday, period_start, period_end, course_name, location, "
        "campus_zone, intensity_level, notes "
        "FROM schedule_entries ORDER BY weekday, period_start"));

    if (!query.exec()) {
        return entries;
    }

    while (query.next()) {
        ScheduleEntry entry;
        entry.id = query.value(0).toInt();
        entry.weekday = query.value(1).toInt();
        entry.periodStart = query.value(2).toInt();
        entry.periodEnd = query.value(3).toInt();
        entry.courseName = query.value(4).toString();
        entry.location = query.value(5).toString();
        entry.campusZone = query.value(6).toString();
        entry.intensityLevel = query.value(7).toString();
        entry.notes = query.value(8).toString();
        entries.append(entry);
    }

    return entries;
}

bool ScheduleRepository::addEntry(const ScheduleEntry &entry, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "INSERT INTO schedule_entries "
        "(weekday, period_start, period_end, course_name, location, campus_zone, "
        "intensity_level, notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(entry.weekday);
    query.addBindValue(entry.periodStart);
    query.addBindValue(entry.periodEnd);
    query.addBindValue(entry.courseName);
    query.addBindValue(entry.location);
    query.addBindValue(entry.campusZone);
    query.addBindValue(entry.intensityLevel);
    query.addBindValue(entry.notes);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

bool ScheduleRepository::updateEntry(const ScheduleEntry &entry, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "UPDATE schedule_entries SET "
        "weekday = ?, period_start = ?, period_end = ?, course_name = ?, "
        "location = ?, campus_zone = ?, intensity_level = ?, notes = ? "
        "WHERE id = ?"));
    query.addBindValue(entry.weekday);
    query.addBindValue(entry.periodStart);
    query.addBindValue(entry.periodEnd);
    query.addBindValue(entry.courseName);
    query.addBindValue(entry.location);
    query.addBindValue(entry.campusZone);
    query.addBindValue(entry.intensityLevel);
    query.addBindValue(entry.notes);
    query.addBindValue(entry.id);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ScheduleRepository::deleteEntry(int entryId, QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("DELETE FROM schedule_entries WHERE id = ?"));
    query.addBindValue(entryId);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ScheduleRepository::clearAllEntries(QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("DELETE FROM schedule_entries"));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}
