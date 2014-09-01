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
// Contains code from Lubbo's Fan Control.
//

#ifndef SMC_H
#define SMC_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#define TEMPERATURE_ERROR -1.0
#define FAN_ERROR -1

///
/// \brief The System Management Controller
///
class Smc {
public:
    static Smc &getInstance();

    int readKey(const char* key, unsigned char* buffer, int len);
    bool writeKey(const char* key, const unsigned char* buffer, int len);
    int readKeyS(const char* key, short* buffer, int len);
    bool writeKeyS(const char* key, const short* buffer, int len);

    ///
    /// \brief Gets a temperature from a sensor.
    /// \param key The SPM key for the sensor.
    /// \return The temperature, in celsius.
    ///
    double getTemperature(char key[]);

    ///
    /// \brief Gets the number of fans.
    /// \return The fan count.
    ///
    int getFanCount();

    ///
    /// \brief Gets the RPM of a fan. If something goes wrong, it returns FAN_ERROR.
    /// \param key The SMC key for the fan, either SMC_KEY_LEFT_FAN_RPM or SMC_KEY_RIGHT_FAN_RPM.
    /// \return The fan RPM.
    ///
    int getFanRPM(char key[]);

    ///
    /// \brief Sets the RPM of a fan.
    /// \param key The SMC key for the fan, either SMC_KEY_LEFT_FAN_RPM or SMC_KEY_RIGHT_FAN_RPM
    /// \param rpm The RPM.
    /// \return FAN_ERROR on error, otherwise 0.
    ///
    int setFanRPM(char key[], int rpm);

private:
    Smc();
    ~Smc();

    HANDLE m_hPort;
};

#endif // SMC_H
