#pragma once

#include "device/calibration_device.h"

// Minimal integration skeleton. Replace each method body with the host
// application's asynchronous device-control call, then emit commandCompleted.
class MockCalibrationDevice : public radarcal::CalibrationDevice
{
    Q_OBJECT
public:
    explicit MockCalibrationDevice(QObject *parent = nullptr)
        : CalibrationDevice(parent) {}

    void loadAndSendParameterFile(const QString &filePath) override;
    void position(double azimuthDeg, double elevationDeg) override;
    void scanAzimuth(double startDeg, double endDeg,
                     double elevationDeg, double speedDegPerSecond) override;
    void scanElevation(double startDeg, double endDeg,
                       double azimuthDeg, double speedDegPerSecond) override;
    void stop() override;
    void setSoftwareZero(bool azimuthAxis) override;
};

