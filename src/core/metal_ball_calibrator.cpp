#include "metal_ball_calibrator.h"

#include "calibration_math.h"

#include <QtMath>

#include <algorithm>
#include <cmath>

namespace radarcal {
namespace {

constexpr double kCircularAntennaHpbwFactor = 1.02;

struct Candidate {
    double axis = 0.0;
    double value = 0.0;
    double smooth = 0.0;
};

QVector<AxisSample> finiteSortedSamples(const QVector<AxisSample> &input)
{
    QVector<AxisSample> result;
    result.reserve(input.size());
    for (const AxisSample &sample : input) {
        if (qIsFinite(sample.axisDeg) && qIsFinite(sample.powerDb))
            result.append(sample);
    }
    std::sort(result.begin(), result.end(), [](const AxisSample &left,
                                                const AxisSample &right) {
        return left.axisDeg < right.axisDeg;
    });
    return result;
}

QVector<Candidate> findCandidates(const QVector<AxisSample> &input,
                                  double minimumContrast,
                                  double *baselineOut)
{
    const QVector<AxisSample> samples = finiteSortedSamples(input);
    if (samples.size() < 5)
        return {};

    QVector<double> sortedValues;
    sortedValues.reserve(samples.size());
    for (const AxisSample &sample : samples)
        sortedValues.append(sample.powerDb);
    std::sort(sortedValues.begin(), sortedValues.end());
    const double baseline = sortedValues.at(sortedValues.size() / 2);
    if (baselineOut)
        *baselineOut = baseline;

    QVector<double> smooth(samples.size());
    for (int index = 0; index < samples.size(); ++index) {
        double linearSum = 0.0;
        int count = 0;
        for (int neighbour = qMax(0, index - 2);
             neighbour <= qMin(samples.size() - 1, index + 2); ++neighbour) {
            linearSum += linearPower(samples.at(neighbour).powerDb);
            ++count;
        }
        smooth[index] = powerDb(linearSum / qMax(1, count));
    }

    QVector<Candidate> raw;
    for (int index = 1; index + 1 < samples.size(); ++index) {
        if (smooth.at(index) < smooth.at(index - 1)
                || smooth.at(index) < smooth.at(index + 1)
                || smooth.at(index) - baseline < minimumContrast)
            continue;
        int support = 0;
        for (const AxisSample &sample : samples) {
            if (qAbs(sample.axisDeg - samples.at(index).axisDeg) <= 0.5
                    && sample.powerDb - baseline >= minimumContrast)
                ++support;
        }
        if (support >= 3)
            raw.append({samples.at(index).axisDeg,
                        samples.at(index).powerDb, smooth.at(index)});
    }
    std::sort(raw.begin(), raw.end(), [](const Candidate &left,
                                         const Candidate &right) {
        return left.smooth > right.smooth;
    });

    QVector<Candidate> distinct;
    for (const Candidate &candidate : raw) {
        bool overlaps = false;
        for (const Candidate &accepted : distinct) {
            if (qAbs(candidate.axis - accepted.axis) < 0.35) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps)
            distinct.append(candidate);
    }
    return distinct;
}

PeakResult makePeak(const Candidate &candidate, double baseline,
                    const QString &message)
{
    PeakResult result;
    result.valid = true;
    result.axisDeg = candidate.axis;
    result.powerDb = candidate.smooth;
    result.baselineDb = baseline;
    result.contrastDb = candidate.smooth - baseline;
    result.quality = qBound(0.0, result.contrastDb / 12.0, 1.0);
    result.message = message;
    return result;
}

} // namespace

MetalBallGeometry MetalBallCalibrator::calculateGeometry(
        const MetalBallConfig &config)
{
    MetalBallGeometry result;
    result.sphere = config.uav;
    result.sphere.altitudeM -= qMax(0.0, config.ropeLengthM);

    const double frequencyHz = config.frequencyGHz * 1.0e9;
    result.wavelengthM = frequencyHz > 0.0
            ? kSpeedOfLightMps / frequencyHz : 0.0;
    const double radiusM = qMax(0.0, config.sphereDiameterCm) / 200.0;
    result.rcsSquareM = M_PI * radiusM * radiusM;
    result.rcsDbsm = result.rcsSquareM > 0.0
            ? 10.0 * std::log10(result.rcsSquareM)
            : std::numeric_limits<double>::quiet_NaN();
    result.opticalRatio = result.wavelengthM > 0.0
            ? 2.0 * M_PI * radiusM / result.wavelengthM : 0.0;
    result.horizontalRangeM = horizontalDistance(config.radar, result.sphere);
    result.slantRangeM = slantDistance(config.radar, result.sphere);
    result.azimuthDeg = bearing(config.radar, result.sphere);
    result.elevationDeg = elevationAngle(config.radar, result.sphere);
    result.uavElevationDeg = elevationAngle(config.radar, config.uav);
    result.farFieldDistanceM = result.wavelengthM > 0.0
            ? 2.0 * config.antennaApertureM * config.antennaApertureM
              / result.wavelengthM : 0.0;

    const double estimatedBeamwidth = result.wavelengthM > 0.0
            && config.antennaApertureM > 0.0
            ? qRadiansToDegrees(kCircularAntennaHpbwFactor
                                * result.wavelengthM / config.antennaApertureM)
            : 0.0;
    result.azimuthBeamwidthDeg = config.azimuthBeamwidthDeg > 0.0
            ? config.azimuthBeamwidthDeg : estimatedBeamwidth;
    result.elevationBeamwidthDeg = config.elevationBeamwidthDeg > 0.0
            ? config.elevationBeamwidthDeg : estimatedBeamwidth;

    const double gateLengthM = kSpeedOfLightMps
            * qMax(0.0, config.pulseWidthUs) * 1.0e-6 / 2.0;
    const double azimuthBeamwidthRad = qDegreesToRadians(result.azimuthBeamwidthDeg);
    const double elevationBeamwidthRad = qDegreesToRadians(result.elevationBeamwidthDeg);
    if (result.rcsSquareM > 0.0 && result.slantRangeM > 0.0
            && azimuthBeamwidthRad > 0.0 && elevationBeamwidthRad > 0.0
            && gateLengthM > 0.0 && config.waterDielectricFactor > 0.0) {
        result.pulseVolumeCubicM = M_PI * azimuthBeamwidthRad
                * elevationBeamwidthRad * gateLengthM
                * result.slantRangeM * result.slantRangeM
                / (8.0 * std::log(2.0));
        const double reflectivity = result.rcsSquareM
                * std::pow(result.wavelengthM, 4.0) * 1.0e18
                / (std::pow(M_PI, 5.0) * config.waterDielectricFactor
                   * result.pulseVolumeCubicM);
        if (reflectivity > 0.0)
            result.equivalentReflectivityDbz = 10.0 * std::log10(reflectivity);
    }
    return result;
}

int MetalBallCalibrator::expectedRangeBin(double rangeM,
                                           double gateSpacingM,
                                           int binCount)
{
    if (rangeM <= 0.0 || gateSpacingM <= 0.0 || binCount < 3)
        return -1;
    const int bin = qRound(rangeM / gateSpacingM) - 1;
    return bin > 0 && bin + 1 < binCount ? bin : -1;
}

TripletSample MetalBallCalibrator::extractTriplet(
        double axisDeg, const QVector<double> &radialDb, int targetBin,
        const QDateTime &timestamp)
{
    TripletSample result;
    result.axisDeg = axisDeg;
    result.timestamp = timestamp;
    if (targetBin <= 0 || targetBin + 1 >= radialDb.size())
        return result;
    result.minusOneDb = radialDb.at(targetBin - 1);
    result.centreDb = radialDb.at(targetBin);
    result.plusOneDb = radialDb.at(targetBin + 1);
    return result;
}

PeakResult MetalBallCalibrator::selectRhiSpherePeak(
        const QVector<AxisSample> &samples, const RhiPeakOptions &options,
        double *uavPeakDeg)
{
    if (uavPeakDeg)
        *uavPeakDeg = std::numeric_limits<double>::quiet_NaN();
    double baseline = std::numeric_limits<double>::quiet_NaN();
    const QVector<Candidate> peaks = findCandidates(
                samples, options.minimumContrastDb, &baseline);
    if (peaks.isEmpty()) {
        PeakResult result;
        result.baselineDb = baseline;
        result.message = QStringLiteral("No stable RHI peak above the configured background threshold");
        return result;
    }

    const double separation = qAbs(options.uavTheoryDeg - options.sphereTheoryDeg);
    const double tolerance = qBound(0.8, separation * 0.75 + 0.5, 2.5);
    int sphereIndex = -1;
    int uavIndex = -1;
    double sphereDistance = std::numeric_limits<double>::max();
    double uavDistance = std::numeric_limits<double>::max();
    for (int index = 0; index < peaks.size(); ++index) {
        const double toSphere = qAbs(peaks.at(index).axis - options.sphereTheoryDeg);
        const double toUav = qAbs(peaks.at(index).axis - options.uavTheoryDeg);
        if (toSphere <= tolerance && toSphere + 0.20 < toUav
                && toSphere < sphereDistance) {
            sphereDistance = toSphere;
            sphereIndex = index;
        }
        if (toUav <= tolerance && toUav + 0.20 < toSphere
                && toUav < uavDistance) {
            uavDistance = toUav;
            uavIndex = index;
        }
    }
    if (uavIndex >= 0 && uavPeakDeg)
        *uavPeakDeg = peaks.at(uavIndex).axis;

    const bool unresolvedByBeam = separation
            <= qMax(0.35, options.verticalBeamwidthDeg);
    const bool dominant = peaks.size() == 1
            || peaks.first().smooth - peaks.at(1).smooth >= 6.0;
    if (sphereIndex < 0 && dominant && unresolvedByBeam
            && qAbs(peaks.first().axis - options.sphereTheoryDeg) <= tolerance) {
        return makePeak(peaks.first(), baseline,
                        QStringLiteral("Sphere and carrier are unresolved; selected the dominant peak near the sphere theory angle"));
    }
    if (sphereIndex < 0) {
        PeakResult result;
        result.baselineDb = baseline;
        result.message = QStringLiteral("Detected peaks cannot be associated reliably with the sphere");
        return result;
    }
    return makePeak(peaks.at(sphereIndex), baseline,
                    uavIndex >= 0
                    ? QStringLiteral("Sphere peak selected; carrier peak also detected")
                    : QStringLiteral("Sphere peak selected; a second carrier peak is not required"));
}

PeakResult MetalBallCalibrator::selectPpiPeak(
        const QVector<AxisSample> &samples, const PpiPeakOptions &options)
{
    QVector<AxisSample> unwrapped;
    unwrapped.reserve(samples.size());
    for (const AxisSample &sample : samples) {
        const double offset = wrappedAzimuthError(sample.axisDeg, options.theoryDeg);
        if (qAbs(offset) <= options.halfWidthDeg) {
            AxisSample copy = sample;
            copy.axisDeg = offset;
            unwrapped.append(copy);
        }
    }
    double baseline = std::numeric_limits<double>::quiet_NaN();
    const QVector<Candidate> peaks = findCandidates(
                unwrapped, options.minimumContrastDb, &baseline);
    if (peaks.isEmpty()) {
        PeakResult result;
        result.baselineDb = baseline;
        result.message = QStringLiteral("No stable PPI peak inside the search sector");
        return result;
    }

    int bestIndex = 0;
    double bestScore = -std::numeric_limits<double>::max();
    for (int index = 0; index < peaks.size(); ++index) {
        const Candidate &candidate = peaks.at(index);
        const double score = candidate.smooth - 0.05 * qAbs(candidate.axis);
        if (score > bestScore) {
            bestScore = score;
            bestIndex = index;
        }
    }
    Candidate selected = peaks.at(bestIndex);
    selected.axis = normalizeAzimuth(options.theoryDeg + selected.axis);
    return makePeak(selected, baseline,
                    QStringLiteral("Strongest stable PPI peak inside the theoretical search sector selected"));
}

} // namespace radarcal

