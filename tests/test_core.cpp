#include "core/calibration_math.h"
#include "core/metal_ball_calibrator.h"
#include "core/solar_calibrator.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

#include <cmath>

namespace {

int failures = 0;

void expect(bool condition, const char *message)
{
    if (!condition) {
        qCritical().noquote() << "FAIL:" << message;
        ++failures;
    }
}

void expectNear(double actual, double expected, double tolerance,
                const char *message)
{
    if (!qIsFinite(actual) || std::fabs(actual - expected) > tolerance) {
        qCritical().noquote() << "FAIL:" << message
                              << "actual=" << actual << "expected=" << expected;
        ++failures;
    }
}

double gaussian(double x, double centre, double sigma, double amplitude)
{
    const double normalized = (x - centre) / sigma;
    return amplitude * std::exp(-0.5 * normalized * normalized);
}

void testMath()
{
    expectNear(radarcal::normalizeAzimuth(-10.0), 350.0, 1.0e-9,
               "negative azimuth normalization");
    expectNear(radarcal::wrappedAzimuthError(1.0, 359.0), 2.0, 1.0e-9,
               "azimuth wrap error");
    radarcal::GeoPoint origin{0.0, 0.0, 0.0};
    radarcal::GeoPoint north{1.0, 0.0, 0.0};
    expectNear(radarcal::horizontalDistance(origin, north), 111195.0, 150.0,
               "one degree latitude distance");
    expectNear(radarcal::bearing(origin, north), 0.0, 1.0e-9,
               "north bearing");
}

void testMetalBallGeometry()
{
    radarcal::MetalBallConfig config;
    config.radar = {30.0, 104.0, 500.0};
    config.uav = {30.009, 104.0, 620.0};
    config.ropeLengthM = 20.0;
    config.sphereDiameterCm = 30.0;
    config.frequencyGHz = 9.4;
    config.antennaApertureM = 1.0;
    const radarcal::MetalBallGeometry geometry =
            radarcal::MetalBallCalibrator::calculateGeometry(config);
    expect(geometry.slantRangeM > 990.0 && geometry.slantRangeM < 1020.0,
           "metal-ball slant range");
    expectNear(geometry.azimuthDeg, 0.0, 0.05, "metal-ball azimuth");
    expect(geometry.elevationDeg > 5.0 && geometry.elevationDeg < 6.5,
           "metal-ball elevation");
    expect(geometry.rcsSquareM > 0.07 && geometry.rcsSquareM < 0.071,
           "optical sphere RCS");
    expect(qIsFinite(geometry.equivalentReflectivityDbz),
           "expected reflectivity is finite");
    expect(radarcal::MetalBallCalibrator::expectedRangeBin(1000.0, 30.0, 100)
           == 32, "range-bin conversion");
}

void testMetalBallPeakSelection()
{
    QVector<radarcal::AxisSample> samples;
    for (int index = 0; index <= 160; ++index) {
        const double angle = 5.0 + index * 0.05;
        const double power = gaussian(angle, 8.0, 0.35, 18.0)
                + gaussian(angle, 10.0, 0.35, 24.0);
        samples.append({angle, power, {}});
    }
    radarcal::RhiPeakOptions options;
    options.sphereTheoryDeg = 8.0;
    options.uavTheoryDeg = 10.0;
    options.verticalBeamwidthDeg = 1.0;
    double uavPeak = 0.0;
    const radarcal::PeakResult peak =
            radarcal::MetalBallCalibrator::selectRhiSpherePeak(
                samples, options, &uavPeak);
    expect(peak.valid, "RHI sphere peak is valid");
    expectNear(peak.axisDeg, 8.0, 0.15, "RHI sphere peak angle");
    expectNear(uavPeak, 10.0, 0.15, "RHI carrier peak angle");
}

void testSolarPositionAndFit()
{
    const QDateTime equinoxNoon(QDate(2024, 3, 20), QTime(12, 0), Qt::UTC);
    const radarcal::SolarCoordinates sun =
            radarcal::SolarCalibrator::calculatePosition(
                equinoxNoon, {0.0, 0.0, 0.0});
    expect(sun.elevationDeg > 85.0, "equinox solar elevation near zenith");

    QVector<radarcal::SolarFitSample> samples;
    for (int index = -20; index <= 20; ++index) {
        const double axis = 120.0 + index * 0.15;
        samples.append({axis, 120.0, -10.0 + gaussian(axis, 121.2, 0.7, 4.0), {}});
    }
    radarcal::SolarFitOptions options;
    options.azimuthAxis = true;
    options.scanCentreDeg = 120.0;
    options.scanHalfRangeDeg = 3.0;
    const radarcal::SolarFitResult fit =
            radarcal::SolarCalibrator::fitPeak(samples, options);
    expect(fit.peak.valid, "solar fit is valid");
    expectNear(fit.peak.axisDeg, 121.2, 0.2, "solar fitted azimuth");
    expectNear(fit.pointingErrorDeg, 1.2, 0.2, "solar pointing error");
    expectNear(fit.recommendedCorrectionDeg, -1.2, 0.2,
               "solar recommended correction");
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    testMath();
    testMetalBallGeometry();
    testMetalBallPeakSelection();
    testSolarPositionAndFit();
    if (failures == 0)
        qInfo().noquote() << "All radar calibration tests passed.";
    return failures == 0 ? 0 : 1;
}

