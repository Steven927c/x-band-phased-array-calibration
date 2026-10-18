#include "calibration_math.h"

#include <QtMath>

#include <algorithm>
#include <cmath>

namespace radarcal {

double normalizeAzimuth(double degrees)
{
    degrees = std::fmod(degrees, 360.0);
    return degrees < 0.0 ? degrees + 360.0 : degrees;
}

double wrappedAzimuthError(double valueDeg, double referenceDeg)
{
    double error = normalizeAzimuth(valueDeg) - normalizeAzimuth(referenceDeg);
    if (error > 180.0)
        error -= 360.0;
    if (error < -180.0)
        error += 360.0;
    return error;
}

double linearPower(double db)
{
    return std::pow(10.0, db / 10.0);
}

double powerDb(double linear)
{
    return 10.0 * std::log10(qMax(linear, 1.0e-30));
}

double robustPowerAverage(QVector<double> values)
{
    values.erase(std::remove_if(values.begin(), values.end(),
                                [](double value) { return !qIsFinite(value); }),
                 values.end());
    if (values.isEmpty())
        return std::numeric_limits<double>::quiet_NaN();
    std::sort(values.begin(), values.end());
    const int trim = values.size() >= 10 ? values.size() / 10 : 0;
    double sum = 0.0;
    int count = 0;
    for (int index = trim; index < values.size() - trim; ++index) {
        sum += linearPower(values.at(index));
        ++count;
    }
    return count > 0 ? powerDb(sum / count) : values.at(values.size() / 2);
}

double horizontalDistance(const GeoPoint &from, const GeoPoint &to)
{
    const double lat1 = qDegreesToRadians(from.latitudeDeg);
    const double lat2 = qDegreesToRadians(to.latitudeDeg);
    const double deltaLat = lat2 - lat1;
    const double deltaLon = qDegreesToRadians(to.longitudeDeg - from.longitudeDeg);
    const double a = std::sin(deltaLat / 2.0) * std::sin(deltaLat / 2.0)
            + std::cos(lat1) * std::cos(lat2)
            * std::sin(deltaLon / 2.0) * std::sin(deltaLon / 2.0);
    return 2.0 * kEarthRadiusM * std::asin(std::sqrt(qBound(0.0, a, 1.0)));
}

double slantDistance(const GeoPoint &from, const GeoPoint &to)
{
    const double horizontal = horizontalDistance(from, to);
    const double vertical = to.altitudeM - from.altitudeM;
    return std::hypot(horizontal, vertical);
}

double bearing(const GeoPoint &from, const GeoPoint &to)
{
    const double lat1 = qDegreesToRadians(from.latitudeDeg);
    const double lat2 = qDegreesToRadians(to.latitudeDeg);
    const double deltaLon = qDegreesToRadians(to.longitudeDeg - from.longitudeDeg);
    const double y = std::sin(deltaLon) * std::cos(lat2);
    const double x = std::cos(lat1) * std::sin(lat2)
            - std::sin(lat1) * std::cos(lat2) * std::cos(deltaLon);
    return normalizeAzimuth(qRadiansToDegrees(std::atan2(y, x)));
}

double elevationAngle(const GeoPoint &from, const GeoPoint &to)
{
    return qRadiansToDegrees(std::atan2(to.altitudeM - from.altitudeM,
                                       horizontalDistance(from, to)));
}

} // namespace radarcal

