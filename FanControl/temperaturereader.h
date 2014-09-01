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

#ifndef TEMPERATUREREADER_H
#define TEMPERATUREREADER_H

#include <QList>
#include <QString>

#define HOT_TEMPERATURE  85.0
#define WARM_TEMPERATURE 65.0

struct TemperatureUpdate;
struct SensorTemperature;

class TemperatureReader {
public:
    ///
    /// \brief Get the CPU and GPU temperatures.
    /// \param extended Whether to also get temperatures from other valid sensors.
    /// \return The temperature data.
    ///
    static TemperatureUpdate getTemps(bool extended = false);

private:
    TemperatureReader();

    static QList<SensorTemperature> temperatures;
};

struct TemperatureUpdate {
    double cpu, gpu;
    QList<SensorTemperature> *sensors;
};

struct SensorTemperature {
    QString name;
    char *key;
    double value;
};

#endif // TEMPERATUREREADER_H
