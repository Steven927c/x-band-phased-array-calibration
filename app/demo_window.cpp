#include "demo_window.h"

#include "core/calibration_math.h"
#include "core/metal_ball_calibrator.h"
#include "core/solar_calibrator.h"
#include "widgets/line_plot_widget.h"

#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTabWidget>
#include <QVBoxLayout>

#include <cmath>

namespace radarcal {
namespace {

QDoubleSpinBox *spin(double minimum, double maximum, double value,
                     int decimals = 5)
{
    auto *editor = new QDoubleSpinBox;
    editor->setRange(minimum, maximum);
    editor->setDecimals(decimals);
    editor->setValue(value);
    editor->setKeyboardTracking(false);
    return editor;
}

double gaussian(double x, double centre, double sigma, double amplitude)
{
    const double normalized = (x - centre) / sigma;
    return amplitude * std::exp(-0.5 * normalized * normalized);
}

} // namespace

DemoWindow::DemoWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("X波段相控阵金属球及太阳法标定"));
    resize(1080, 720);
    auto *tabs = new QTabWidget;
    tabs->addTab(buildMetalBallPage(), tr("Metal-ball calibration"));
    tabs->addTab(buildSolarPage(), tr("Solar calibration"));
    auto *layout = new QVBoxLayout(this);
    auto *notice = new QLabel(tr("Demonstration data are synthetic. Replace the device adapter to ingest real radar observations."));
    notice->setStyleSheet("padding:8px;background:#fff4d6;color:#6a4c00;border-radius:4px;");
    layout->addWidget(notice);
    layout->addWidget(tabs, 1);
    updateMetalBallDemo();
    updateSolarDemo();
}

QWidget *DemoWindow::buildMetalBallPage()
{
    auto *page = new QWidget;
    auto *layout = new QHBoxLayout(page);
    auto *formBox = new QGroupBox(tr("Geometry"));
    auto *form = new QFormLayout(formBox);
    m_radarLat = spin(-90, 90, 30.50474);
    m_radarLon = spin(-180, 180, 103.95364);
    m_radarAlt = spin(-500, 9000, 502.2, 1);
    m_targetLat = spin(-90, 90, 30.51350);
    m_targetLon = spin(-180, 180, 103.95364);
    m_targetAlt = spin(-500, 9000, 620.0, 1);
    m_ropeLength = spin(0, 500, 20.0, 1);
    m_sphereDiameter = spin(0, 200, 30.0, 1);
    form->addRow(tr("Radar latitude (deg)"), m_radarLat);
    form->addRow(tr("Radar longitude (deg)"), m_radarLon);
    form->addRow(tr("Radar altitude (m)"), m_radarAlt);
    form->addRow(tr("Carrier latitude (deg)"), m_targetLat);
    form->addRow(tr("Carrier longitude (deg)"), m_targetLon);
    form->addRow(tr("Carrier altitude (m)"), m_targetAlt);
    form->addRow(tr("Rope length (m)"), m_ropeLength);
    form->addRow(tr("Sphere diameter (cm)"), m_sphereDiameter);
    auto *calculate = new QPushButton(tr("Calculate and demonstrate peak selection"));
    m_metalResult = new QLabel;
    m_metalResult->setWordWrap(true);
    form->addRow(calculate);
    form->addRow(m_metalResult);
    layout->addWidget(formBox);
    m_metalPlot = new LinePlotWidget;
    m_metalPlot->setAxisTitles(tr("Elevation (deg)"), tr("Relative power (dB)"));
    layout->addWidget(m_metalPlot, 1);
    connect(calculate, &QPushButton::clicked, this, [this] { updateMetalBallDemo(); });
    return page;
}

QWidget *DemoWindow::buildSolarPage()
{
    auto *page = new QWidget;
    auto *layout = new QHBoxLayout(page);
    auto *formBox = new QGroupBox(tr("Station and time"));
    auto *form = new QFormLayout(formBox);
    m_solarLat = spin(-90, 90, 30.50474);
    m_solarLon = spin(-180, 180, 103.95364);
    m_solarAlt = spin(-500, 9000, 502.2, 1);
    m_solarTime = new QDateTimeEdit(QDateTime::currentDateTime());
    m_solarTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss t");
    m_solarTime->setCalendarPopup(true);
    form->addRow(tr("Latitude (deg)"), m_solarLat);
    form->addRow(tr("Longitude (deg)"), m_solarLon);
    form->addRow(tr("Altitude (m)"), m_solarAlt);
    form->addRow(tr("Local time"), m_solarTime);
    auto *calculate = new QPushButton(tr("Calculate and demonstrate fitting"));
    m_solarResult = new QLabel;
    m_solarResult->setWordWrap(true);
    form->addRow(calculate);
    form->addRow(m_solarResult);
    layout->addWidget(formBox);
    m_solarPlot = new LinePlotWidget;
    m_solarPlot->setAxisTitles(tr("Azimuth (deg)"), tr("Relative power (dB)"));
    layout->addWidget(m_solarPlot, 1);
    connect(calculate, &QPushButton::clicked, this, [this] { updateSolarDemo(); });
    return page;
}

