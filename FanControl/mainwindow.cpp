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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debug.h"
#include "fanspeedwidget.h"

#define NOMINMAX
#include <windows.h>

#define HOT_COLOR  0xFF352C
#define WARM_COLOR 0xDA6510
#define COOL_COLOR 0x209E00
#define ICON_WIDTH 16

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent, Qt::CustomizeWindowHint | Qt::Tool),
      ui(new Ui::MainWindow),
      controller(new FanController),
      updateTimer(new TemperatureUpdateTimer) {
    ui->setupUi(this);

    // Setup UI

    ui->autoLeftSlider->setSpan(1200, 6000);
    ui->autoRightSlider->setSpan(1200, 6000);

    ui->autoLeftIndicator->setDirection(FanSpeedWidget::Left);
    ui->autoRightIndicator->setDirection(FanSpeedWidget::Right);
    ui->manualLeftIndicator->setDirection(FanSpeedWidget::Left);
    ui->manualRightIndicator->setDirection(FanSpeedWidget::Right);

    int top = ui->autoLeftSlider->y() - ui->autoLeftIndicator->y();
    ui->autoLeftIndicator->setVerticalMargins(top, ui->autoLeftIndicator->height() - top - ui->autoLeftSlider->height());
    top = ui->autoRightSlider->y() - ui->autoRightIndicator->y();
    ui->autoRightIndicator->setVerticalMargins(top, ui->autoRightIndicator->height() - top - ui->autoRightSlider->height());

    ui->stackedWidget->setCurrentIndex(1); // needed for accurate geometry
    top = ui->manualLeftSlider->y() - ui->manualLeftIndicator->y();
    ui->manualLeftIndicator->setVerticalMargins(top, ui->manualLeftIndicator->height() - top - ui->manualLeftSlider->height());
    top = ui->manualRightSlider->y() - ui->manualRightIndicator->y();
    ui->manualRightIndicator->setVerticalMargins(top, ui->manualRightIndicator->height() - top - ui->manualRightSlider->height());

    connect(ui->temperatureButton, SIGNAL(clicked()), this, SLOT(showTemperatures()));

    createMenu();
    createTrayIcon();

    // Modify UI from prefs

    ui->autoLeftSlider->setLowerValue(Preferences::autoLeftMin());
    ui->autoLeftSlider->setUpperValue(Preferences::autoLeftMax());
    ui->autoRightSlider->setLowerValue(Preferences::autoRightMin());
    ui->autoRightSlider->setUpperValue(Preferences::autoRightMax());
    ui->manualLeftSlider->setValue(Preferences::manualLeft());
    ui->manualRightSlider->setValue(Preferences::manualRight());

    currentMode = Preferences::mode();
    toggleMode(currentMode == Manual);
    manualAction->setChecked(currentMode == Manual);

    linkAction_toggled(Preferences::linked());
    linkAction->setChecked(Preferences::linked());

    // Start update timer
    qRegisterMetaType<TemperatureUpdate>("TemperatureUpdate");
    connect(updateTimer, SIGNAL(updated(TemperatureUpdate)), this, SLOT(update(TemperatureUpdate)));
    updateTimer->start();

    connect(ui->autoLeftSlider,  SIGNAL(spanChanged(int,int)), this, SLOT(apply()));
    connect(ui->autoRightSlider, SIGNAL(spanChanged(int,int)), this, SLOT(apply()));
    connect(ui->manualLeftSlider,  SIGNAL(valueChanged(int)), this, SLOT(apply()));
    connect(ui->manualRightSlider, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    lastAlertTime = QTime::currentTime().addSecs(-Preferences::heatWarningsInterval());
}

MainWindow::~MainWindow() {
    trayIcon->hide();

    delete ui;
    delete controller;

    delete updateTimer;

    if (tempsWindow) delete tempsWindow;
    if (prefsDialog) delete prefsDialog;
    if (aboutDialog) delete aboutDialog;

    delete menu;
    delete manualAction;
    delete linkAction;
    delete prefsAction;
    delete aboutAction;
    delete quitAction;
    delete trayIcon;
}

void MainWindow::update(TemperatureUpdate update) {
    if (tempsWindow && tempsWindow->isVisible()) {
        // Send the update to the temperature window
        tempsWindow->update(update);
    }

    // Find if we need to display an alert again
    if (Preferences::heatWarningsEnabled() &&
            lastAlertTime.secsTo(QTime::currentTime()) > Preferences::heatWarningsInterval()) {
        int heatThresh = Preferences::heatWarningsThreshold();
        bool cpuHot = update.cpu >= heatThresh,
             gpuHot = update.gpu >= heatThresh;

        if (cpuHot || gpuHot) {
            QString title, message;

            if (cpuHot) title = title.append(tr("CPU "));
            if (cpuHot && gpuHot) title = title.append(tr("and "));
            if (gpuHot) title = title.append(tr("GPU "));

            if (cpuHot && gpuHot) title = title.append(tr("are hot"));
            else title = title.append(tr("is hot"));

            if (cpuHot) message = message.append(tr("CPU temperature is %1. ").arg(Preferences::tempStr(update.cpu)));
            if (gpuHot) message = message.append(tr("GPU temperature is %1. ").arg(Preferences::tempStr(update.gpu)));

            message = message.append((cpuHot && gpuHot)
                                     ? tr("Increase your fan speed to cool them down.")
                                     : tr("Increase your fan speed to cool it down."));

            trayIcon->showMessage(title, message, QSystemTrayIcon::Warning, 8000);
            lastAlertTime = QTime::currentTime();
        }
    }


    int leftRPM  = controller->leftRPM(),
        rightRPM = controller->rightRPM();
    double cpuTemp = update.cpu,
           gpuTemp = update.gpu;
    bool hasRightFan = controller->hasRightFan();

    ui->autoLeftIndicator->setActualFanSpeed(leftRPM);
    ui->manualLeftIndicator->setActualFanSpeed(leftRPM);
    ui->autoRightIndicator->setActualFanSpeed(rightRPM);
    ui->manualRightIndicator->setActualFanSpeed(rightRPM);

    QString notifyText, labelText;

    int cpuTempColor, gpuTempColor;

    if      (cpuTemp > HOT_TEMPERATURE)  cpuTempColor = HOT_COLOR;
    else if (cpuTemp > WARM_TEMPERATURE) cpuTempColor = WARM_COLOR;
    else                                 cpuTempColor = COOL_COLOR;

    if      (gpuTemp > HOT_TEMPERATURE)  gpuTempColor = HOT_COLOR;
    else if (gpuTemp > WARM_TEMPERATURE) gpuTempColor = WARM_COLOR;
    else                                 gpuTempColor = COOL_COLOR;

    // Assemble temperature label text
    labelText = tr("CPU <span style=\"color:#%1\">%2</span> GPU <span style=\"color:#%3\">%4</span>")
                .arg(QString::number(cpuTempColor, 16)).arg(Preferences::tempStr(cpuTemp, 1, false))
                .arg(QString::number(gpuTempColor, 16)).arg(Preferences::tempStr(gpuTemp, 1, false));

    // Assemble tray icon tooltop text
    notifyText = tr("Left: %1 RPM").arg(QString::number(leftRPM));
    if (hasRightFan) {
        notifyText = notifyText.append(tr("\nRight: %1 RPM").arg(QString::number(rightRPM)));
    }
    notifyText = notifyText.append(tr("\nCPU: %1\nGPU: %2")
                                   .arg(Preferences::tempStr(cpuTemp))
                                   .arg(Preferences::tempStr(gpuTemp)));

    ui->tempLabel->setText(labelText);
    trayIcon->setToolTip(notifyText);
}

void MainWindow::apply() {
    if (currentMode == Auto) {
        if (slidersLinked) {
            // Update links
            if (sender() == ui->autoLeftSlider) {
                ui->autoRightSlider->setUpperValue(ui->autoLeftSlider->upperValue());
                ui->autoRightSlider->setLowerValue(ui->autoLeftSlider->lowerValue());
            }
            else if (sender() == ui->autoRightSlider) {
                ui->autoLeftSlider->setUpperValue(ui->autoRightSlider->upperValue());
                ui->autoLeftSlider->setLowerValue(ui->autoRightSlider->lowerValue());
            }
        }

        Preferences::setAutoLeftMin(ui->autoLeftSlider->lowerValue());
        Preferences::setAutoLeftMax(ui->autoLeftSlider->upperValue());
        Preferences::setAutoRightMin(ui->autoRightSlider->lowerValue());
        Preferences::setAutoRightMax(ui->autoRightSlider->upperValue());
    }
    else {
        if (slidersLinked) {
            // Update links
            if (sender() == ui->manualLeftSlider) {
                ui->manualRightSlider->setValue(ui->manualLeftSlider->value());
            }
            else if (sender() == ui->manualRightSlider) {
                ui->manualLeftSlider->setValue(ui->manualRightSlider->value());
            }
        }

        Preferences::setManualLeft(ui->manualLeftSlider->value());
        Preferences::setManualRight(ui->manualRightSlider->value());
    }

    controller->refreshPreferences();
}

void MainWindow::toggleMode(bool manual) {
    ui->stackedWidget->setCurrentIndex(manual ? 1 : 0);
    currentMode = manual ? Manual : Auto;
    apply();

    Preferences::setMode(currentMode);
    controller->refreshPreferences();
}

bool MainWindow::event(QEvent *e) {
    switch (e->type()) {
    case QEvent::WindowDeactivate:
        hide();
        break;
    }
    return QMainWindow::event(e);
}

void MainWindow::showWindowNearTray() {
    trayIconPos = trayIcon->geometry().topLeft();

    int windowX, windowY;

    // Find the taskbar bounds to place the icon
    HWND taskbar = FindWindow(L"Shell_traywnd", NULL);
    RECT tbRect;
    if (taskbar && GetWindowRect(taskbar, &tbRect)) {
        if (tbRect.right - tbRect.left > tbRect.bottom - tbRect.top) {
            // taskbar is horizontal
            windowX = trayIconPos.x() + ICON_WIDTH / 2 - this->geometry().width() / 2;

            // stay on screen
            if (windowX < 8) {
                windowX = 8;
            }
            else if (windowX > tbRect.right - 22 - this->geometry().width()) {
                windowX = tbRect.right - 22 - this->geometry().width();
            }

            if (tbRect.top == 0) {
                // top
                windowY = tbRect.bottom + 8;
            }
            else {
                // bottom
                windowY = tbRect.top - 22 - this->geometry().height();
            }
        }
        else {
            // taskbar is vertical
            windowY = trayIconPos.y() + ICON_WIDTH / 2 - this->geometry().height() / 2;

            // stay on screen
            if (windowY < 8) {
                windowY = 8;
            }
            else if (windowY > tbRect.bottom - 22 - this->geometry().height()) {
                windowY = tbRect.bottom - 22 - this->geometry().height();
            }

            if (tbRect.left == 0) {
                // left
                windowX = tbRect.right + 8;
            }
            else {
                // right
                windowX = tbRect.left - 22 - this->geometry().width();
            }
        }
    }
    else {
        // Fallback
        windowX = trayIconPos.x() + ICON_WIDTH / 2 - this->geometry().width() / 2;
        windowY = trayIconPos.y() - 24 - this->geometry().height();
    }

    move(windowX, windowY);
    show();
    activateWindow();
}

void MainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Context:
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (!isVisible()) showWindowNearTray();
        else hide();
        break;
    }
}

