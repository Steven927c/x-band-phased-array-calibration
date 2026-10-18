QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = radar-calibration-demo

INCLUDEPATH += $$PWD/src $$PWD/app

SOURCES += \
    src/core/calibration_math.cpp \
    src/core/metal_ball_calibrator.cpp \
    src/core/solar_calibrator.cpp \
    src/widgets/line_plot_widget.cpp \
    app/demo_window.cpp \
    app/main.cpp

HEADERS += \
    src/core/calibration_types.h \
    src/core/calibration_math.h \
    src/core/metal_ball_calibrator.h \
    src/core/solar_calibrator.h \
    src/widgets/line_plot_widget.h \
    app/demo_window.h

