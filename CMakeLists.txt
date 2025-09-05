cmake_minimum_required(VERSION 3.10)
project(BasicWinAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

set(SOURCES
    main.cpp
    src/MainWindow.cpp
    src/AboutDialog.cpp
    src/MenuManager.cpp
    src/DarkScreen.cpp
)

set(HEADERS
    include/MainWindow.h
    include/AboutDialog.h
    include/MenuManager.h
    include/Resource.h
    include/DarkScreen.h
)

add_executable(BasicWinAPI WIN32 ${SOURCES} ${HEADERS})

target_include_directories(BasicWinAPI PRIVATE include)

target_link_libraries(BasicWinAPI user32 gdi32 comctl32)
