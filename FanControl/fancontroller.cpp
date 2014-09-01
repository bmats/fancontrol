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

#include "fancontroller.h"
#include "debug.h"

#include "temperaturereader.h"

FanController::FanController()
    : smc(Smc::getInstance()),
      timer(new QTimer) {
    m_hasRightFan = smc.getFanCount() > 1;

    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(2000);
}

FanController::~FanController() {
    delete timer;
}

void FanController::refreshPreferences() {
    mode            = Preferences::mode();
    leftFanControl  = Preferences::leftFanControl();
    rightFanControl = Preferences::rightFanControl();
    autoLeftMinRPM  = Preferences::autoLeftMin();
    autoLeftMaxRPM  = Preferences::autoLeftMax();
    autoRightMinRPM = Preferences::autoRightMin();
    autoRightMaxRPM = Preferences::autoRightMax();
    manualLeft      = Preferences::manualLeft();
    manualRight     = Preferences::manualRight();
    lowerTempThresh = Preferences::lowerTempThresh();
    upperTempThresh = Preferences::upperTempThresh();
}


#define SMC_KEY_LEFT_FAN_RPM      "F0Ac"
#define SMC_KEY_RIGHT_FAN_RPM     "F1Ac"

#define SMC_KEY_LEFT_FAN_RPM_MIN  "F0Mn"
#define SMC_KEY_RIGHT_FAN_RPM_MIN "F1Mn"

#define SMC_KEY_LEFT_FAN_RPM_MAX  "F0Mx"
#define SMC_KEY_RIGHT_FAN_RPM_MAX "F1Mx"

int FanController::leftRPM() {
    return smc.getFanRPM(SMC_KEY_LEFT_FAN_RPM);
}

int FanController::rightRPM() {
    return smc.getFanRPM(SMC_KEY_RIGHT_FAN_RPM);
}

bool FanController::hasRightFan() {
    return m_hasRightFan;
}

// Values that control the speed-stepping of the fans:
// - fans are adjusted in increments of RPM_SPEED_STEP
// - fans are adjusted to RPMs on RPM_SPEED_STEP boundaries
// - fans are adjusted in speed increments no larger than RPM_MAX_SPEED_STEP
#define RPM_SPEED_STEP     100
#define RPM_MAX_SPEED_STEP 500

static int leftFanRPM = 0, rightFanRPM = 0;
static int lastLeftTargetRPM = 0, lastRightTargetRPM = 0;
static double cpuTemp, gpuTemp;

int FanController::calculateTargetRpm(double controlTemp, int fanRPM, int minRPM, int maxRPM) {
    int targetRPM;

    // Determine the target RPM as indicated by the preference settings
    if (controlTemp < lowerTempThresh) {
        targetRPM = minRPM;
    }
    else if (controlTemp > upperTempThresh) {
        targetRPM = maxRPM;
    }
    else {
        // Scale temperature to RPM
        targetRPM = minRPM +
            ((floor(controlTemp + 0.5) - lowerTempThresh) /
            (upperTempThresh - lowerTempThresh) *
            (maxRPM - minRPM));
    }

    DEBUG("leftControlTemp = %f\n", controlTemp);
    DEBUG("rounded leftControlTemp = %f\n", floor(controlTemp + 0.5));
    DEBUG("ideal leftFanTargetRPM = %d\n", targetRPM);
    DEBUG("leftFanRPM = %d\n", fanRPM);

    // Correct the fan speed if we don't have a fan-speed reading from SMC
    if (fanRPM == 0) {
        fanRPM = targetRPM;
        DEBUG("corrected leftFanRPM = %d\n", fanRPM);
    }

    // Determine the difference between fan's target RPM and the current RPM
    int adjustmentRPM = (targetRPM - fanRPM);

#ifdef FAN_EASING
    if (abs(adjustmentRPM) < (RPM_SPEED_STEP / 2)) {
        adjustmentRPM = 0; // current speed's within 1/2 of an RPM step, leave it
    }
    else {
        // Ensure the +/- difference is not greater than the maximum allowed
        if (adjustmentRPM < -RPM_MAX_SPEED_STEP) adjustmentRPM = -RPM_MAX_SPEED_STEP;
        if (adjustmentRPM >  RPM_MAX_SPEED_STEP) adjustmentRPM =  RPM_MAX_SPEED_STEP;
    }
    DEBUG("adjustmentRPM = %d\n", adjustmentRPM);

    // Compute the new target RPM
    targetRPM = fanRPM + adjustmentRPM;
    DEBUG("next leftFanTargetRPM = %d\n", targetRPM);
#endif

    // Set the target RPM to the nearest RPM_SPEED_STEP-RPM boundary
    int alignmentRPM = (targetRPM % RPM_SPEED_STEP);
    targetRPM = targetRPM - alignmentRPM;

    if (alignmentRPM > (RPM_SPEED_STEP / 2)) {
        targetRPM = targetRPM + RPM_SPEED_STEP;
    }

    DEBUG("%d RPM-aligned next leftFanTargetRPM = %d\n", RPM_SPEED_STEP, targetRPM);

    // When decreasing speed, don't target below the set "slowest fan speed" and
    // When increasing speed, don't target above the maximum safe fan speed
    DEBUG("leftFanBaseRPM = %d\n", minRPM);
    if ((adjustmentRPM <= 0) && (targetRPM < minRPM)) {
        targetRPM = minRPM;
    }

    if (targetRPM > maxRPM) {
        targetRPM = maxRPM;
    }

    DEBUG("final next leftFanTargetRPM = %d\n\n", targetRPM);
    DEBUG("********************************\n\n\n");

    return targetRPM;
}

