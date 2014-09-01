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

#include "temperaturereader.h"
#include "debug.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QCoreApplication>
#include "smc.h"
#include "nvapi.h"

static Smc &smc = Smc::getInstance();

QList<SensorTemperature> TemperatureReader::temperatures;

#define SMC_CPU_TEMP_KEY1 "TC0D" // CPU diode temp
#define SMC_CPU_TEMP_KEY2 "TC0F"
#define SMC_GPU_TEMP_KEY "TG0P" // GPU proximity temp
// TODO: also try TG0D for GPU

#define CPU_ERROR_TEMP 129.0 // We sometimes get this after resuming from sleep

struct SensorInfo {
    QString key;
    QString name;

    bool operator== (const SensorInfo &other) {
        return key == other.key;
    }
};

QList<SensorInfo> &getSensors();

double getNvidiaTemperature();

// Whether CPU key and NVIDIA on/off has been determined
static bool calibrated = false;
// Whether the valid extended sensors have been determined
static bool filteredExtended = false;

static char *cpuKey;
static bool gpuUseNvidia = true;

TemperatureUpdate TemperatureReader::getTemps(bool extended) {
    if (!calibrated) {
        // Test which CPU key works
        cpuKey = (smc.getTemperature(SMC_CPU_TEMP_KEY1) != TEMPERATURE_ERROR) ? SMC_CPU_TEMP_KEY1 : SMC_CPU_TEMP_KEY2;

        // Check if the NVIDIA API is working
        if (getNvidiaTemperature() == TEMPERATURE_ERROR) gpuUseNvidia = false;

        calibrated = true;
    }

    // Get the main update temps
    TemperatureUpdate update;
    update.cpu = smc.getTemperature(cpuKey);
    if (update.cpu == CPU_ERROR_TEMP) update.cpu = TEMPERATURE_ERROR;
    update.gpu = gpuUseNvidia ? getNvidiaTemperature() : smc.getTemperature(SMC_GPU_TEMP_KEY);

    // Get extended temps if necessary
    if (extended) {
        if (filteredExtended) {
            for (int i = 0; i < temperatures.count(); ++i) {
                SensorTemperature temp = temperatures[i];
                temp.value = smc.getTemperature(temp.key);
            }
            update.sensors = &temperatures;
        }
        else {
            QList<SensorInfo> &sensors = getSensors();
            temperatures.clear();

            double temp;
            for (int i = 0; i < sensors.count(); ++i) {
                SensorInfo s = sensors[i];
                temp = smc.getTemperature(s.key.toLocal8Bit().data());
                if (temp != TEMPERATURE_ERROR) {
                    SensorTemperature sensorTemp;
                    sensorTemp.name  = s.name;
                    sensorTemp.key   = s.key.toLocal8Bit().data();
                    sensorTemp.value = temp;
                    temperatures.append(sensorTemp);
                }
            }
            update.sensors = &temperatures;

            filteredExtended = true;
        }
    }

    return update;
}

///
/// \brief Gets the NVIDIA temperature.
/// \return The temperature, or TEMPERATURE_ERROR if there is an error.
///
double getNvidiaTemperature() {
    int i = 0;
    unsigned long cnt;
    NvDisplayHandle hDisplay_a[NVAPI_MAX_PHYSICAL_GPUS * 2] = { 0 };
    NvPhysicalGpuHandle nvGPUHandle;

    NvAPI_Status status;
    NvAPI_ShortString nvAPIstatus_output;

    status = NvAPI_Initialize();
    NvAPI_GetErrorMessage(status, nvAPIstatus_output);

    status = NvAPI_EnumNvidiaDisplayHandle(i, &hDisplay_a[i]);
    if (status != NVAPI_OK) {
#ifdef DEBUG_ON
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(status, string);
        DEBUG("NVAPI error in NvAPI_EnumNvidiaDisplayHandle: %s\n", string);
#endif
        return TEMPERATURE_ERROR;
    }

    status = NvAPI_GetPhysicalGPUsFromDisplay(hDisplay_a[0], &nvGPUHandle, &cnt);
    if (status != NVAPI_OK) {
#ifdef DEBUG_ON
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(status, string);
        DEBUG("NVAPI error in NvAPI_GetPhysicalGPUsFromDisplay: %s\n", string);
#endif
        return TEMPERATURE_ERROR;
    }

    NV_GPU_THERMAL_SETTINGS set = { 0 };
    set.version = NV_GPU_THERMAL_SETTINGS_VER;
    set.count = 0;
    set.sensor[0].controller = NVAPI_THERMAL_CONTROLLER_UNKNOWN;
    set.sensor[0].target = NVAPI_THERMAL_TARGET_GPU;

    status = NvAPI_GPU_GetThermalSettings(nvGPUHandle, 0, &set);
    if (status != NVAPI_OK) {
#ifdef DEBUG_ON
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(status, string);
        DEBUG("NVAPI error in NvAPI_GPU_GetThermalSettings: %s\n", string);
#endif
        return TEMPERATURE_ERROR;
    }

    return set.sensor[0].currentTemp;
}

void readJson(QString name, QList<SensorInfo> *sensors, bool override = false) {
    QFile file(name);
    if (!file.exists()) return;

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray json = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonArray sensorsJson = doc.array();

    for (int i = 0; i < sensorsJson.count(); ++i) {
        QJsonObject sensorObject = sensorsJson[i].toObject();
        QJsonValue nameValue = sensorObject["name"];

        SensorInfo sensor;
        sensor.key = sensorObject["key"].toString();

        QString name = (nameValue.type() == QJsonValue::Null ||
                        nameValue.type() == QJsonValue::Undefined) ? sensor.key : nameValue.toString();

        if (override && sensors->contains(sensor)) {
            int index = sensors->indexOf(sensor);
            SensorInfo existingSensor = sensors->at(index);
            existingSensor.name = name;
            sensors->replace(index, existingSensor);
        }
        else {
            sensor.name = name;
            sensors->append(sensor);
        }
    }
}

QList<SensorInfo> &getSensors() {
    static QList<SensorInfo> sensors;

    if (sensors.empty()) {
        // Read the default sensors
        readJson(":/sensors.json", &sensors);
        // Read custom sensors
        readJson(QCoreApplication::applicationDirPath().append("/sensors.json"), &sensors, true);
    }

    return sensors;
}
