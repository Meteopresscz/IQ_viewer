cmake_minimum_required(VERSION 3.16)

project(IQ_viewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# --- Kiss FFT Integration Start ---

# 1. (Recommended) Set options for the kissfft build.
#    Build as a static library and disable tools and tests.
set(KISSFFT_STATIC ON CACHE BOOL "Build kissfft as a static library")
set(KISSFFT_TOOLS OFF CACHE BOOL "Do not build kissfft command-line tools")
set(KISSFFT_TEST OFF CACHE BOOL "Do not build kissfft tests")

# 2. Add the kissfft subdirectory. CMake will find and use its CMakeLists.txt.
add_subdirectory(libs/kissfft)

# --- Kiss FFT Integration End ---

set(TS_FILES IQ_viewer_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        iq_viewer.cpp
        iq_viewer.h
        iq_viewer.ui
        ${TS_FILES}
)

# This block is now correctly structured for Qt6 vs older versions
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(IQ_viewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    # Fallback for Qt5
    if(ANDROID)
        add_library(IQ_viewer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(IQ_viewer
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 3. Link your application with Qt Widgets and the kissfft library.
#    Using "kissfft::kissfft" is best practice as it's a CMake alias.
#    CMake will also automatically handle the include directories.
target_link_libraries(IQ_viewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets kissfft::kissfft)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.IQ_viewer)
endif()
set_target_properties(IQ_viewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS IQ_viewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(IQ_viewer)
endif()