void FanController::update() {
    // Get fan RPMs

    int leftFanRPMTmp = smc.getFanRPM(SMC_KEY_LEFT_FAN_RPM);
    if (leftFanRPMTmp == FAN_ERROR) {
        // If there are problems reading the left fan, use the last value
        leftFanRPM = lastLeftTargetRPM;
    }
    else {
        leftFanRPM = leftFanRPMTmp;
    }

    if (m_hasRightFan) {
        int rightFanRPMTmp = smc.getFanRPM(SMC_KEY_RIGHT_FAN_RPM);
        if (rightFanRPMTmp == FAN_ERROR) {
            // If there are problems reading the right fan, use the last value
            rightFanRPM = lastRightTargetRPM;
        }
        else {
            rightFanRPM = rightFanRPMTmp;
        }
    }
    else {
        rightFanRPM = 0;
    }

    // Get temperatures

    TemperatureUpdate update = TemperatureReader::getTemps();
    cpuTemp = update.cpu;
    gpuTemp = update.gpu;

    if (mode == Auto) {
        // Determine the controlling temperature
        double leftControlTemp, rightControlTemp;

        switch (leftFanControl) {
        case CPU:
            leftControlTemp = cpuTemp;
            break;
        case GPU:
            leftControlTemp = gpuTemp;
            break;
        case MaxBoth:
            leftControlTemp = cpuTemp > gpuTemp ? cpuTemp : gpuTemp;
        }

        switch (rightFanControl) {
        case CPU:
            rightControlTemp = cpuTemp;
            break;
        case GPU:
            rightControlTemp = gpuTemp;
            break;
        case MaxBoth:
            rightControlTemp = cpuTemp > gpuTemp ? cpuTemp : gpuTemp;
        }

        // Compute the target fan speeds
        int leftTargetRPM  = calculateTargetRpm(leftControlTemp,  leftFanRPM,  autoLeftMinRPM,  autoLeftMaxRPM),
            rightTargetRPM = calculateTargetRpm(rightControlTemp, rightFanRPM, autoRightMinRPM, autoRightMaxRPM);

        // Set max RPMs
        smc.setFanRPM(SMC_KEY_LEFT_FAN_RPM_MAX,  autoLeftMaxRPM);
        smc.setFanRPM(SMC_KEY_RIGHT_FAN_RPM_MAX, autoRightMaxRPM);


        DEBUG("LAST LEFT: %i\n", lastLeftTargetRPM);

        // Set left fan speed only if it differs from the previous value
        if ((leftTargetRPM > (leftFanRPM + (RPM_SPEED_STEP / 2))) ||
            (leftTargetRPM < (leftFanRPM - (RPM_SPEED_STEP / 2))) ||
            (leftTargetRPM != lastLeftTargetRPM)) {

            if (!smc.setFanRPM(SMC_KEY_LEFT_FAN_RPM_MIN, leftTargetRPM)) {
                lastLeftTargetRPM = leftTargetRPM;
            }
        }

        if (m_hasRightFan) {
            DEBUG("LAST RIGHT: %i\n", lastRightTargetRPM);

            // Set the right fan speed only if it differs from the previous value
            if ((rightTargetRPM > (rightFanRPM + (RPM_SPEED_STEP / 2))) ||
                (rightTargetRPM < (rightFanRPM - (RPM_SPEED_STEP / 2))) ||
                (rightTargetRPM != lastRightTargetRPM)) {

                if (!smc.setFanRPM(SMC_KEY_RIGHT_FAN_RPM_MIN, rightTargetRPM)) {
                    lastRightTargetRPM = rightTargetRPM;
                }
            }
        }
        else {
            lastRightTargetRPM = 0;
        }
    }
    else {
        smc.setFanRPM(SMC_KEY_LEFT_FAN_RPM_MIN,  manualLeft);
        smc.setFanRPM(SMC_KEY_RIGHT_FAN_RPM_MIN, manualRight);
        smc.setFanRPM(SMC_KEY_LEFT_FAN_RPM_MAX,  manualLeft);
        smc.setFanRPM(SMC_KEY_RIGHT_FAN_RPM_MAX, manualRight);
    }
}
