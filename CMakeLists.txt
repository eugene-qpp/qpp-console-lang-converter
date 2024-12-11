cmake_minimum_required(VERSION 3.20)
project(qpp-console-lang-converter VERSION 1.0.0 LANGUAGES CXX)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

qt_standard_project_setup()

set(SRCS src/main.cpp src/convert.cpp src/appwindow.cpp)

qt_add_executable(qpp-console-lang-converter WIN32 ${SRCS})
target_link_libraries(qpp-console-lang-converter PRIVATE Qt6::Widgets)
