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

#ifndef FANSPEEDWIDGET_H
#define FANSPEEDWIDGET_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QFontMetrics>

class FanSpeedWidget : public QWidget {
    Q_OBJECT
public:
    enum Direction { Left, Right };

    explicit FanSpeedWidget(QWidget *parent = 0);

public slots:
    void setDirection(Direction d);
    void setValue(int value);
    void setValueRange(int lower, int upper);
    void setActualFanSpeed(int speed);
    void setVerticalMargins(int top, int bottom);

protected:
    void paintEvent(QPaintEvent*);

private:
    void paintArrowText(QPainter *painter, int value);
    QPainterPath *makeArrowPath(float y, int width, int height);

    Direction direction;
    bool ranged;
    int minValue, maxValue;
    int fanSpeed;
    int topMargin, bottomMargin;

    QBrush limitArrowBrush, speedArrowBrush;
    QPen textPen, limitArrowPen, speedArrowPen;
    QFont font;
    QFontMetrics fm;
};

#endif // FANSPEEDWIDGET_H
