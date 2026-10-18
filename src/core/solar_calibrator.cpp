#include "solar_calibrator.h"

#include "calibration_math.h"

#include <QtMath>

#include <algorithm>
#include <cmath>

namespace radarcal {
namespace {

double degToRad(double value) { return value * M_PI / 180.0; }
double radToDeg(double value) { return value * 180.0 / M_PI; }

} // namespace

SolarCoordinates SolarCalibrator::calculatePosition(
        const QDateTime &localTime, const GeoPoint &station)
{
    const QDateTime utc = localTime.toUTC();
    const double julianDay = utc.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;
    const double centuries = (julianDay - 2451545.0) / 36525.0;
    const double meanLongitude = normalizeAzimuth(
                280.46646 + 36000.76983 * centuries
                + 0.0003032 * centuries * centuries);
    const double meanAnomaly = normalizeAzimuth(
                357.52911 + 35999.05029 * centuries
                - 0.0001537 * centuries * centuries);
    const double eccentricity = 0.016708634 - 0.000042037 * centuries
            - 0.0000001267 * centuries * centuries;
    const double anomalyRad = degToRad(meanAnomaly);
    const double centre =
            (1.914602 - 0.004817 * centuries
             - 0.000014 * centuries * centuries) * std::sin(anomalyRad)
            + (0.019993 - 0.000101 * centuries) * std::sin(2.0 * anomalyRad)
            + 0.000289 * std::sin(3.0 * anomalyRad);
    const double trueLongitude = meanLongitude + centre;
    const double omega = 125.04 - 1934.136 * centuries;
    const double apparentLongitude = trueLongitude - 0.00569
            - 0.00478 * std::sin(degToRad(omega));
    const double meanObliquity = 23.439291 - 0.0130042 * centuries
            - 1.64e-7 * centuries * centuries
            + 5.04e-7 * centuries * centuries * centuries;
    const double obliquity = meanObliquity
            + 0.00256 * std::cos(degToRad(omega));
    const double lambda = degToRad(apparentLongitude);
    const double epsilon = degToRad(obliquity);
    const double declination = std::asin(std::sin(epsilon) * std::sin(lambda));

    double y = std::tan(degToRad(obliquity / 2.0));
    y *= y;
    const double longitudeRad = degToRad(meanLongitude);
    const double equationOfTime = 4.0 * radToDeg(
                y * std::sin(2.0 * longitudeRad)
                - 2.0 * eccentricity * std::sin(anomalyRad)
                + 4.0 * eccentricity * y * std::sin(anomalyRad)
                  * std::cos(2.0 * longitudeRad)
                - 0.5 * y * y * std::sin(4.0 * longitudeRad)
                - 1.25 * eccentricity * eccentricity
                  * std::sin(2.0 * anomalyRad));
    const QTime utcTime = utc.time();
    const double utcMinutes = utcTime.hour() * 60.0 + utcTime.minute()
            + utcTime.second() / 60.0 + utcTime.msec() / 60000.0;
    double trueSolarMinutes = std::fmod(
                utcMinutes + equationOfTime + 4.0 * station.longitudeDeg,
                1440.0);
    if (trueSolarMinutes < 0.0)
        trueSolarMinutes += 1440.0;
    double hourAngleDeg = trueSolarMinutes / 4.0 - 180.0;
    if (hourAngleDeg < -180.0)
        hourAngleDeg += 360.0;

    const double latitude = degToRad(station.latitudeDeg);
    const double hourAngle = degToRad(hourAngleDeg);
    const double sinAltitude = qBound(-1.0,
        std::sin(latitude) * std::sin(declination)
        + std::cos(latitude) * std::cos(declination)
          * std::cos(hourAngle), 1.0);
    double elevation = radToDeg(std::asin(sinAltitude));
    const double azimuth = std::atan2(
                std::sin(hourAngle),
                std::cos(hourAngle) * std::sin(latitude)
                - std::tan(declination) * std::cos(latitude)) + M_PI;

    // Bennett atmospheric-refraction approximation. Pressure is estimated
    // from station altitude and the engineering default temperature is 15 C.
    if (elevation > -1.0) {
        const double pressureHpa = 1013.25
                * std::exp(-station.altitudeM / 8434.0);
        double refractionDeg = 1.02 / std::tan(degToRad(
                elevation + 10.3 / (elevation + 5.11))) / 60.0;
        refractionDeg *= pressureHpa / 1010.0;
        elevation += refractionDeg;
    }

    SolarCoordinates result;
    result.azimuthDeg = normalizeAzimuth(radToDeg(azimuth));
    result.elevationDeg = elevation;
    return result;
}

SolarFitResult SolarCalibrator::fitPeak(
        const QVector<SolarFitSample> &input,
        const SolarFitOptions &options)
{
    QVector<SolarFitSample> samples;
    samples.reserve(input.size());
    for (const SolarFitSample &sample : input) {
        if (qIsFinite(sample.measuredAxisDeg)
                && qIsFinite(sample.theoreticalAxisDeg)
                && qIsFinite(sample.powerDb))
            samples.append(sample);
    }
    SolarFitResult result;
    if (samples.size() < 5) {
        result.peak.message = QStringLiteral("At least five finite samples are required");
        return result;
    }

    std::sort(samples.begin(), samples.end(), [&options](
              const SolarFitSample &left, const SolarFitSample &right) {
        const double leftAxis = options.azimuthAxis
                ? wrappedAzimuthError(left.measuredAxisDeg, options.scanCentreDeg)
                : left.measuredAxisDeg;
        const double rightAxis = options.azimuthAxis
                ? wrappedAzimuthError(right.measuredAxisDeg, options.scanCentreDeg)
                : right.measuredAxisDeg;
        return leftAxis < rightAxis;
    });

    const auto relativeAxis = [&options](const SolarFitSample &sample) {
        return options.azimuthAxis
                ? wrappedAzimuthError(sample.measuredAxisDeg,
                                      options.scanCentreDeg)
                : sample.measuredAxisDeg - options.scanCentreDeg;
    };
    if (relativeAxis(samples.first()) > -0.6 * options.scanHalfRangeDeg
            || relativeAxis(samples.last()) < 0.6 * options.scanHalfRangeDeg) {
        result.peak.message = QStringLiteral("The scan does not bracket both sides of the expected solar position");
        return result;
    }

    QVector<double> raw;
    QVector<double> smooth(samples.size());
    raw.reserve(samples.size());
    for (const SolarFitSample &sample : samples)
        raw.append(sample.powerDb);
    for (int index = 0; index < samples.size(); ++index) {
        double sum = 0.0;
        int count = 0;
        for (int neighbour = qMax(0, index - 1);
             neighbour <= qMin(samples.size() - 1, index + 1); ++neighbour) {
            sum += linearPower(raw.at(neighbour));
            ++count;
        }
        smooth[index] = powerDb(sum / qMax(1, count));
    }

    QVector<double> sortedPower = raw;
    std::sort(sortedPower.begin(), sortedPower.end());
    const double baseline = sortedPower.at(sortedPower.size() / 5);
    const auto maximumIt = std::max_element(smooth.constBegin(), smooth.constEnd());
    const int maximumIndex = int(maximumIt - smooth.constBegin());
    if (maximumIndex == 0 || maximumIndex == smooth.size() - 1) {
        result.peak.message = QStringLiteral("The strongest response is on the scan boundary");
        return result;
    }

    QVector<double> changes;
    for (int index = 1; index < smooth.size(); ++index)
        changes.append(qAbs(smooth.at(index) - smooth.at(index - 1)));
    std::sort(changes.begin(), changes.end());
    const double localNoise = changes.isEmpty()
            ? 0.0 : changes.at(changes.size() / 2);
    const double contrast = *maximumIt - baseline;
    const double requiredContrast = qMax(options.minimumContrastDb,
                                         1.5 * localNoise);
    if (contrast < requiredContrast) {
        result.peak.baselineDb = baseline;
        result.peak.contrastDb = contrast;
        result.peak.message = QStringLiteral("Solar peak contrast is below the adaptive threshold");
        return result;
    }

    const double halfPowerThreshold = baseline + 0.5 * contrast;
    int left = maximumIndex;
    int right = maximumIndex;
    while (left > 0 && smooth.at(left - 1) >= halfPowerThreshold)
        --left;
    while (right + 1 < smooth.size()
           && smooth.at(right + 1) >= halfPowerThreshold)
        ++right;
    if (right - left + 1 > qMax(5, samples.size() / 2)) {
        result.peak.message = QStringLiteral("The response is too broad to identify a unique solar peak");
        return result;
    }

    const double noiseLinear = linearPower(baseline);
    double weightSum = 0.0;
    double measuredSum = 0.0;
    double theoreticalSum = 0.0;
    const double theoryReference = samples.at(maximumIndex).theoreticalAxisDeg;
    for (int index = left; index <= right; ++index) {
        const double weight = qMax(0.0, linearPower(smooth.at(index)) - noiseLinear);
        weightSum += weight;
        measuredSum += weight * relativeAxis(samples.at(index));
        theoreticalSum += weight * (options.azimuthAxis
                ? wrappedAzimuthError(samples.at(index).theoreticalAxisDeg,
                                      theoryReference)
                : samples.at(index).theoreticalAxisDeg);
    }
    if (weightSum <= 0.0) {
        result.peak.message = QStringLiteral("Peak weights are not positive");
        return result;
    }

    result.peak.valid = true;
    result.peak.axisDeg = options.azimuthAxis
            ? normalizeAzimuth(options.scanCentreDeg + measuredSum / weightSum)
            : options.scanCentreDeg + measuredSum / weightSum;
    result.theoreticalAxisDeg = options.azimuthAxis
            ? normalizeAzimuth(theoryReference + theoreticalSum / weightSum)
            : theoreticalSum / weightSum;
    result.peak.powerDb = *maximumIt;
    result.peak.baselineDb = baseline;
    result.peak.contrastDb = contrast;
    const double contrastScore = qBound(0.0, contrast / 8.0, 1.0);
    const double widthScore = qBound(0.0,
        1.0 - (right - left) / double(qMax(1, samples.size() - 1)), 1.0);
    result.peak.quality = 0.8 * contrastScore + 0.2 * widthScore;
    result.pointingErrorDeg = options.azimuthAxis
            ? wrappedAzimuthError(result.peak.axisDeg, result.theoreticalAxisDeg)
            : result.peak.axisDeg - result.theoreticalAxisDeg;
    result.recommendedCorrectionDeg = -result.pointingErrorDeg;
    result.peak.message = QStringLiteral("Solar peak fitted from the excess linear-power centroid");
    return result;
}

} // namespace radarcal

