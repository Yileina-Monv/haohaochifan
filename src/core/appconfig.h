#pragma once

#include <QString>

namespace AppConfig
{
QString appDataDirectory();
QString databaseFilePath();
QString llmApiUrl();
QString llmApiKey();
QString llmModel();
bool llmApiConfigured();
}
