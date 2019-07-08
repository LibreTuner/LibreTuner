cmake_minimum_required(VERSION 3.13)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(libretuner_SRC
    main.cpp
    
    libretuner.cpp
    libretuner.h
    timer.cpp
    timer.h
    logger.cpp
    logger.h
    timerrunloop.cpp
    timerrunloop.h
    asyncroutine.cpp
    asyncroutine.h
    log.cpp
    log.h
    dtcdescriptions.cpp
    dtcdescriptions.h
    serializeddata.cpp
    serializeddata.h
    uiutil.h
    
    plugin/plugin.cpp
    plugin/plugin.h
    plugin/pluginstate.cpp
    plugin/pluginstate.h

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/verticallabel.cpp
    ui/verticallabel.h
    ui/titlebar.cpp
    ui/titlebar.h
    ui/backgroundtask.cpp
    ui/backgroundtask.h
    ui/qcustomplot.cpp
    ui/qcustomplot.h
    ui/createtunedialog.cpp
    ui/createtunedialog.h
    ui/fileselectwidget.cpp
    ui/fileselectwidget.h
    ui/flasherwindow.cpp
    ui/flasherwindow.h
    ui/authoptionsview.cpp
    ui/authoptionsview.h
    ui/downloadwindow.cpp
    ui/downloadwindow.h
    ui/datalinkswidget.cpp
    ui/datalinkswidget.h
    ui/adddatalinkdialog.cpp
    ui/adddatalinkdialog.h
    ui/sessionscannerdialog.cpp
    ui/sessionscannerdialog.h
    ui/dataloggerwindow.cpp
    ui/dataloggerwindow.h

    ui/docks/overviewwidget.cpp
    ui/docks/overviewwidget.h
    ui/docks/sidebarwidget.cpp
    ui/docks/sidebarwidget.h
    ui/docks/tableswidget.cpp
    ui/docks/tableswidget.h
    ui/docks/logview.h
    ui/docks/logview.cpp
    ui/docks/editorwidget.cpp
    ui/docks/editorwidget.h
    ui/docks/diagnosticswidget.cpp
    ui/docks/diagnosticswidget.h
    ui/docks/graphwidget.cpp
    ui/docks/graphwidget.h
    ui/docks/hexeditwidget.cpp
    ui/docks/hexeditwidget.h
        ui/docks/explorerwidget.cpp
        ui/docks/explorerwidget.h

    ui/windows/definitionswindow.cpp
    ui/windows/definitionswindow.h
        ui/windows/newprojectdialog.cpp
        ui/windows/newprojectdialog.h
    
    ui/widget/datalogview.cpp
    ui/widget/datalogview.h
    ui/widget/datalogliveview.cpp
    ui/widget/datalogliveview.h
    ui/widget/customcombo.cpp
    ui/widget/customcombo.h
    ui/widget/datalinksettings.cpp
    ui/widget/datalinksettings.h

    models/tablemodel.cpp
    models/tablemodel.h
    models/dtcmodel.cpp
    models/dtcmodel.h
    models/serialportmodel.cpp
    models/serialportmodel.h

    database/definitions.cpp
    database/definitions.h
    database/links.cpp
    database/links.h
        database/projects.cpp
        database/projects.h


    ${QHEXVIEW_SOURCES} ${QHEXVIEW_HEADERS})

if (UNIX AND NOT APPLE)

endif ()

if (WIN32)

endif ()


# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(libretuner_SRC
    ui/canviewer.ui
    ui/createtunedialog.ui

    framelesswindow/framelesswindow.ui
)

qt5_add_resources(libretuner_SRC
    resources/icons.qrc
    resources/definitions.qrc
    resources/codes.qrc
    resources/qdarkstyle/style.qrc
)


add_executable(LibreTuner ${libretuner_SRC})

target_link_libraries(LibreTuner Qt5::Widgets Qt5::DataVisualization Qt5::Charts Qt5::PrintSupport cparse ${CMAKE_THREAD_LIBS_INIT} sol2 Lua LibLibreTuner)

target_include_directories(LibreTuner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../lib/QHexView)



if (UNIX AND NOT APPLE)
    target_compile_definitions(LibreTuner PRIVATE WITH_SOCKETCAN=1)
    target_compile_definitions(LibLibreTuner PRIVATE WITH_SOCKETCAN=1)
    target_link_libraries(LibreTuner stdc++fs)
endif ()

if (WIN32)
    target_compile_definitions(LibreTuner PRIVATE WITH_J2534=1)
    target_compile_definitions(LibLibreTuner PRIVATE WITH_J2534=1)
endif (WIN32)


# Set warnings
if(MSVC)
  target_compile_options(LibreTuner PRIVATE /W4)
else()
  target_compile_options(LibreTuner PRIVATE -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-missing-braces)
endif()


if (MINGW)
    target_compile_options(LibreTuner PRIVATE -mwindows)
endif()
