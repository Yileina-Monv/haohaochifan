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
        {0, 3, 3, 4, QStringLiteral("导入课程：大学物理 A"), QStringLiteral("A-110 教室"), QStringLiteral("北校区"), QStringLiteral("medium"), QString()},
        {0, 3, 7, 8, QStringLiteral("导入课程：数学分析"), QStringLiteral("B-208 教室"), QStringLiteral("北校区"), QStringLiteral("medium"), QString()},
        {0, 3, 9, 10, QStringLiteral("导入课程：统计学"), QStringLiteral("C-107 教室"), QStringLiteral("北校区"), QStringLiteral("medium"), QString()},
        {0, 3, 11, 12, QStringLiteral("导入课程：思政课"), QStringLiteral("待确认教室"), QStringLiteral("北校区"), QStringLiteral("low"), QStringLiteral("来自原始截图，教室信息后续确认后可替换。")},
        {0, 4, 3, 5, QStringLiteral("导入课程：C 程序设计实验"), QStringLiteral("D-207 教室"), QStringLiteral("南校区"), QStringLiteral("high"), QString()},
        {0, 4, 9, 10, QStringLiteral("导入课程：C 程序设计实验"), QStringLiteral("D-207 教室"), QStringLiteral("南校区"), QStringLiteral("high"), QString()},
        {0, 5, 1, 2, QStringLiteral("导入课程：大学物理 A"), QStringLiteral("A-110 教室"), QStringLiteral("北校区"), QStringLiteral("medium"), QString()},
        {0, 5, 7, 9, QStringLiteral("导入课程：信号与系统"), QStringLiteral("B-417 教室"), QStringLiteral("北校区"), QStringLiteral("high"), QString()}
    };
}

QString sessionLabel(const QString &sessionGroup)
{
    if (sessionGroup == QStringLiteral("morning")) {
        return QStringLiteral("上午");
    }
    if (sessionGroup == QStringLiteral("afternoon")) {
        return QStringLiteral("下午");
    }
    if (sessionGroup == QStringLiteral("evening")) {
        return QStringLiteral("晚上");
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
        m_lastError = QStringLiteral("请选择有效的课表条目。");
        emit scheduleChanged();
        return false;
    }

    ScheduleRepository repository(m_databaseManager.connectionName());
    QString errorMessage;
    if (!repository.deleteEntry(entryId, &errorMessage)) {
        m_lastError = errorMessage.isEmpty()
                          ? QStringLiteral("删除课表条目失败。")
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
                            QStringLiteral("第 %1-%2 节").arg(entry.periodStart).arg(entry.periodEnd));
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
        m_lastError = QStringLiteral("课程名不能为空。");
        emit scheduleChanged();
        return false;
    }

    if (!managedWeekdays().contains(weekday)) {
        m_lastError = QStringLiteral("星期必须在当前规划范围内。");
        emit scheduleChanged();
        return false;
    }

    if (periodStart <= 0 || periodEnd < periodStart) {
        m_lastError = QStringLiteral("节次范围无效。");
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
        m_lastError = QStringLiteral("选择的节次超出范围。");
        emit scheduleChanged();
        return false;
    }

    if (isUpdate && entryId <= 0) {
        m_lastError = QStringLiteral("请选择有效的课表条目。");
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
                          ? QStringLiteral("保存课表条目失败。")
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
        return QStringLiteral("周二");
    case 3:
        return QStringLiteral("周三");
    case 4:
        return QStringLiteral("周四");
    case 5:
        return QStringLiteral("周五");
    default:
        return QStringLiteral("周 %1").arg(weekday);
    }
}

QList<int> ScheduleManager::managedWeekdays()
{
    return {2, 3, 4, 5};
}
