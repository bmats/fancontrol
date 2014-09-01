// Fan Control
// Copyright (C) 2014 Bryce
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "fanspeedwidget.h"
#include "debug.h"
#include <QPainter>
#include <QPointF>

#define MIN_RPM 1200
#define MAX_RPM 6000

#define SLIDER_HANDLE_MARGIN 5
#define ARROW_TIP_WIDTH 8.0f
#define LIMIT_ARROW_OFFSET 5.0

#define LIMIT_FILL_COLOR   QColor(240, 240, 240)
#define LIMIT_BORDER_COLOR QColor(200, 200, 200)
#define SPEED_FILL_COLOR   QColor(217, 230, 255)
#define SPEED_BORDER_COLOR QColor(124, 161, 207)

FanSpeedWidget::FanSpeedWidget(QWidget *parent) :
    QWidget(parent),
    textPen(QPen(QBrush(QColor(0, 0, 0)), 2)),
    limitArrowBrush(QBrush(LIMIT_FILL_COLOR)),
    speedArrowBrush(QBrush(SPEED_FILL_COLOR)),
    limitArrowPen(QPen(QBrush(LIMIT_BORDER_COLOR), 0)),
    speedArrowPen(QPen(QBrush(SPEED_BORDER_COLOR), 0)),
    font(QFont(QString("Segoe UI"), 9)),
    fm(QFontMetrics(font)) {

    direction = Left;
    ranged = true;
    minValue = MIN_RPM;
    maxValue = MAX_RPM;
}

void FanSpeedWidget::setDirection(Direction d) {
    direction = d;
}

void FanSpeedWidget::setValue(int value) {
    ranged = false;
    maxValue = value; // just use maxValue as the single value
    repaint();
}

void FanSpeedWidget::setValueRange(int lower, int upper) {
    ranged = true;
    minValue = lower;
    maxValue = upper;
    repaint();
}

void FanSpeedWidget::setActualFanSpeed(int speed) {
    fanSpeed = speed;
    repaint();
}

void FanSpeedWidget::setVerticalMargins(int top, int bottom) {
    topMargin = top;
    bottomMargin = bottom;
}

void FanSpeedWidget::paintEvent(QPaintEvent *) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setFont(font);

    painter.setPen(speedArrowPen);
    painter.setBrush(speedArrowBrush);
    paintArrowText(&painter, fanSpeed);

    // Move the limiting arrows off to the side a bit
    painter.translate(direction == Left ? -LIMIT_ARROW_OFFSET : LIMIT_ARROW_OFFSET, 0.0);

    painter.setPen(limitArrowPen);
    painter.setBrush(limitArrowBrush);
    if (ranged) {
        paintArrowText(&painter, minValue);

        painter.setPen(limitArrowPen);
        painter.setBrush(limitArrowBrush);
        paintArrowText(&painter, maxValue);
    }
    else {
        paintArrowText(&painter, maxValue);
    }

    painter.end();
}

void FanSpeedWidget::paintArrowText(QPainter *painter, int value) {
    float y = topMargin +
              SLIDER_HANDLE_MARGIN +
              (MAX_RPM - (float)value) / (MAX_RPM - MIN_RPM) * (height() - topMargin - bottomMargin - SLIDER_HANDLE_MARGIN * 2);
    QString text = QString::number(value);
    int textWidth = fm.width(text);

    QPainterPath *arrow = makeArrowPath(y, textWidth, fm.height() + 4);
    painter->drawPath(*arrow);
    delete arrow;

    painter->setPen(textPen);
    if (direction == Right) {
        painter->drawText(QPointF(12, y + 4), text);
    }
    else {
        painter->drawText(QPointF(width() - textWidth - 8 - 2, y + 4), text);
    }
}

QPainterPath *FanSpeedWidget::makeArrowPath(float y, int width, int height) {
    QPainterPath *path = new QPainterPath();
    float fullWidth = width + ARROW_TIP_WIDTH + 4 + 4;

    if (direction == Right) {
        float x = 1.0f;
        path->moveTo(x, y);
        path->lineTo(x + ARROW_TIP_WIDTH,  y - (height * 0.5f));
        path->lineTo(x + fullWidth - 2.0f, y - (height * 0.5f));
        path->lineTo(x + fullWidth,        y - (height * 0.5f) + 2.0f);
        path->lineTo(x + fullWidth,        y + (height * 0.5f) - 2.0f);
        path->lineTo(x + fullWidth - 2.0f, y + (height * 0.5f));
        path->lineTo(x + ARROW_TIP_WIDTH,  y + (height * 0.5f));
        path->lineTo(x, y);
    }
    else {
        float x = this->width() - fullWidth;
        path->moveTo(x + fullWidth, y);
        path->lineTo(x + fullWidth - ARROW_TIP_WIDTH, y - (height * 0.5f));
        path->lineTo(x + 2.0f,                        y - (height * 0.5f));
        path->lineTo(x,                               y - (height * 0.5f) + 2.0f);
        path->lineTo(x,                               y + (height * 0.5f) - 2.0f);
        path->lineTo(x + 2.0f,                        y + (height * 0.5f));
        path->lineTo(x + fullWidth - ARROW_TIP_WIDTH, y + (height * 0.5f));
        path->lineTo(x + fullWidth,                   y);
    }

    return path;
}
