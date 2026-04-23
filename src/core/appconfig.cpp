#include "appconfig.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>

namespace
{
QString trimmedSettingValue(QSettings &settings, const QString &key)
{
    return settings.value(key).toString().trimmed();
}

QString envValue(const QString &key)
{
    return QProcessEnvironment::systemEnvironment().value(key).trimmed();
}
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
}

QString AppSettings::llmApiKey() const
{
    return AppConfig::storedLlmApiKey();
}

QString AppSettings::llmApiUrl() const
{
    return AppConfig::storedLlmApiUrl();
}

QString AppSettings::llmModel() const
{
    return AppConfig::storedLlmModel();
}

QString AppSettings::effectiveLlmApiUrl() const
{
    return AppConfig::llmApiUrl();
}

QString AppSettings::effectiveLlmModel() const
{
    return AppConfig::llmModel();
}

bool AppSettings::llmApiConfigured() const
{
    return AppConfig::llmApiConfigured();
}

QString AppSettings::llmConfigSummary() const
{
    const bool hasStoredKey = !AppConfig::storedLlmApiKey().isEmpty();
    const bool hasStoredUrl = !AppConfig::storedLlmApiUrl().isEmpty();
    const bool hasStoredModel = !AppConfig::storedLlmModel().isEmpty();

    if (hasStoredKey || hasStoredUrl || hasStoredModel) {
        return QStringLiteral("当前优先使用应用内保存的 LLM 配置。空白项会继续回退到环境变量。");
    }

    if (AppConfig::llmApiConfigured()) {
        return QStringLiteral("当前未保存应用内配置，正在使用环境变量或默认接口参数。");
    }

    return QStringLiteral("当前还没有可用的 LLM API Key。");
}

void AppSettings::saveLlmSettings(const QString &apiKey,
                                  const QString &apiUrl,
                                  const QString &model)
{
    QSettings settings;
    settings.setValue(QStringLiteral("llm/api_key"), apiKey.trimmed());
    settings.setValue(QStringLiteral("llm/api_url"), apiUrl.trimmed());
    settings.setValue(QStringLiteral("llm/model"), model.trimmed());
    settings.sync();
    emit llmSettingsChanged();
}

void AppSettings::clearLlmSettings()
{
    QSettings settings;
    settings.remove(QStringLiteral("llm"));
    settings.sync();
    emit llmSettingsChanged();
}

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

QString storedLlmApiUrl()
{
    QSettings settings;
    return trimmedSettingValue(settings, QStringLiteral("llm/api_url"));
}

QString storedLlmApiKey()
{
    QSettings settings;
    return trimmedSettingValue(settings, QStringLiteral("llm/api_key"));
}

QString storedLlmModel()
{
    QSettings settings;
    return trimmedSettingValue(settings, QStringLiteral("llm/model"));
}

QString llmApiUrl()
{
    const QString savedUrl = storedLlmApiUrl();
    if (!savedUrl.isEmpty()) {
        return savedUrl;
    }

    const QString configuredUrl = envValue(QStringLiteral("MEALADVISOR_LLM_API_URL"));
    if (!configuredUrl.isEmpty()) {
        return configuredUrl;
    }

    return QStringLiteral("https://api.openai.com/v1/chat/completions");
}

QString llmApiKey()
{
    const QString savedKey = storedLlmApiKey();
    if (!savedKey.isEmpty()) {
        return savedKey;
    }

    const QString explicitKey = envValue(QStringLiteral("MEALADVISOR_LLM_API_KEY"));
    if (!explicitKey.isEmpty()) {
        return explicitKey;
    }

    return envValue(QStringLiteral("OPENAI_API_KEY"));
}

QString llmModel()
{
    const QString savedModel = storedLlmModel();
    if (!savedModel.isEmpty()) {
        return savedModel;
    }

    const QString configuredModel = envValue(QStringLiteral("MEALADVISOR_LLM_MODEL"));
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
