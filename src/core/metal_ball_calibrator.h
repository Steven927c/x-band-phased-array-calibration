#pragma once

#include "calibration_types.h"

namespace radarcal {

struct MetalBallConfig {
    GeoPoint radar;
    GeoPoint uav;
    double ropeLengthM = 20.0;
    double sphereDiameterCm = 30.0;
    double frequencyGHz = 9.4;
    double antennaApertureM = 1.0;
    double pulseWidthUs = 1.0;
    double waterDielectricFactor = 0.93;
    double azimuthBeamwidthDeg = 0.0;
    double elevationBeamwidthDeg = 0.0;
};

struct MetalBallGeometry {
    GeoPoint sphere;
    double wavelengthM = 0.0;
    double rcsSquareM = 0.0;
    double rcsDbsm = 0.0;
    double opticalRatio = 0.0;
    double horizontalRangeM = 0.0;
    double slantRangeM = 0.0;
    double azimuthDeg = 0.0;
    double elevationDeg = 0.0;
    double uavElevationDeg = 0.0;
    double farFieldDistanceM = 0.0;
    double azimuthBeamwidthDeg = 0.0;
    double elevationBeamwidthDeg = 0.0;
    double pulseVolumeCubicM = 0.0;
    double equivalentReflectivityDbz = std::numeric_limits<double>::quiet_NaN();
};

struct RhiPeakOptions {
    double sphereTheoryDeg = 0.0;
    double uavTheoryDeg = 0.0;
    double verticalBeamwidthDeg = 1.0;
    double minimumContrastDb = 3.0;
};

struct PpiPeakOptions {
    double theoryDeg = 0.0;
    double halfWidthDeg = 40.0;
    double minimumContrastDb = 3.0;
};

class MetalBallCalibrator
{
public:
    static MetalBallGeometry calculateGeometry(const MetalBallConfig &config);
    static int expectedRangeBin(double rangeM, double gateSpacingM, int binCount);
    static TripletSample extractTriplet(double axisDeg,
                                        const QVector<double> &radialDb,
                                        int targetBin,
                                        const QDateTime &timestamp = QDateTime());
    static PeakResult selectRhiSpherePeak(const QVector<AxisSample> &samples,
                                          const RhiPeakOptions &options,
                                          double *uavPeakDeg = nullptr);
    static PeakResult selectPpiPeak(const QVector<AxisSample> &samples,
                                    const PpiPeakOptions &options);
};

} // namespace radarcal

