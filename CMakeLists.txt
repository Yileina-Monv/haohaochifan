cmake_minimum_required(VERSION 3.21)

project(MealAdvisor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Quick Qml Sql Network)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

qt_add_executable(MealAdvisor
    src/core/appconfig.cpp
    src/core/appconfig.h
    src/core/foodmanager.cpp
    src/core/foodmanager.h
    src/core/meallogmanager.cpp
    src/core/meallogmanager.h
    src/core/appstate.cpp
    src/core/appstate.h
    src/core/domainmodels.cpp
    src/core/domainmodels.h
    src/core/schedulemanager.cpp
    src/core/schedulemanager.h
    src/data/databasemanager.cpp
    src/data/databasemanager.h
    src/data/dishrepository.cpp
    src/data/dishrepository.h
    src/data/mealfeedbackrepository.cpp
    src/data/mealfeedbackrepository.h
    src/data/merchantrepository.cpp
    src/data/merchantrepository.h
    src/data/meallogrepository.cpp
    src/data/meallogrepository.h
    src/data/planningrepository.cpp
    src/data/planningrepository.h
    src/data/recommendationrecordrepository.cpp
    src/data/recommendationrecordrepository.h
    src/data/schedulerepository.cpp
    src/data/schedulerepository.h
    src/main.cpp
    src/recommendation/recommendationengine.cpp
    src/recommendation/recommendationengine.h
)

if (NOT ANDROID)
    qt_add_executable(MealAdvisorValidation
        src/core/appconfig.cpp
        src/core/appconfig.h
        src/core/foodmanager.cpp
        src/core/foodmanager.h
        src/core/meallogmanager.cpp
        src/core/meallogmanager.h
        src/core/domainmodels.cpp
        src/core/domainmodels.h
        src/core/schedulemanager.cpp
        src/core/schedulemanager.h
        src/data/databasemanager.cpp
        src/data/databasemanager.h
        src/data/dishrepository.cpp
        src/data/dishrepository.h
        src/data/mealfeedbackrepository.cpp
        src/data/mealfeedbackrepository.h
        src/data/merchantrepository.cpp
        src/data/merchantrepository.h
        src/data/meallogrepository.cpp
        src/data/meallogrepository.h
        src/data/planningrepository.cpp
        src/data/planningrepository.h
        src/data/recommendationrecordrepository.cpp
        src/data/recommendationrecordrepository.h
        src/data/schedulerepository.cpp
        src/data/schedulerepository.h
        src/recommendation/recommendationengine.cpp
        src/recommendation/recommendationengine.h
        tools/validation/main.cpp
    )
endif()

qt_add_qml_module(MealAdvisor
    URI MealAdvisor
    VERSION 1.0
    QML_FILES
        app/qml/FoodPage.qml
        app/qml/Main.qml
        app/qml/MealLogPage.qml
        app/qml/SchedulePage.qml
)

target_link_libraries(MealAdvisor
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Sql
        Qt6::Network
)

if (NOT ANDROID)
    target_link_libraries(MealAdvisorValidation
        PRIVATE
            Qt6::Core
            Qt6::Sql
            Qt6::Network
    )
endif()

if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    if (EXISTS "${WINDEPLOYQT_EXECUTABLE}")
        add_custom_command(TARGET MealAdvisor POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E env
                "PATH=${QT_BIN_DIR};$ENV{PATH}"
                "${WINDEPLOYQT_EXECUTABLE}"
                --dir "$<TARGET_FILE_DIR:MealAdvisor>"
                --qmldir "${CMAKE_SOURCE_DIR}/app/qml"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:MealAdvisor>"
            COMMENT "Deploying Qt runtime dependencies for MealAdvisor"
            VERBATIM
        )
    endif()
endif()

qt_add_resources(MealAdvisor "mealadvisor_resources"
    PREFIX "/"
    FILES
        data/schema.sql
)

if (NOT ANDROID)
    qt_add_resources(MealAdvisorValidation "mealadvisor_validation_resources"
        PREFIX "/"
        FILES
            data/schema.sql
    )
endif()

set_target_properties(MealAdvisor PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS MealAdvisor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if (NOT ANDROID)
    install(TARGETS MealAdvisorValidation
        BUNDLE DESTINATION .
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()
