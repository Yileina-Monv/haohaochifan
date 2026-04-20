#pragma once

#include "../core/domainmodels.h"

#include <QString>

class ScheduleRepository
{
public:
    explicit ScheduleRepository(QString connectionName);

    QList<ClassPeriod> loadClassPeriods() const;
    QList<ScheduleEntry> loadEntriesForWeekday(int weekday) const;
    QList<ScheduleEntry> loadAllEntries() const;
    bool addEntry(const ScheduleEntry &entry, QString *errorMessage = nullptr) const;
    bool updateEntry(const ScheduleEntry &entry, QString *errorMessage = nullptr) const;
    bool deleteEntry(int entryId, QString *errorMessage = nullptr) const;
    bool clearAllEntries(QString *errorMessage = nullptr) const;

private:
    QString m_connectionName;
};
