cmake_minimum_required(VERSION 2.8.11)
project(LibreTune)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(libretune_SRC
  main.cpp
  libretune.cpp
  libretune.h
  
  ui/mainwindow.cpp
  ui/mainwindow.h
  ui/canviewer.cpp
  ui/canviewer.h
  ui/canlogview.cpp
  ui/canlogview.h
  ui/romlistmodel.cpp
  ui/romlistmodel.h
  ui/flowlayout.cpp
  ui/flowlayout.h
  ui/romwidget.cpp
  ui/romwidget.h
  ui/downloadwindow.cpp
  ui/downloadwindow.h
  
  protocols/caninterface.cpp
  protocols/caninterface.h
  protocols/socketcaninterface.cpp
  protocols/socketcaninterface.h
  protocols/canlog.cpp
  protocols/canlog.h
  protocols/isotpinterface.cpp
  
  os/sockethandler.cpp
  os/sockethandler.h
)

# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(libretune_SRC
ui/mainwindow.ui
ui/canviewer.ui
ui/downloadwindow.ui
)

qt5_add_resources(libretune_SRC
resources/icons.qrc
)

# Tell CMake to create the helloworld executable
add_executable(LibreTune ${libretune_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(LibreTune Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
