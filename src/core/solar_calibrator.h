#pragma once

#include "calibration_types.h"

namespace radarcal {

struct SolarFitSample {
    double measuredAxisDeg = 0.0;
    double theoreticalAxisDeg = 0.0;
    double powerDb = std::numeric_limits<double>::quiet_NaN();
    QDateTime timestamp;
};

struct SolarFitOptions {
    bool azimuthAxis = true;
    double scanCentreDeg = 0.0;
    double scanHalfRangeDeg = 3.0;
    double minimumContrastDb = 0.03;
};

struct SolarFitResult {
    PeakResult peak;
    double theoreticalAxisDeg = 0.0;
    double pointingErrorDeg = 0.0;
    double recommendedCorrectionDeg = 0.0;
};

class SolarCalibrator
{
public:
    static SolarCoordinates calculatePosition(const QDateTime &localTime,
                                              const GeoPoint &station);
    static SolarFitResult fitPeak(const QVector<SolarFitSample> &samples,
                                  const SolarFitOptions &options);
};

} // namespace radarcal

