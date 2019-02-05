cmake_minimum_required(VERSION 3.13)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(libretuner_SRC
    libretuner.cpp

    libretuner.h
    rommanager.cpp
    rommanager.h
    rom.cpp
    rom.h
    tablegroup.cpp
    tablegroup.h
    table.cpp
    table.h
    endian.h
    udsauthenticator.cpp
    udsauthenticator.h
    timer.cpp
    timer.h
    vehicle.cpp
    vehicle.h
    logger.cpp
    logger.h
    timerrunloop.cpp
    timerrunloop.h
    asyncroutine.cpp
    asyncroutine.h
    log.cpp
    log.h
    scanresult.cpp
    scanresult.h
    diagnosticsinterface.cpp
    diagnosticsinterface.h
    dtcdescriptions.cpp
    dtcdescriptions.h
    
    download/downloader.cpp
    download/downloader.h
    
    flash/flasher.cpp
    flash/flasher.h
    flash/flashable.cpp
    flash/flashable.h
    
    datalog/datalogger.cpp
    datalog/datalogger.h
    datalog/datalog.cpp
    datalog/datalog.h

    util/signal.cpp
    util/signal.h

    util.hpp

    definitions/definitionmanager.cpp
    definitions/definitionmanager.h
    definitions/definition.cpp
    definitions/definition.h
    definitions/checksummanager.cpp
    definitions/checksummanager.h
    definitions/piddefinitions.cpp
    definitions/piddefinitions.h

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/canviewer.cpp
    ui/canviewer.h
    ui/canlogview.cpp
    ui/canlogview.h
    ui/downloadwindow.cpp
    ui/downloadwindow.h
    ui/createtunedialog.cpp
    ui/createtunedialog.h
    ui/verticallabel.cpp
    ui/verticallabel.h
    ui/flasherwindow.cpp
    ui/flasherwindow.h
    ui/titlebar.cpp
    ui/titlebar.h
    ui/styledwindow.cpp
    ui/styledwindow.h
    ui/dataloggerwindow.cpp
    ui/dataloggerwindow.h
    ui/romsview.cpp
    ui/romsview.h
    ui/tunedialog.cpp
    ui/tunedialog.h
    ui/romexplorer.cpp
    ui/romexplorer.h
    ui/setupdialog.cpp
    ui/setupdialog.h
    ui/datalinkswidget.cpp
    ui/datalinkswidget.h
    ui/authoptionsview.cpp
    ui/authoptionsview.h
    
    ui/docks/logview.cpp
    ui/docks/logview.h
    ui/docks/diagnosticswidget.cpp
    ui/docks/diagnosticswidget.h
    ui/docks/sidebarwidget.cpp
    ui/docks/sidebarwidget.h
    ui/docks/tableswidget.cpp
    ui/docks/tableswidget.h
    ui/docks/editorwidget.cpp
    ui/docks/editorwidget.h
    ui/docks/graphwidget.cpp
    ui/docks/graphwidget.h
    
    ui/tunesmodel.cpp
    ui/tunesmodel.h

    protocols/caninterface.cpp
    protocols/caninterface.h
    protocols/canlog.cpp
    protocols/canlog.h
    protocols/udsprotocol.cpp
    protocols/udsprotocol.h
    protocols/mockcaninterface.cpp
    protocols/mockcaninterface.h
    protocols/isotpserver.cpp
    protocols/isotpserver.h
    protocols/isotpprotocol.cpp
    protocols/isotpprotocol.h

    datalink/datalink.cpp
    datalink/datalink.h
    )

if (UNIX AND NOT APPLE)
    set(libretuner_SRC ${libretuner_SRC}
        protocols/socketcaninterface.cpp
        protocols/socketcaninterface.h
        os/sockethandler.cpp
        os/sockethandler.h
        datalink/socketcan.cpp
        datalink/socketcan.h)
endif ()

if (WIN32)
    set(libretuner_SRC ${libretuner_SRC}
        j2534/j2534.cpp
        j2534/j2534.h
        j2534/j2534caninterface.cpp
        j2534/j2534caninterface.h

        datalink/passthru.cpp
        datalink/passthru.h)
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

target_link_libraries(LibreTuner_lib Qt5::Widgets Qt5::DataVisualization Qt5::Charts cparse ${CMAKE_THREAD_LIBS_INIT} yaml-cpp QHexView)

target_include_directories(LibreTuner_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${Boost_INCLUDE_DIRS})



if (UNIX AND NOT APPLE)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_SOCKETCAN=1)
endif ()

if (WIN32)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_J2534=1)
endif (WIN32)

add_executable(LibreTuner main.cpp)
target_link_libraries(LibreTuner LibreTuner_lib)


if (MINGW)
    message("Detected MINGW")
    target_link_options(LibreTuner PRIVATE -mwindows)
endif(MINGW)
