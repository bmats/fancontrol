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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>
#include "fancontroller.h"
#include "preferences.h"
#include "temperaturereader.h"
#include "temperaturewindow.h"
#include "preferencesdialog.h"
#include "aboutdialog.h"
#include "temperatureupdatetimer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void trayIcon_messageClicked();
    void toggleMenu();
    void toggleMode(bool manual);
    void linkAction_toggled(bool linked);
    void showTemperatures();
    void showPreferences();
    void showAbout();
    void apply();
    void updatePreferences();

    ///
    /// Updates the UI with fan speeds and current status.
    ///
    void update(TemperatureUpdate update);

private:
    bool event(QEvent *e);
    void createTrayIcon();
    void createMenu();
    void showWindowNearTray();

    bool slidersLinked;

    Ui::MainWindow *ui;

    UIMode currentMode;
    FanController *controller;
    TemperatureUpdateTimer *updateTimer;

    TemperatureWindow *tempsWindow = 0;
    PreferencesDialog *prefsDialog = 0;
    AboutDialog *aboutDialog = 0;

    QTime lastAlertTime;

    QMenu *menu = 0;
    QAction *manualAction = 0, *linkAction = 0, *prefsAction = 0, *aboutAction = 0, *quitAction = 0;
    QSystemTrayIcon *trayIcon = 0;
    QPoint trayIconPos;
};

#endif // MAINWINDOW_H
