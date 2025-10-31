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
    src/TextEditor.cpp
    src/CellManager.cpp
    src/FontManager.cpp
    src/ScrollManager.cpp
    src/ClipboardHandler.cpp
    src/ClipboardManager.cpp
    src/FileManager.cpp
    src/ReplaceDialog.cpp
    src/DllFunctionManager.cpp
    src/DllDialog.cpp
)

set(HEADERS
    include/MainWindow.h
    include/AboutDialog.h
    include/MenuManager.h
    include/Resource.h
    include/DarkScreen.h
    include/TextEditor.h
    include/CellManager.h
    include/FontManager.h
    include/ScrollManager.h
    include/ClipboardHandler.h
    include/ClipboardManager.h
    include/FileManager.h
    include/ReplaceDialog.h
    include/DllFunctionManager.h
    include/DllDialog.h
)

add_executable(BasicWinAPI WIN32 ${SOURCES} ${HEADERS})

target_include_directories(BasicWinAPI PRIVATE include ${CMAKE_SOURCE_DIR}/third_party/SimpleLib/include)

target_link_libraries(BasicWinAPI user32 gdi32 comctl32 shlwapi ${CMAKE_SOURCE_DIR}/third_party/SimpleLib/lib/SimpleLib.lib)

if(MSVC)
    # Совместимость с SimpleLib, собранной в Release (/MD, iterator debug level 0)
    target_compile_definitions(BasicWinAPI PRIVATE $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
    target_compile_options(BasicWinAPI PRIVATE $<$<CONFIG:Debug>:/MD>)
endif()
