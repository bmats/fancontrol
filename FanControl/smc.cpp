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

#include "smc.h"
#include "debug.h"

#define SMC_RETRY_INTERVAL 1
#define SMC_RETRY_INTERVAL_LONG 100
#define SMC_RETRY_COUNT    3

#define SMC_FAN_COUNT_KEY "FNum"

Smc &Smc::getInstance() {
    static Smc instance;
    return instance;
}

Smc::Smc() {
    m_hPort = CreateFileA(
        "\\\\.\\MacHALDriver",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
}

Smc::~Smc() {
    CloseHandle(m_hPort);
}

int Smc::readKey(const char* key, unsigned char* buffer, int len) {
    if (m_hPort == 0) return false;

    BOOL success = false;
    DWORD bytesReturned = 0;

    char keyb[5];
    for (int i = 0; i < 4; i++) {
        keyb[i] = key[i];
    }
    keyb[4] = 0;

    success = DeviceIoControl(
        m_hPort,
        0x9C402454, // IOCTRL_SMC_READ_KEY
        reinterpret_cast<void*>(keyb),
        5,
        buffer,
        len,
        &bytesReturned,
        NULL
    );

    return (success) ? static_cast<int>(bytesReturned) : 0;
}

bool Smc::writeKey(const char* key, const unsigned char* buffer, int len) {
    if (m_hPort == 0) return false;

    BOOL success = false;
    DWORD bytesReturned = 0;

    unsigned char* keyb = new unsigned char[5 + len];

    for (int i = 0; i < 4; i++) {
        keyb[i] = static_cast<unsigned char>(key[i]);
    }
    keyb[4] = 0;
    memmove(keyb + 5, buffer, len);

    success = DeviceIoControl(
        m_hPort,
        0x9C402458, // IOCTRL_SMC_WRITE_KEY
        keyb,
        5 + len, // must be 5 + len
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    delete[] keyb;

    return (success != 0); // convert BOOL to bool
}

int Smc::readKeyS(const char* key, short* buffer, int len) {
    unsigned char *bufb = reinterpret_cast<unsigned char*>(buffer);
    int ret = readKey(key, bufb, len);
    for (int i = len - 1; i > 0; i--) {
        bufb[2 * i] = bufb[i];
    }

    for (int i = 0; i < len; ++i) {
        bufb[2 * i + 1] = 0;
    }
    return ret;
}

bool Smc::writeKeyS(const char* key, const short* buffer, int len) {
    unsigned char *bufb = new unsigned char[len];
    for (int i = 0; i < len; ++i) {
        bufb[i] = buffer[i];
    }

    bool ret = writeKey(key, bufb, len);
    delete[] bufb;
    return ret;
}

double Smc::getTemperature(char key[]) {
    unsigned char temp[2];

    int i;
    for (i = 0; i < SMC_RETRY_COUNT; i++) {
        memset(temp, 0, sizeof(temp));

        // Read, break on success
        if (readKey(key, temp, 2) == 2) break;

        // Otherwise wait before trying again
        Sleep(SMC_RETRY_INTERVAL);
    }

    // Error if retried max times
    if (i == SMC_RETRY_COUNT) return TEMPERATURE_ERROR;

    int intValue = (temp[0] * 256 + temp[1]) >> 2;
    return intValue / 64.0;
}

int Smc::getFanCount() {
    short buffer[1];
    int ret = readKeyS(SMC_FAN_COUNT_KEY, buffer, 2);

    return (ret == 0) ? 0 : buffer[0];
}

int Smc::getFanRPM(char key[]) {
    short rpm[2];

    int i;
    for (i = 0; i < SMC_RETRY_COUNT; i++) {
        memset(rpm, 0, sizeof(rpm));
        if (readKeyS(key, rpm, 2) == 2) break;

        Sleep(SMC_RETRY_INTERVAL);
    }

    // Error if retried max times
    if (i == SMC_RETRY_COUNT) return FAN_ERROR;

    return (rpm[0] * 0xFF + rpm[1]) / 4;
}

int Smc::setFanRPM(char key[], int rpm) {
    short speed[2];

    rpm = rpm * 4;
    speed[0] = rpm >> 8;
    speed[1] = rpm & 0x00FF;

    int i;
    for (i = 0; i < SMC_RETRY_COUNT; i++) {
        if (writeKeyS(key, speed, 2)) break;

        Sleep(SMC_RETRY_INTERVAL_LONG);
    }

    // Error if retried max times
    if (i == SMC_RETRY_COUNT) return FAN_ERROR;

    return 0;
}
