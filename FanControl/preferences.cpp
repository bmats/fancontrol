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

#include "preferences.h"
#include "debug.h"

#include <QSettings>

#define MODE_KEY                   QString("FanControl/mode")
#define LINKED_KEY                 QString("FanControl/linked")
#define LEFT_FAN_CONTROL_KEY       QString("FanControl/leftFanControl")
#define RIGHT_FAN_CONTROL_KEY      QString("FanControl/rightFanControl")
#define AUTO_LEFT_MIN_KEY          QString("FanControl/autoLeftMin")
#define AUTO_LEFT_MAX_KEY          QString("FanControl/autoLeftMax")
#define AUTO_RIGHT_MIN_KEY         QString("FanControl/autoRightMin")
#define AUTO_RIGHT_MAX_KEY         QString("FanControl/autoRightMax")
#define MANUAL_LEFT_KEY            QString("FanControl/manualLeft")
#define MANUAL_RIGHT_KEY           QString("FanControl/manualRight")
#define LOWER_TEMP_THRESH_KEY      QString("FanControl/lowerTempThresh")
#define UPPER_TEMP_THRESH_KEY      QString("FanControl/upperTempThresh")
#define USE_CELSIUS_KEY            QString("FanControl/useCelsius")
#define HEAT_WARNINGS_ENABLED_KEY  QString("FanControl/heatWarningsEnabled")
#define HEAT_WARNINGS_INTERVAL_KEY QString("FanControl/heatWarningsInterval")
#define HEAT_WARNINGS_THRESH_KEY   QString("FanControl/heatWarningsThreshold")

// Use a relative path so the .ini sits alonside the executable
#define PREFS_FILE "preferences.ini"
static QSettings settings(QString(PREFS_FILE), QSettings::IniFormat);

bool Preferences::_isFirstRun;

void Preferences::init() {
    // Check if this is the first run
    _isFirstRun = !settings.contains(MODE_KEY);

    // Set default values
    if (_isFirstRun)                                       setMode(Auto);
    if (!settings.contains(LINKED_KEY))                 setLinked(true);
    if (!settings.contains(LEFT_FAN_CONTROL_KEY))       setLeftFanControl(CPU);
    if (!settings.contains(RIGHT_FAN_CONTROL_KEY))      setRightFanControl(GPU);
    if (!settings.contains(AUTO_LEFT_MIN_KEY))          setAutoLeftMin(1500); // rpm
    if (!settings.contains(AUTO_LEFT_MAX_KEY))          setAutoLeftMax(6000); // rpm
    if (!settings.contains(AUTO_RIGHT_MIN_KEY))         setAutoRightMin(1500); // rpm
    if (!settings.contains(AUTO_RIGHT_MAX_KEY))         setAutoRightMax(6000); // rpm
    if (!settings.contains(MANUAL_LEFT_KEY))            setManualLeft(4000);  // rpm
    if (!settings.contains(MANUAL_RIGHT_KEY))           setManualRight(4000); // rpm
    if (!settings.contains(LOWER_TEMP_THRESH_KEY))      setLowerTempThresh(30); // degrees
    if (!settings.contains(UPPER_TEMP_THRESH_KEY))      setUpperTempThresh(85); // degrees
    if (!settings.contains(USE_CELSIUS_KEY))            setUseCelsius(true);
    if (!settings.contains(HEAT_WARNINGS_ENABLED_KEY))  setHeatWarningsEnabled(true); // degrees
    if (!settings.contains(HEAT_WARNINGS_INTERVAL_KEY)) setHeatWarningsInterval(60); // seconds
    if (!settings.contains(HEAT_WARNINGS_THRESH_KEY))   setHeatWarningsThreshold(85); // degrees
}

void Preferences::write() {
    settings.sync();
}

bool Preferences::isFirstRun() {
    return _isFirstRun;
}

