#pragma once

#include <QObject>
#include <QString>
#include <QVector>

namespace radarcal {

struct RadarObservation {
    qint64 timestampMs = 0;
    double azimuthDeg = 0.0;
    double mechanicalElevationDeg = 0.0;
    double electricElevationDeg = 0.0;
    double gateSpacingM = 0.0;
    QVector<double> reflectivityDbz;
    QVector<double> velocityMps;
    QVector<double> spectrumWidthMps;
    QVector<double> differentialReflectivityDb;
    QVector<double> correlationCoefficient;
};

// Implement this interface in the host application. The open-source module
// never assumes a proprietary servo, DSP or DBF command layout.
class CalibrationDevice : public QObject
{
    Q_OBJECT
public:
    explicit CalibrationDevice(QObject *parent = nullptr) : QObject(parent) {}
    ~CalibrationDevice() override = default;

    virtual void loadAndSendParameterFile(const QString &filePath) = 0;
    virtual void position(double azimuthDeg, double elevationDeg) = 0;
    virtual void scanAzimuth(double startDeg, double endDeg,
                             double elevationDeg, double speedDegPerSecond) = 0;
    virtual void scanElevation(double startDeg, double endDeg,
                               double azimuthDeg, double speedDegPerSecond) = 0;
    virtual void stop() = 0;
    virtual void setSoftwareZero(bool azimuthAxis) = 0;

signals:
    void poseUpdated(double azimuthDeg, double elevationDeg);
    void observationAvailable(const radarcal::RadarObservation &observation);
    void commandCompleted(bool accepted, const QString &detail);
};

} // namespace radarcal

Q_DECLARE_METATYPE(radarcal::RadarObservation)

