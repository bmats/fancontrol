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

#ifndef TEMPERATUREWINDOW_H
#define TEMPERATUREWINDOW_H

#include <QMainWindow>
#include <QAbstractTableModel>
#include <QCloseEvent>
#include <QList>
#include "temperaturereader.h"
#include "temperatureupdatetimer.h"

namespace Ui {
class TemperatureWindow;
}

class TemperatureModel;
struct TemperatureModelRow;

class TemperatureWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TemperatureWindow(QWidget *parent, TemperatureUpdateTimer *timer);
    ~TemperatureWindow();

    void update(TemperatureUpdate update);

private:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

    Ui::TemperatureWindow *ui;
    TemperatureModel *model;
    TemperatureUpdateTimer *updateTimer;
};

class TemperatureModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit TemperatureModel() { }

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &) const;

    void updateData(TemperatureUpdate update);

private:
    bool dataLoaded = false;
    QList<TemperatureModelRow> rows;
};

struct TemperatureModelRow {
    QString name;
    double value;
};

#endif // TEMPERATUREWINDOW_H
