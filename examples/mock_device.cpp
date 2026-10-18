#include "mock_device.h"

#include <QFileInfo>
#include <QTimer>

namespace {

void finishLater(MockCalibrationDevice *device, bool accepted,
                 const QString &detail)
{
    QTimer::singleShot(0, device, [device, accepted, detail] {
        emit device->commandCompleted(accepted, detail);
    });
}

} // namespace

void MockCalibrationDevice::loadAndSendParameterFile(const QString &filePath)
{
    finishLater(this, QFileInfo::exists(filePath),
                QFileInfo::exists(filePath)
                ? QStringLiteral("Example parameter request accepted")
                : QStringLiteral("Parameter file not found"));
}

void MockCalibrationDevice::position(double, double)
{
    finishLater(this, true, QStringLiteral("Example position request accepted"));
}

void MockCalibrationDevice::scanAzimuth(double, double, double, double)
{
    finishLater(this, true, QStringLiteral("Example azimuth scan request accepted"));
}

void MockCalibrationDevice::scanElevation(double, double, double, double)
{
    finishLater(this, true, QStringLiteral("Example elevation scan request accepted"));
}

void MockCalibrationDevice::stop()
{
    finishLater(this, true, QStringLiteral("Example stop request accepted"));
}

void MockCalibrationDevice::setSoftwareZero(bool)
{
    finishLater(this, false,
                QStringLiteral("Mock adapter never writes a real servo zero"));
}

