cmake_minimum_required(VERSION 3.16)

project(IQ_viewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt modules, including PrintSupport needed by QCustomPlot
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport LinguistTools)

# --- Kiss FFT Integration Start ---

# Set options for the kissfft build (static library, no tools/tests)
set(KISSFFT_STATIC ON CACHE BOOL "Build kissfft as a static library")
set(KISSFFT_TOOLS OFF CACHE BOOL "Do not build kissfft command-line tools")
set(KISSFFT_TEST OFF CACHE BOOL "Do not build kissfft tests")

# Add the kissfft subdirectory so CMake can find its own CMakeLists.txt
add_subdirectory(libs/kissfft)

# --- Kiss FFT Integration End ---

set(TS_FILES IQ_viewer_en_US.ts)

# Define all project sources, including the correct path to QCustomPlot
set(PROJECT_SOURCES
        main.cpp
        iq_viewer.cpp
        iq_viewer.h
        iq_viewer.ui
        libs/customPlot/qcustomplot.cpp
        libs/customPlot/qcustomplot.h
        ${TS_FILES}
)

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

# --- FIX: Add the include directory for QCustomPlot ---
# This command tells the compiler to look in this directory for header files.
target_include_directories(IQ_viewer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/customPlot"
)

# Link your application with Qt Widgets, PrintSupport, and kissfft
target_link_libraries(IQ_viewer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    kissfft::kissfft
)

# Bundle identifier for macOS/iOS
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
