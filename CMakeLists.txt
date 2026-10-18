cmake_minimum_required(VERSION 3.16)

project(XBandPhasedArrayCalibration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

option(RCT_BUILD_DEMO "Build the Qt Widgets demonstration application" ON)
option(RCT_BUILD_TESTS "Build unit tests" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(radar_calibration_core
    src/core/calibration_types.h
    src/core/calibration_math.h
    src/core/calibration_math.cpp
    src/core/metal_ball_calibrator.h
    src/core/metal_ball_calibrator.cpp
    src/core/solar_calibrator.h
    src/core/solar_calibrator.cpp
    src/device/calibration_device.h
)
target_include_directories(radar_calibration_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(radar_calibration_core PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(radar_calibration_core PRIVATE QT_NO_CAST_FROM_ASCII)

if(RCT_BUILD_DEMO)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
    add_executable(radar_calibration_demo
        app/main.cpp
        app/demo_window.h
        app/demo_window.cpp
        src/widgets/line_plot_widget.h
        src/widgets/line_plot_widget.cpp
    )
    target_link_libraries(radar_calibration_demo PRIVATE
        radar_calibration_core Qt${QT_VERSION_MAJOR}::Widgets)
endif()

if(RCT_BUILD_TESTS)
    enable_testing()
    add_executable(radar_calibration_tests tests/test_core.cpp)
    target_link_libraries(radar_calibration_tests PRIVATE radar_calibration_core)
    add_test(NAME radar_calibration_tests COMMAND radar_calibration_tests)
endif()

install(TARGETS radar_calibration_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
install(DIRECTORY src/core src/device DESTINATION include/radar-calibration-toolkit
    FILES_MATCHING PATTERN "*.h")

