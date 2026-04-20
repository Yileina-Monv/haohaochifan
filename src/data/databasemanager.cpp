#include "databasemanager.h"

#include "../core/appconfig.h"

#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace
{
QString connectionName()
{
    return QStringLiteral("mealadvisor_connection");
}

bool tableHasColumn(QSqlDatabase database, const QString &tableName,
                    const QString &columnName)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral("PRAGMA table_info(%1)").arg(tableName));
    if (!query.exec()) {
        return false;
    }

    while (query.next()) {
        if (query.value(1).toString() == columnName) {
            return true;
        }
    }

    return false;
}

bool ensureColumn(QSqlDatabase database, const QString &tableName,
                  const QString &columnDefinition, QString *errorMessage)
{
    const QString columnName = columnDefinition.section(' ', 0, 0);
    if (tableHasColumn(database, tableName, columnName)) {
        return true;
    }

    QSqlQuery query(database);
    const QString statement =
        QStringLiteral("ALTER TABLE %1 ADD COLUMN %2").arg(tableName,
                                                           columnDefinition);
    if (!query.exec(statement)) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

QStringList classPeriodSeedStatements()
{
    return {
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (1, '08:30', '09:10', 'morning')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (2, '09:15', '09:55', 'morning')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (3, '10:15', '10:55', 'morning')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (4, '11:00', '11:40', 'morning')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (5, '11:45', '12:25', 'morning')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (6, '13:30', '14:10', 'afternoon')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (7, '14:15', '14:55', 'afternoon')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (8, '15:00', '15:40', 'afternoon')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (9, '16:00', '16:40', 'afternoon')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (10, '16:45', '17:25', 'afternoon')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (11, '19:00', '19:40', 'evening')"),
        QStringLiteral("INSERT OR IGNORE INTO class_periods (period_index, start_time, end_time, session_group) VALUES (12, '19:45', '20:20', 'evening')")
    };
}

QStringList planningPolicySeedStatements()
{
    return {
        QStringLiteral(
            "INSERT OR IGNORE INTO planning_policies "
            "(id, name, enabled_weekdays, include_commute_days, skip_non_enabled_days, default_daily_budget, flexible_budget_cap, breakfast_recommendation_enabled, created_at, updated_at) "
            "VALUES (1, 'Default Campus Plan', '2,3,4,5', 1, 1, 80, 120, 0, datetime('now'), datetime('now'))")
    };
}

QStringList recommendationProfileSeedStatements()
{
    return {
        QStringLiteral(
            "INSERT OR IGNORE INTO recommendation_profiles "
            "(id, name, has_class_priority, health_priority, budget_priority, time_effort_priority, satiety_priority, carb_control_priority, fat_control_priority, protein_priority, sleepiness_avoid_priority, breakfast_recommendation_enabled, enabled_weekdays, planning_scope, is_default, created_at, updated_at) "
            "VALUES (1, 'Class Day', 1, 7, 4, 9, 6, 9, 7, 6, 10, 0, '2,3,4,5', 'class_day', 1, datetime('now'), datetime('now'))"),
        QStringLiteral(
            "INSERT OR IGNORE INTO recommendation_profiles "
            "(id, name, has_class_priority, health_priority, budget_priority, time_effort_priority, satiety_priority, carb_control_priority, fat_control_priority, protein_priority, sleepiness_avoid_priority, breakfast_recommendation_enabled, enabled_weekdays, planning_scope, is_default, created_at, updated_at) "
            "VALUES (2, 'Commute Day', 1, 8, 4, 8, 7, 8, 7, 6, 9, 0, '2,5', 'commute_day', 0, datetime('now'), datetime('now'))")
    };
}
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent),
      m_connectionName(connectionName()),
      m_databasePath(AppConfig::databaseFilePath())
{
}

bool DatabaseManager::initialize()
{
    if (!openConnection()) {
        return false;
    }

    if (!applySchema()) {
        return false;
    }

    if (!applyMigrations()) {
        return false;
    }

    if (!seedClassPeriods()) {
        return false;
    }

    if (!seedPlanningPolicies()) {
        return false;
    }

    return seedRecommendationProfiles();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

QString DatabaseManager::databasePath() const
{
    return m_databasePath;
}

QString DatabaseManager::connectionName() const
{
    return m_connectionName;
}

bool DatabaseManager::openConnection()
{
    QSqlDatabase database;
    if (QSqlDatabase::contains(m_connectionName)) {
        database = QSqlDatabase::database(m_connectionName);
    } else {
        database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
        database.setDatabaseName(m_databasePath);
    }

    if (!database.open()) {
        m_lastError = database.lastError().text();
        return false;
    }

    QSqlQuery pragmaQuery(database);
    pragmaQuery.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    return true;
}

bool DatabaseManager::applySchema()
{
    QFile schemaFile(QStringLiteral(":/data/schema.sql"));
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("Failed to open bundled schema.sql resource.");
        return false;
    }

    const QString schemaText = QString::fromUtf8(schemaFile.readAll());
    const QStringList statements = schemaText.split(';', Qt::SkipEmptyParts);
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    for (QString statement : statements) {
        statement = statement.trimmed();
        if (statement.isEmpty()) {
            continue;
        }

        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::applyMigrations()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);

    if (!ensureColumn(database, QStringLiteral("merchants"),
                      QStringLiteral("delivery_eta_minutes INTEGER"),
                      &m_lastError)) {
        return false;
    }

    const QStringList dishColumns = {
        QStringLiteral("vitamin_level TEXT"),
        QStringLiteral("flavor_level TEXT"),
        QStringLiteral("odor_level TEXT"),
        QStringLiteral("meal_impact_weight REAL NOT NULL DEFAULT 1.0"),
        QStringLiteral("notes TEXT")
    };

    for (const QString &columnDefinition : dishColumns) {
        if (!ensureColumn(database, QStringLiteral("dishes"), columnDefinition,
                          &m_lastError)) {
            return false;
        }
    }

    const QStringList mealFeedbackColumns = {
        QStringLiteral("recommendation_record_id INTEGER"),
        QStringLiteral("taste_rating INTEGER"),
        QStringLiteral("repeat_willingness INTEGER")
    };

    for (const QString &columnDefinition : mealFeedbackColumns) {
        if (!ensureColumn(database, QStringLiteral("meal_feedback"),
                          columnDefinition, &m_lastError)) {
            return false;
        }
    }

    const QStringList recommendationRecordColumns = {
        QStringLiteral("selected_meal_log_id INTEGER"),
        QStringLiteral("selected_candidate_rank INTEGER")
    };

    for (const QString &columnDefinition : recommendationRecordColumns) {
        if (!ensureColumn(database, QStringLiteral("recommendation_records"),
                          columnDefinition, &m_lastError)) {
            return false;
        }
    }

    return true;
}

bool DatabaseManager::seedClassPeriods()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    for (const QString &statement : classPeriodSeedStatements()) {
        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::seedPlanningPolicies()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    for (const QString &statement : planningPolicySeedStatements()) {
        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::seedRecommendationProfiles()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    for (const QString &statement : recommendationProfileSeedStatements()) {
        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}
