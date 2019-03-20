cmake_minimum_required(VERSION 3.13)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

add_definitions(-std=c++17)

set(libretuner_SRC
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

    util/signal.cpp
    util/signal.h

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/verticallabel.cpp
    ui/verticallabel.h
    ui/titlebar.cpp
    ui/titlebar.h
    ui/styledwindow.cpp
    ui/styledwindow.h
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

    models/tablemodel.cpp
    models/tablemodel.h
    models/dtcmodel.cpp
    models/dtcmodel.h

    database/definitions.cpp
    database/definitions.h
    database/roms.cpp
    database/roms.h
    database/links.cpp
    database/links.h)

if (UNIX AND NOT APPLE)
    set(libretuner_SRC ${libretuner_SRC}
        os/sockethandler.cpp
        os/sockethandler.h)
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

# Separate the program into a library for testing
add_library(LibreTuner_lib ${libretuner_SRC})

target_link_libraries(LibreTuner_lib Qt5::Widgets Qt5::DataVisualization Qt5::Charts Qt5::PrintSupport cparse ${CMAKE_THREAD_LIBS_INIT} yaml-cpp QHexView sol2 Lua LibLibreTuner)

target_include_directories(LibreTuner_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${Boost_INCLUDE_DIRS})



if (UNIX AND NOT APPLE)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_SOCKETCAN=1)
    target_compile_definitions(LibLibreTuner PRIVATE WITH_SOCKETCAN=1)
    target_link_libraries(LibreTuner_lib stdc++fs)
endif ()

if (WIN32)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_J2534=1)
    target_compile_definitions(LibLibreTuner PRIVATE WITH_J2534=1)
endif (WIN32)

add_executable(LibreTuner main.cpp)
target_link_libraries(LibreTuner LibreTuner_lib)


if (MINGW)
    message("Detected MINGW")
    target_link_options(LibreTuner PRIVATE -mwindows)
endif(MINGW)
