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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "debug.h"

static Temp heatWarnThreshValue, heatWarnThreshMax, heatWarnThreshMin;
static Temp autoTempSliderMinValue, autoTempSliderMaxValue, autoTempSliderMin, autoTempSliderMax;

// Temperature scale macros
#define C_TO_F(value) value * 1.8 + 32.0
#define F_TO_C(value) (value - 32.0) / 1.8
#define C_TO_F_IF_F(value) useCelsius ? value : C_TO_F(value)
#define F_TO_C_IF_C(value) useCelsius ? F_TO_C(value) : value
#define F_TO_C_IF_F(value) useCelsius ? value : F_TO_C(value)

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PreferencesDialog) {
    ui->setupUi(this);

    // Hide QDialog help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


    // Load settings from preferences

    // Fan Control Mode
    TempControlMode leftControl = Preferences::leftFanControl(), rightControl = Preferences::rightFanControl();
    ui->fansLeftCpuRadio->setChecked(leftControl == CPU);
    ui->fansLeftGpuRadio->setChecked(leftControl == GPU);
    ui->fansLeftMaxRadio->setChecked(leftControl == MaxBoth);
    ui->fansRightCpuRadio->setChecked(rightControl == CPU);
    ui->fansRightGpuRadio->setChecked(rightControl == GPU);
    ui->fansRightMaxRadio->setChecked(rightControl == MaxBoth);

    // Temperature Scale
    useCelsius = Preferences::useCelsius();
    ui->celsiusRadio->setChecked(useCelsius);
    ui->fahrenheitRadio->setChecked(!useCelsius);

    // Auto Temperature Range

    Temp autoTempLower = Preferences::lowerTempThresh(), autoTempUpper = Preferences::upperTempThresh();

    // Save initial values
    autoTempSliderMinValue = autoTempLower;
    autoTempSliderMaxValue = autoTempUpper;
    autoTempSliderMin = ui->autoTempSlider->minimum();
    autoTempSliderMax = ui->autoTempSlider->maximum();

    if (!useCelsius) {
        ui->autoTempSlider->setMinimum(C_TO_F(autoTempSliderMin));
        ui->autoTempSlider->setMaximum(C_TO_F(autoTempSliderMax));
    }

    ui->autoTempSlider->setSpan(C_TO_F_IF_F(autoTempLower), C_TO_F_IF_F(autoTempUpper));
    setAutoTempLower(C_TO_F_IF_F(autoTempLower));
    setAutoTempUpper(C_TO_F_IF_F(autoTempUpper));

    // Heat Warnings

    ui->heatWarningsEnabledBox->setChecked(Preferences::heatWarningsEnabled());
    setHeatControlsEnabled(Preferences::heatWarningsEnabled());

    ui->heatWarningsIntervalSpin->setValue(Preferences::heatWarningsInterval());

    // Save initial values
    heatWarnThreshMin   = ui->heatWarningsThresholdSpin->minimum();
    heatWarnThreshMax   = ui->heatWarningsThresholdSpin->maximum();
    heatWarnThreshValue = Preferences::heatWarningsThreshold();

    if (!useCelsius) {
        ui->heatWarningsThresholdSpin->setMinimum(C_TO_F(heatWarnThreshMin));
        ui->heatWarningsThresholdSpin->setMaximum(C_TO_F(heatWarnThreshMax));
        ui->heatWarningsThresholdUnit->setText(QString("° F"));
    }

    ui->heatWarningsThresholdSpin->setValue(C_TO_F_IF_F(Preferences::heatWarningsThreshold()));


    // Connect signals
    connect(ui->autoTempSlider, SIGNAL(lowerValueChanged(int)), this, SLOT(setAutoTempLower(int)));
    connect(ui->autoTempSlider, SIGNAL(upperValueChanged(int)), this, SLOT(setAutoTempUpper(int)));
    connect(ui->celsiusRadio, SIGNAL(toggled(bool)), this, SLOT(setUiTempScale(bool)));
    connect(ui->heatWarningsEnabledBox, SIGNAL(toggled(bool)), this, SLOT(setHeatControlsEnabled(bool)));
    connect(this, SIGNAL(accepted()), this, SLOT(savePreferences()));
}

PreferencesDialog::~PreferencesDialog() {
    delete ui;
}

void PreferencesDialog::savePreferences() {
    if (ui->fansLeftCpuRadio->isChecked())      Preferences::setLeftFanControl(CPU);
    else if (ui->fansLeftGpuRadio->isChecked()) Preferences::setLeftFanControl(GPU);
    else                                        Preferences::setLeftFanControl(MaxBoth);
    if (ui->fansRightCpuRadio->isChecked())      Preferences::setRightFanControl(CPU);
    else if (ui->fansRightGpuRadio->isChecked()) Preferences::setRightFanControl(GPU);
    else                                         Preferences::setRightFanControl(MaxBoth);

    Preferences::setLowerTempThresh(F_TO_C_IF_F(ui->autoTempSlider->lowerValue())); // save the C values
    Preferences::setUpperTempThresh(F_TO_C_IF_F(ui->autoTempSlider->upperValue()));

    Preferences::setUseCelsius(ui->celsiusRadio->isChecked());

    Preferences::setHeatWarningsEnabled(ui->heatWarningsEnabledBox->isChecked());
    Preferences::setHeatWarningsInterval(ui->heatWarningsIntervalSpin->value());
    Preferences::setHeatWarningsThreshold(heatWarnThreshValue); // save the C value
}

void PreferencesDialog::setAutoTempLower(int value) {
    ui->autoTempMin->setText(QString("%1°").arg(value));
}

void PreferencesDialog::setAutoTempUpper(int value) {
    ui->autoTempMax->setText(QString("%1°").arg(value));
}

void PreferencesDialog::setUiTempScale(bool celsius) {
    useCelsius = celsius;

    // Auto Temperature Range
    autoTempSliderMinValue = F_TO_C_IF_C(ui->autoTempSlider->lowerValue());
    autoTempSliderMaxValue = F_TO_C_IF_C(ui->autoTempSlider->upperValue());
    ui->autoTempSlider->setMinimum(C_TO_F_IF_F(autoTempSliderMin));
    ui->autoTempSlider->setMaximum(C_TO_F_IF_F(autoTempSliderMax));
    ui->autoTempSlider->setSpan(C_TO_F_IF_F(autoTempSliderMinValue), C_TO_F_IF_F(autoTempSliderMaxValue));

    // Heat Warnings Threshold
    heatWarnThreshValue = F_TO_C_IF_C(ui->heatWarningsThresholdSpin->value());
    ui->heatWarningsThresholdSpin->setMinimum(C_TO_F_IF_F(heatWarnThreshMin));
    ui->heatWarningsThresholdSpin->setMaximum(C_TO_F_IF_F(heatWarnThreshMax));
    ui->heatWarningsThresholdSpin->setValue(C_TO_F_IF_F(heatWarnThreshValue));

    ui->heatWarningsThresholdUnit->setText(QString("° %1").arg(celsius ? 'C' : 'F'));
}

void PreferencesDialog::setHeatControlsEnabled(bool enabled) {
    ui->heatWarningsIntervalSpin->setEnabled(enabled);
    ui->heatWarningsThresholdSpin->setEnabled(enabled);
}
