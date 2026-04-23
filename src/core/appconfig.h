#pragma once

#include <QObject>
#include <QString>

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString llmApiKey READ llmApiKey NOTIFY llmSettingsChanged)
    Q_PROPERTY(QString llmApiUrl READ llmApiUrl NOTIFY llmSettingsChanged)
    Q_PROPERTY(QString llmModel READ llmModel NOTIFY llmSettingsChanged)
    Q_PROPERTY(QString effectiveLlmApiUrl READ effectiveLlmApiUrl NOTIFY llmSettingsChanged)
    Q_PROPERTY(QString effectiveLlmModel READ effectiveLlmModel NOTIFY llmSettingsChanged)
    Q_PROPERTY(bool llmApiConfigured READ llmApiConfigured NOTIFY llmSettingsChanged)
    Q_PROPERTY(QString llmConfigSummary READ llmConfigSummary NOTIFY llmSettingsChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    QString llmApiKey() const;
    QString llmApiUrl() const;
    QString llmModel() const;
    QString effectiveLlmApiUrl() const;
    QString effectiveLlmModel() const;
    bool llmApiConfigured() const;
    QString llmConfigSummary() const;

    Q_INVOKABLE void saveLlmSettings(const QString &apiKey,
                                     const QString &apiUrl,
                                     const QString &model);
    Q_INVOKABLE void clearLlmSettings();

signals:
    void llmSettingsChanged();
};

namespace AppConfig
{
QString appDataDirectory();
QString databaseFilePath();
QString storedLlmApiUrl();
QString storedLlmApiKey();
QString storedLlmModel();
QString llmApiUrl();
QString llmApiKey();
QString llmModel();
bool llmApiConfigured();
}
