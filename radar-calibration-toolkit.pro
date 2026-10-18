QT += core
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = radar-calibration-tests

INCLUDEPATH += $$PWD/src

SOURCES += \
    src/core/calibration_math.cpp \
    src/core/metal_ball_calibrator.cpp \
    src/core/solar_calibrator.cpp \
    tests/test_core.cpp

HEADERS += \
    src/core/calibration_types.h \
    src/core/calibration_math.h \
    src/core/metal_ball_calibrator.h \
    src/core/solar_calibrator.h \
    src/device/calibration_device.h

