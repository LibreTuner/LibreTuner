cmake_minimum_required(VERSION 2.8.11)
project(LibreTuner)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(libretuner_SRC
  main.cpp
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
  
  definitions/definitionmanager.cpp
  definitions/definitionmanager.h
  definitions/definition.cpp
  definitions/definition.h
  definitions/tabledefinitions.cpp
  definitions/tabledefinitions.h
  definitions/checksummanager.cpp
  definitions/checksummanager.h
  
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
  
  protocols/caninterface.cpp
  protocols/caninterface.h
  protocols/canlog.cpp
  protocols/canlog.h
  protocols/isotpinterface.cpp
  protocols/udsprotocol.cpp
  protocols/udsprotocol.h
)

if (UNIX)
    set(libretuner_SRC ${libretuner_SRC}
        protocols/socketcaninterface.cpp
        protocols/socketcaninterface.h
        os/sockethandler.cpp
        os/sockethandler.h
    )
endif (UNIX)


# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(libretuner_SRC
ui/mainwindow.ui
ui/canviewer.ui
ui/downloadwindow.ui
ui/createtunedialog.ui
ui/tuneeditor.ui
)

qt5_add_resources(libretuner_SRC
resources/icons.qrc
resources/definitions.qrc
)

# Tell CMake to create the executable
add_executable(LibreTuner ${libretuner_SRC})

target_link_libraries(LibreTuner Qt5::Widgets Qt5::DataVisualization Qt5::Charts ${CMAKE_THREAD_LIBS_INIT})


if (UNIX)
    target_compile_definitions(LibreTuner PRIVATE WITH_SOCKETCAN=1)
endif (UNIX)

if (WIN32)
    target_compile_definitions(LibreTuner PRIVATE WITH_J2534=1)
endif (WIN32)
