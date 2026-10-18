#pragma once

#include <QWidget>

class QDateTimeEdit;
class QDoubleSpinBox;
class QLabel;

namespace radarcal {
class LinePlotWidget;

class DemoWindow : public QWidget
{
public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    QWidget *buildMetalBallPage();
    QWidget *buildSolarPage();
    void updateMetalBallDemo();
    void updateSolarDemo();

    QDoubleSpinBox *m_radarLat = nullptr;
    QDoubleSpinBox *m_radarLon = nullptr;
    QDoubleSpinBox *m_radarAlt = nullptr;
    QDoubleSpinBox *m_targetLat = nullptr;
    QDoubleSpinBox *m_targetLon = nullptr;
    QDoubleSpinBox *m_targetAlt = nullptr;
    QDoubleSpinBox *m_ropeLength = nullptr;
    QDoubleSpinBox *m_sphereDiameter = nullptr;
    QLabel *m_metalResult = nullptr;
    LinePlotWidget *m_metalPlot = nullptr;

    QDoubleSpinBox *m_solarLat = nullptr;
    QDoubleSpinBox *m_solarLon = nullptr;
    QDoubleSpinBox *m_solarAlt = nullptr;
    QDateTimeEdit *m_solarTime = nullptr;
    QLabel *m_solarResult = nullptr;
    LinePlotWidget *m_solarPlot = nullptr;
};

} // namespace radarcal

