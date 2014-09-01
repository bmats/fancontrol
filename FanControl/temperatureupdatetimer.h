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

#ifndef TEMPERATUREUPDATETIMER_H
#define TEMPERATUREUPDATETIMER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "temperaturereader.h"

///
/// \brief Allows async temperature updating for MainWindow.
///
class TemperatureUpdateTimer : public QObject {
    Q_OBJECT

public:
    TemperatureUpdateTimer(QObject *parent = 0);
    ~TemperatureUpdateTimer();

    bool extended();
    void setExtended(bool extended);

public slots:
    void start();

signals:
    void updated(TemperatureUpdate update);

private slots:
    void update();

private:
    QTimer *timer;
    QThread *thread;

    bool m_extended = false;
};

Q_DECLARE_METATYPE(TemperatureUpdate)

#endif // TEMPERATUREUPDATETIMER_H
