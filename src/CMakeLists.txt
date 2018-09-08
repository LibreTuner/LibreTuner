cmake_minimum_required(VERSION 2.8.11)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 17)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(libretuner_SRC
    libretuner.cpp
    libretuner.h
    downloadinterface.cpp
    downloadinterface.h
    rommanager.cpp
    rommanager.h
    tunemanager.cpp
    tunemanager.h
    tune.cpp
    tune.h
    rom.cpp
    rom.h
    tablegroup.cpp
    tablegroup.h
    table.cpp
    table.h
    flasher.cpp
    flasher.h
    flashable.cpp
    flashable.h
    endian.h
    udsauthenticator.cpp
    udsauthenticator.h
    timer.cpp
    timer.h
    datalog.cpp
    datalog.h
    vehicle.cpp
    vehicle.h
    datalink.cpp
    datalink.h
    interface.cpp
    interface.h
    interfacemanager.cpp
    interfacemanager.h
    logger.cpp
    logger.h
    timerrunloop.cpp
    timerrunloop.h
    datalogger.cpp
    datalogger.h
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

    util/signal.cpp
    util/signal.h

    util.hpp

    definitions/definitionmanager.cpp
    definitions/definitionmanager.h
    definitions/definition.cpp
    definitions/definition.h
    definitions/tabledefinitions.cpp
    definitions/tabledefinitions.h
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
    ui/flowlayout.cpp
    ui/flowlayout.h
    ui/romwidget.cpp
    ui/romwidget.h
    ui/downloadwindow.cpp
    ui/downloadwindow.h
    ui/createtunedialog.cpp
    ui/createtunedialog.h
    ui/tuneeditor.cpp
    ui/tuneeditor.h
    ui/tunewidget.cpp
    ui/tunewidget.h
    ui/verticallabel.cpp
    ui/verticallabel.h
    ui/graphwidget.cpp
    ui/graphwidget.h
    ui/flashwindow.cpp
    ui/flashwindow.h
    ui/interfaceswindow.cpp
    ui/interfaceswindow.h
    ui/addinterfacedialog.cpp
    ui/addinterfacedialog.h
    ui/socketcansettingsui.cpp
    ui/socketcansettingsui.h
    ui/settingswidget.cpp
    ui/settingswidget.h
    ui/titlebar.cpp
    ui/titlebar.h
    ui/styledwindow.cpp
    ui/styledwindow.h
    ui/dataloggerwindow.cpp
    ui/dataloggerwindow.h
    ui/logview.cpp
    ui/logview.h
    ui/diagnosticswidget.cpp
    ui/diagnosticswidget.h

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
    protocols/isotpprotocol.h)

if (UNIX AND NOT APPLE)
    set(libretuner_SRC ${libretuner_SRC}
        protocols/socketcaninterface.cpp
        protocols/socketcaninterface.h
        os/sockethandler.cpp
        os/sockethandler.h)
endif ()

if (WIN32)
    set(libretuner_SRC ${libretuner_SRC}
        j2534/j2534.cpp
        j2534/j2534.h
        j2534/j2534manager.cpp
        j2534/j2534manager.h
        j2534/j2534caninterface.cpp
        j2534/j2534caninterface.h)
endif ()


# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(libretuner_SRC
    ui/canviewer.ui
    ui/downloadwindow.ui
    ui/createtunedialog.ui
    ui/tuneeditor.ui
    ui/flashwindow.ui
    ui/interfaceswindow.ui
    ui/addinterfacedialog.ui
)

qt5_add_resources(libretuner_SRC
    resources/icons.qrc
    resources/definitions.qrc
    resources/stylesheet.qrc
    resources/codes.qrc
)

# Separate the program into a library for testing
add_library(LibreTuner_lib ${libretuner_SRC})

target_link_libraries(LibreTuner_lib Qt5::Widgets Qt5::DataVisualization Qt5::Charts GSL ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(LibreTuner_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if (UNIX AND NOT APPLE)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_SOCKETCAN=1)
endif ()

if (WIN32)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_J2534=1)
endif (WIN32)

add_executable(LibreTuner main.cpp)
target_link_libraries(LibreTuner LibreTuner_lib)
