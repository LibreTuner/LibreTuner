cmake_minimum_required(VERSION 2.8.11)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 14)

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

  util/signal.cpp
  util/signal.h
  
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
  ui/flashwindow.cpp
  ui/flashwindow.h
  
  protocols/caninterface.cpp
  protocols/caninterface.h
  protocols/canlog.cpp
  protocols/canlog.h
  protocols/isotpinterface.cpp
  protocols/udsprotocol.cpp
  protocols/udsprotocol.h
  protocols/mockcaninterface.cpp
  protocols/mockcaninterface.h
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
ui/flashwindow.ui
)

qt5_add_resources(libretuner_SRC
resources/icons.qrc
resources/definitions.qrc
)

# Separate the program into a library for testing
add_library(LibreTuner_lib ${libretuner_SRC})

target_link_libraries(LibreTuner_lib Qt5::Widgets Qt5::DataVisualization Qt5::Charts ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

target_include_directories(LibreTuner_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if (UNIX)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_SOCKETCAN=1)
endif (UNIX)

if (WIN32)
    target_compile_definitions(LibreTuner_lib PRIVATE WITH_J2534=1)
endif (WIN32)

add_executable(LibreTuner main.cpp)
target_link_libraries(LibreTuner LibreTuner_lib)
