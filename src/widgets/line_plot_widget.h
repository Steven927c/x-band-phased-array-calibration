#pragma once

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QWidget>

namespace radarcal {

struct PlotSeries {
    QString name;
    QColor color;
    QVector<QPointF> points;
};

class LinePlotWidget : public QWidget
{
public:
    explicit LinePlotWidget(QWidget *parent = nullptr);
    void setSeries(const QVector<PlotSeries> &series);
    void setAxisTitles(const QString &xTitle, const QString &yTitle);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<PlotSeries> m_series;
    QString m_xTitle;
    QString m_yTitle;
};

} // namespace radarcal

