#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

#include <limits>

namespace radarcal {

struct GeoPoint {
    double latitudeDeg = 0.0;
    double longitudeDeg = 0.0;
    double altitudeM = 0.0;
};

struct AxisSample {
    double axisDeg = 0.0;
    double powerDb = std::numeric_limits<double>::quiet_NaN();
    QDateTime timestamp;
};

struct TripletSample {
    double axisDeg = 0.0;
    double centreDb = std::numeric_limits<double>::quiet_NaN();
    double minusOneDb = std::numeric_limits<double>::quiet_NaN();
    double plusOneDb = std::numeric_limits<double>::quiet_NaN();
    QDateTime timestamp;
};

struct PeakResult {
    bool valid = false;
    double axisDeg = 0.0;
    double powerDb = std::numeric_limits<double>::quiet_NaN();
    double baselineDb = std::numeric_limits<double>::quiet_NaN();
    double contrastDb = 0.0;
    double quality = 0.0;
    QString message;
};

struct SolarCoordinates {
    double azimuthDeg = 0.0;
    double elevationDeg = 0.0;
};

} // namespace radarcal

