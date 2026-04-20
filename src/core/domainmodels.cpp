#include "domainmodels.h"

#include <QStringList>

QList<int> parseWeekdayList(const QString &serializedWeekdays)
{
    QList<int> weekdays;
    const QStringList parts = serializedWeekdays.split(',', Qt::SkipEmptyParts);

    for (const QString &part : parts) {
        bool ok = false;
        const int value = part.trimmed().toInt(&ok);
        if (ok) {
            weekdays.append(value);
        }
    }

    return weekdays;
}

QString serializeWeekdayList(const QList<int> &weekdays)
{
    QStringList parts;
    for (const int weekday : weekdays) {
        parts.append(QString::number(weekday));
    }

    return parts.join(',');
}