void DemoWindow::updateMetalBallDemo()
{
    MetalBallConfig config;
    config.radar = {m_radarLat->value(), m_radarLon->value(), m_radarAlt->value()};
    config.uav = {m_targetLat->value(), m_targetLon->value(), m_targetAlt->value()};
    config.ropeLengthM = m_ropeLength->value();
    config.sphereDiameterCm = m_sphereDiameter->value();
    config.frequencyGHz = 9.4;
    config.antennaApertureM = 1.0;
    config.pulseWidthUs = 1.0;
    const MetalBallGeometry geometry = MetalBallCalibrator::calculateGeometry(config);

    QVector<AxisSample> samples;
    PlotSeries centre{tr("T"), QColor(235, 112, 31), {}};
    PlotSeries minus{tr("T-1"), QColor(47, 155, 91), {}};
    PlotSeries plus{tr("T+1"), QColor(55, 91, 145), {}};
    const double low = geometry.elevationDeg - 5.0;
    for (int index = 0; index <= 100; ++index) {
        const double axis = low + index * 0.1;
        const double noise = QRandomGenerator::global()->generateDouble() * 0.7 - 0.35;
        const double centreDb = 3.0 + gaussian(axis, geometry.elevationDeg, 0.65, 31.0)
                + gaussian(axis, geometry.uavElevationDeg, 0.55, 24.0) + noise;
        const double minusDb = centreDb - 1.2 + noise * 0.3;
        const double plusDb = centreDb - 1.6 - noise * 0.2;
        samples.append({axis, centreDb, {}});
        centre.points.append({axis, centreDb});
        minus.points.append({axis, minusDb});
        plus.points.append({axis, plusDb});
    }
    RhiPeakOptions options;
    options.sphereTheoryDeg = geometry.elevationDeg;
    options.uavTheoryDeg = geometry.uavElevationDeg;
    options.verticalBeamwidthDeg = geometry.elevationBeamwidthDeg;
    const PeakResult peak = MetalBallCalibrator::selectRhiSpherePeak(samples, options);
    m_metalPlot->setSeries({centre, minus, plus});
    m_metalResult->setText(tr("Range: %1 km\nTheory AZ/EL: %2 deg / %3 deg\nRCS: %4 dBsm\nEXP: %5 dBZ\nSelected RHI peak: %6")
            .arg(geometry.slantRangeM / 1000.0, 0, 'f', 3)
            .arg(geometry.azimuthDeg, 0, 'f', 2)
            .arg(geometry.elevationDeg, 0, 'f', 2)
            .arg(geometry.rcsDbsm, 0, 'f', 2)
            .arg(geometry.equivalentReflectivityDbz, 0, 'f', 2)
            .arg(peak.valid ? QString::number(peak.axisDeg, 'f', 2) + " deg"
                            : peak.message));
}

void DemoWindow::updateSolarDemo()
{
    const GeoPoint station{m_solarLat->value(), m_solarLon->value(),
                           m_solarAlt->value()};
    const SolarCoordinates sun = SolarCalibrator::calculatePosition(
                m_solarTime->dateTime(), station);
    const double simulatedBias = 1.25;
    QVector<SolarFitSample> samples;
    PlotSeries power{tr("T relative power"), QColor(235, 112, 31), {}};
    for (int index = -30; index <= 30; ++index) {
        const double axis = sun.azimuthDeg + index * 0.2;
        const double noise = QRandomGenerator::global()->generateDouble() * 0.04 - 0.02;
        const double value = -0.2 + gaussian(axis, sun.azimuthDeg + simulatedBias,
                                             1.0, 1.1) + noise;
        samples.append({normalizeAzimuth(axis), sun.azimuthDeg, value, {}});
        power.points.append({axis, value});
    }
    SolarFitOptions options;
    options.azimuthAxis = true;
    options.scanCentreDeg = sun.azimuthDeg;
    options.scanHalfRangeDeg = 6.0;
    const SolarFitResult fit = SolarCalibrator::fitPeak(samples, options);
    m_solarPlot->setSeries({power});
    m_solarResult->setText(tr("Sun AZ/EL: %1 deg / %2 deg\nFitted AZ: %3 deg\nPointing error: %4 deg\nRecommended correction: %5 deg")
            .arg(sun.azimuthDeg, 0, 'f', 2)
            .arg(sun.elevationDeg, 0, 'f', 2)
            .arg(fit.peak.axisDeg, 0, 'f', 2)
            .arg(fit.pointingErrorDeg, 0, 'f', 3)
            .arg(fit.recommendedCorrectionDeg, 0, 'f', 3));
}

} // namespace radarcal

