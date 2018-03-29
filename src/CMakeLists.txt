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
  
  protocols/caninterface.cpp
  protocols/caninterface.h
  protocols/socketcaninterface.cpp
  protocols/socketcaninterface.h
  protocols/canlog.cpp
  protocols/canlog.h
  protocols/isotpinterface.cpp
  protocols/udsprotocol.cpp
  protocols/udsprotocol.h
  
  os/sockethandler.cpp
  os/sockethandler.h
)

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
)

# Tell CMake to create the helloworld executable
add_executable(LibreTuner ${libretuner_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(LibreTuner Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
    target_compile_definitions(LibreTuner PRIVATE WITH_SOCKETCAN=1)
endif (UNIX)

if (WIN32)
    target_compile_definitions(LibreTuner PRIVATE WITH_J2534=1)
endif (WIN32)
