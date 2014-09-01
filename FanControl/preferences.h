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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>

typedef int RPM;
typedef double Temp;

enum UIMode;
enum TempControlMode;

class Preferences {
public:
    static void init();
    static void write();
    static bool isFirstRun();

    static UIMode mode();
    static bool linked();
    static TempControlMode leftFanControl();
    static TempControlMode rightFanControl();
    static RPM autoLeftMin();
    static RPM autoLeftMax();
    static RPM autoRightMin();
    static RPM autoRightMax();
    static RPM manualLeft();
    static RPM manualRight();
    static Temp lowerTempThresh();
    static Temp upperTempThresh();
    static bool useCelsius();
    static bool heatWarningsEnabled();
    static int heatWarningsInterval();
    static Temp heatWarningsThreshold();

    static void setMode(UIMode value);
    static void setLinked(bool value);
    static void setLeftFanControl(TempControlMode value);
    static void setRightFanControl(TempControlMode value);
    static void setAutoLeftMin(RPM value);
    static void setAutoLeftMax(RPM value);
    static void setAutoRightMin(RPM value);
    static void setAutoRightMax(RPM value);
    static void setManualLeft(RPM value);
    static void setManualRight(RPM value);
    static void setLowerTempThresh(Temp value);
    static void setUpperTempThresh(Temp value);
    static void setUseCelsius(bool value);
    static void setHeatWarningsEnabled(bool value);
    static void setHeatWarningsInterval(int value);
    static void setHeatWarningsThreshold(Temp value);

    ///
    /// \brief Converts Celsius to Fahrenheit if necessary and returns a QString
    /// to the specified precision and with unit appended if specified.
    /// \param c The Celsius temperature.
    /// \param precision The number of decimal places to output
    /// \param showUnit Whether to output the unit ("C" or "F").
    /// \return The temperature string, or invalid if the temperature is <= 0.
    ///
    static QString tempStr(double c, int precision = 1, bool showUnit = true);

private:
    Preferences() {}
    static bool _isFirstRun;
};

enum UIMode {
    Auto, Manual
};

enum TempControlMode {
    CPU, GPU, MaxBoth
};

#endif // PREFERENCES_H