UIMode Preferences::mode()                     { return (UIMode)settings.value(MODE_KEY).toInt(); }
bool Preferences::linked()                     { return settings.value(LINKED_KEY).toBool(); }
TempControlMode Preferences::leftFanControl()  { return (TempControlMode)settings.value(LEFT_FAN_CONTROL_KEY).toInt(); }
TempControlMode Preferences::rightFanControl() { return (TempControlMode)settings.value(RIGHT_FAN_CONTROL_KEY).toInt(); }
RPM Preferences::autoLeftMin()                 { return settings.value(AUTO_LEFT_MIN_KEY).toInt(); }
RPM Preferences::autoLeftMax()                 { return settings.value(AUTO_LEFT_MAX_KEY).toInt(); }
RPM Preferences::autoRightMin()                { return settings.value(AUTO_RIGHT_MIN_KEY).toInt(); }
RPM Preferences::autoRightMax()                { return settings.value(AUTO_RIGHT_MAX_KEY).toInt(); }
RPM Preferences::manualLeft()                  { return settings.value(MANUAL_LEFT_KEY).toInt(); }
RPM Preferences::manualRight()                 { return settings.value(MANUAL_RIGHT_KEY).toInt(); }
Temp Preferences::lowerTempThresh()            { return settings.value(LOWER_TEMP_THRESH_KEY).toDouble(); }
Temp Preferences::upperTempThresh()            { return settings.value(UPPER_TEMP_THRESH_KEY).toDouble(); }
bool Preferences::useCelsius()                 { return settings.value(USE_CELSIUS_KEY).toBool(); }
bool Preferences::heatWarningsEnabled()        { return settings.value(HEAT_WARNINGS_ENABLED_KEY).toBool(); }
int Preferences::heatWarningsInterval()        { return settings.value(HEAT_WARNINGS_INTERVAL_KEY).toInt(); }
Temp Preferences::heatWarningsThreshold()      { return settings.value(HEAT_WARNINGS_THRESH_KEY).toDouble(); }

void Preferences::setMode(UIMode value)                     { settings.setValue(MODE_KEY,                   QVariant(value)); }
void Preferences::setLinked(bool value)                     { settings.setValue(LINKED_KEY,                 QVariant(value)); }
void Preferences::setLeftFanControl(TempControlMode value)  { settings.setValue(LEFT_FAN_CONTROL_KEY,       QVariant(value)); }
void Preferences::setRightFanControl(TempControlMode value) { settings.setValue(RIGHT_FAN_CONTROL_KEY,      QVariant(value)); }
void Preferences::setAutoLeftMin(RPM value)                 { settings.setValue(AUTO_LEFT_MIN_KEY,          QVariant(value)); }
void Preferences::setAutoLeftMax(RPM value)                 { settings.setValue(AUTO_LEFT_MAX_KEY,          QVariant(value)); }
void Preferences::setAutoRightMin(RPM value)                { settings.setValue(AUTO_RIGHT_MIN_KEY,         QVariant(value)); }
void Preferences::setAutoRightMax(RPM value)                { settings.setValue(AUTO_RIGHT_MAX_KEY,         QVariant(value)); }
void Preferences::setManualLeft(RPM value)                  { settings.setValue(MANUAL_LEFT_KEY,            QVariant(value)); }
void Preferences::setManualRight(RPM value)                 { settings.setValue(MANUAL_RIGHT_KEY,           QVariant(value)); }
void Preferences::setLowerTempThresh(Temp value)            { settings.setValue(LOWER_TEMP_THRESH_KEY,      QVariant(value)); }
void Preferences::setUpperTempThresh(Temp value)            { settings.setValue(UPPER_TEMP_THRESH_KEY,      QVariant(value)); }
void Preferences::setUseCelsius(bool value)                 { settings.setValue(USE_CELSIUS_KEY,            QVariant(value)); }
void Preferences::setHeatWarningsEnabled(bool value)        { settings.setValue(HEAT_WARNINGS_ENABLED_KEY,  QVariant(value)); }
void Preferences::setHeatWarningsInterval(int value)        { settings.setValue(HEAT_WARNINGS_INTERVAL_KEY, QVariant(value)); }
void Preferences::setHeatWarningsThreshold(Temp value)      { settings.setValue(HEAT_WARNINGS_THRESH_KEY,   QVariant(value)); }

QString Preferences::tempStr(double c, int precision, bool showUnit) {
    if (c <= 0) return QString("invalid");

    bool useC = useCelsius();
    if (!useC) c = c * 1.8 + 32.0; // convert to F

    if (showUnit) return QString("%1° %2").arg(c, 0, 'f', precision).arg(useC ? 'C' : 'F');
    return QString("%1°").arg(c, 0, 'f', precision);
}
