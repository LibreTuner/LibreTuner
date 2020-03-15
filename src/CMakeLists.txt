cmake_minimum_required(VERSION 3.1)
project(Overboost)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Find includes in corresponding build directories
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(libretuner_SRC
        main.cpp

        OverboostApp.cpp
        OverboostApp.h

        MainWindow.cpp
        MainWindow.h
        MainWindow.ui

        dialogs/QuickStartDialog.cpp
        dialogs/QuickStartDialog.h
        dialogs/QuickStartDialog.ui
        dialogs/ImportCalibrationDialog.cpp
        dialogs/ImportCalibrationDialog.h
        dialogs/ImportCalibrationDialog.ui

        models/PlatformsModel.cpp
        models/PlatformsModel.h
        models/CategorizedTablesModel.cpp
        models/CategorizedTablesModel.h
        models/TableModel.cpp
        models/TableModel.h

        widgets/TableView.cpp
        widgets/TableView.h
        widgets/VerticalLabel.cpp
        widgets/VerticalLabel.h
        widgets/GraphWidget.cpp
        widgets/GraphWidget.h

        ${QHEXVIEW_SOURCES} ${QHEXVIEW_HEADERS})

if (UNIX AND NOT APPLE)

endif ()

if (WIN32)

endif ()


# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
#qt5_wrap_ui(libretuner_SRC
#    ui/canviewer.ui
#        ui/windows/createtunedialog.ui
#
#    framelesswindow/framelesswindow.ui
#)

qt5_add_resources(libretuner_SRC
        resources/icons.qrc
        resources/definitions.qrc
        resources/codes.qrc
        resources/qdarkstyle/style.qrc
        )


add_executable(Overboost ${libretuner_SRC})

target_link_libraries(Overboost OverboostCore Qt5::Widgets Qt5::DataVisualization Qt5::Charts Qt5::PrintSupport ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(Overboost PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../lib/QHexView)


if (UNIX AND NOT APPLE)
    target_compile_definitions(Overboost PRIVATE WITH_SOCKETCAN=1)
    target_compile_definitions(OverboostCore PRIVATE WITH_SOCKETCAN=1)
    target_link_libraries(Overboost stdc++fs)
endif ()

if (WIN32)
    target_compile_definitions(Overboost PRIVATE WITH_J2534=1)
    target_compile_definitions(OverboostCore PRIVATE WITH_J2534=1)
endif (WIN32)


# Set warnings
if (MSVC)
    target_compile_options(Overboost PRIVATE /W4)
else ()
    target_compile_options(Overbost PRIVATE -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-missing-braces)
endif ()


if (MINGW)
    target_compile_options(Overboost PRIVATE -mwindows)
endif ()
