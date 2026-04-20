#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "core/appstate.h"
#include "core/foodmanager.h"
#include "core/meallogmanager.h"
#include "core/schedulemanager.h"
#include "data/databasemanager.h"
#include "recommendation/recommendationengine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    DatabaseManager databaseManager;

    if (!databaseManager.initialize()) {
        qFatal("Database initialization failed: %s",
               qPrintable(databaseManager.lastError()));
    }

    AppState appState(databaseManager);
    FoodManager foodManager(databaseManager);
    MealLogManager mealLogManager(databaseManager);
    ScheduleManager scheduleManager(databaseManager);
    RecommendationEngine recommendationEngine(databaseManager);

    engine.rootContext()->setContextProperty("appState", &appState);
    engine.rootContext()->setContextProperty("databaseManager", &databaseManager);
    engine.rootContext()->setContextProperty("foodManager", &foodManager);
    engine.rootContext()->setContextProperty("mealLogManager", &mealLogManager);
    engine.rootContext()->setContextProperty("scheduleManager", &scheduleManager);
    engine.rootContext()->setContextProperty("recommendationEngine",
                                             &recommendationEngine);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MealAdvisor", "Main");

    return app.exec();
}
