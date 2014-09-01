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

#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

#include <QObject>
#include <QTimer>
#include "smc.h"
#include "preferences.h"

#define MAX_TEMPERATURE 90.0

//#define FAN_EASING

class FanController : QObject {
    Q_OBJECT

public:
    FanController();
    ~FanController();

    void refreshPreferences();

    int leftRPM();
    int rightRPM();
    bool hasRightFan();

private slots:
    void update();

private:
    int calculateTargetRpm(double controlTemp, int fanRPM, int minRPM, int maxRPM);

    Smc &smc;
    QTimer *timer;

    bool m_hasRightFan;

    UIMode mode;
    TempControlMode leftFanControl, rightFanControl;
    RPM autoLeftMinRPM, autoLeftMaxRPM, autoRightMinRPM, autoRightMaxRPM;
    RPM manualLeft, manualRight;
    Temp lowerTempThresh, upperTempThresh;
};

#endif // FANCONTROLLER_H
