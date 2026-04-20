#include "appconfig.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace AppConfig
{
QString appDataDirectory()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basePath.isEmpty()) {
        basePath = QDir::homePath() + "/MealAdvisor";
    }

    QDir dir(basePath);
    dir.mkpath(".");
    return dir.absolutePath();
}

QString databaseFilePath()
{
    return QDir(appDataDirectory()).filePath("mealadvisor.sqlite");
}

QString llmApiUrl()
{
    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString configuredUrl =
        environment.value(QStringLiteral("MEALADVISOR_LLM_API_URL")).trimmed();
    if (!configuredUrl.isEmpty()) {
        return configuredUrl;
    }

    return QStringLiteral("https://api.openai.com/v1/chat/completions");
}

QString llmApiKey()
{
    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString explicitKey =
        environment.value(QStringLiteral("MEALADVISOR_LLM_API_KEY")).trimmed();
    if (!explicitKey.isEmpty()) {
        return explicitKey;
    }

    return environment.value(QStringLiteral("OPENAI_API_KEY")).trimmed();
}

QString llmModel()
{
    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString configuredModel =
        environment.value(QStringLiteral("MEALADVISOR_LLM_MODEL")).trimmed();
    if (!configuredModel.isEmpty()) {
        return configuredModel;
    }

    return QStringLiteral("gpt-4o-mini");
}

bool llmApiConfigured()
{
    return !llmApiUrl().trimmed().isEmpty() && !llmApiKey().trimmed().isEmpty();
}
}
