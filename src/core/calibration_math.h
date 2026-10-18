#pragma once

#include "calibration_types.h"

namespace radarcal {

constexpr double kEarthRadiusM = 6371008.8;
constexpr double kSpeedOfLightMps = 299792458.0;

double normalizeAzimuth(double degrees);
double wrappedAzimuthError(double valueDeg, double referenceDeg);
double linearPower(double db);
double powerDb(double linear);
double robustPowerAverage(QVector<double> values);
double horizontalDistance(const GeoPoint &from, const GeoPoint &to);
double slantDistance(const GeoPoint &from, const GeoPoint &to);
double bearing(const GeoPoint &from, const GeoPoint &to);
double elevationAngle(const GeoPoint &from, const GeoPoint &to);

} // namespace radarcal

