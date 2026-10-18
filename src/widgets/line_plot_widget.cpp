#include "line_plot_widget.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <limits>

namespace radarcal {

LinePlotWidget::LinePlotWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(260);
    setAutoFillBackground(true);
}

void LinePlotWidget::setSeries(const QVector<PlotSeries> &series)
{
    m_series = series;
    update();
}

void LinePlotWidget::setAxisTitles(const QString &xTitle, const QString &yTitle)
{
    m_xTitle = xTitle;
    m_yTitle = yTitle;
    update();
}

void LinePlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(250, 252, 255));
    const QRectF area = rect().adjusted(64, 18, -130, -48);
    painter.setPen(QColor(90, 105, 125));
    painter.drawLine(area.bottomLeft(), area.topLeft());
    painter.drawLine(area.bottomLeft(), area.bottomRight());

    double xMin = std::numeric_limits<double>::max();
    double xMax = -std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double yMax = -std::numeric_limits<double>::max();
    for (const PlotSeries &series : m_series) {
        for (const QPointF &point : series.points) {
            xMin = qMin(xMin, point.x());
            xMax = qMax(xMax, point.x());
            yMin = qMin(yMin, point.y());
            yMax = qMax(yMax, point.y());
        }
    }
    if (!(xMax > xMin) || !(yMax > yMin)) {
        painter.drawText(area, Qt::AlignCenter, tr("No samples"));
        return;
    }
    const double yMargin = qMax(0.5, (yMax - yMin) * 0.08);
    yMin -= yMargin;
    yMax += yMargin;
    const auto mapPoint = [&](const QPointF &point) {
        return QPointF(area.left() + (point.x() - xMin) / (xMax - xMin) * area.width(),
                       area.bottom() - (point.y() - yMin) / (yMax - yMin) * area.height());
    };

    painter.setPen(QPen(QColor(220, 226, 235), 1, Qt::DashLine));
    for (int index = 1; index < 5; ++index) {
        const double y = area.top() + area.height() * index / 5.0;
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }
    for (int index = 0; index < m_series.size(); ++index) {
        const PlotSeries &series = m_series.at(index);
        if (series.points.isEmpty())
            continue;
        QPainterPath path(mapPoint(series.points.first()));
        for (int pointIndex = 1; pointIndex < series.points.size(); ++pointIndex)
            path.lineTo(mapPoint(series.points.at(pointIndex)));
        painter.setPen(QPen(series.color, 2));
        painter.drawPath(path);
        painter.drawText(QPointF(area.right() + 16, area.top() + 20 + index * 22),
                         series.name);
    }
    painter.setPen(QColor(40, 55, 75));
    painter.drawText(QRectF(area.left(), area.bottom() + 12, area.width(), 24),
                     Qt::AlignCenter, m_xTitle);
    painter.save();
    painter.translate(18, area.center().y());
    painter.rotate(-90);
    painter.drawText(QRectF(-area.height() / 2, -12, area.height(), 24),
                     Qt::AlignCenter, m_yTitle);
    painter.restore();
    painter.drawText(QPointF(area.left(), area.bottom() + 16), QString::number(xMin, 'f', 1));
    painter.drawText(QPointF(area.right() - 25, area.bottom() + 16), QString::number(xMax, 'f', 1));
    painter.drawText(QPointF(area.left() - 48, area.bottom()), QString::number(yMin, 'f', 1));
    painter.drawText(QPointF(area.left() - 48, area.top() + 5), QString::number(yMax, 'f', 1));
}

} // namespace radarcal