void MainWindow::trayIcon_messageClicked() {
    if (!isVisible()) showWindowNearTray();
}

void MainWindow::createMenu() {
    menu = new QMenu(this);

    manualAction = new QAction(tr("Manual"), this);
    manualAction->setCheckable(true);
    connect(manualAction, SIGNAL(toggled(bool)), this, SLOT(toggleMode(bool)));

    linkAction = new QAction(tr("Link"), this);
    linkAction->setCheckable(true);
    connect(linkAction, SIGNAL(toggled(bool)), this, SLOT(linkAction_toggled(bool)));

    prefsAction = new QAction(tr("Preferences..."), this);
    connect(prefsAction, SIGNAL(triggered()), this, SLOT(showPreferences()));

    aboutAction = new QAction(tr("About..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

    quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    menu->addAction(manualAction);
    menu->addAction(linkAction);
    menu->addSeparator();
    menu->addAction(prefsAction);
    menu->addAction(aboutAction);
    menu->addSeparator();
    menu->addAction(quitAction);

    connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(toggleMenu()));
}

void MainWindow::toggleMenu() {
    if (menu->isVisible()) {
        menu->hide();
    }
    else {
        menu->show();
        menu->move(mapToGlobal(ui->menuButton->pos() + QPoint(0, -menu->height() - 5)));
    }
}

void MainWindow::linkAction_toggled(bool linked) {
    slidersLinked = linked;
    Preferences::setLinked(linked);
}

void MainWindow::showTemperatures() {
    if (!tempsWindow) tempsWindow = new TemperatureWindow(this, updateTimer);

    tempsWindow->show();
}

void MainWindow::showPreferences() {
    if (!prefsDialog) prefsDialog = new PreferencesDialog;
    connect(prefsDialog, SIGNAL(accepted()), this, SLOT(updatePreferences()));
    prefsDialog->show();
    prefsDialog->activateWindow();
}

void MainWindow::updatePreferences() {
    controller->refreshPreferences();
}

void MainWindow::showAbout() {
    if (!aboutDialog) aboutDialog = new AboutDialog;
    aboutDialog->show();
    aboutDialog->activateWindow();
}

void MainWindow::createTrayIcon() {
    trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon(":/res/fan.png"));
    trayIcon->setToolTip(tr("Fan Control"));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(trayIcon_messageClicked()));

    trayIcon->show();
}
