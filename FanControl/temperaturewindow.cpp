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

#include "temperaturewindow.h"
#include "ui_temperaturewindow.h"
#include "debug.h"
#include <QDesktopWidget>
#include <QColor>
#include "preferences.h"

#define HOT_COLOR  QColor(214, 45, 37)
#define WARM_COLOR QColor(212, 98, 16)
#define COOL_COLOR QColor(28, 135, 0)

TemperatureWindow::TemperatureWindow(QWidget *parent, TemperatureUpdateTimer *timer)
    : QMainWindow(parent, Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint),
      ui(new Ui::TemperatureWindow),
      model(new TemperatureModel),
      updateTimer(timer) {
    ui->setupUi(this);

    setGeometry(QStyle::alignedRect(
        Qt::LeftToRight,
        Qt::AlignCenter,
        size(),
        qApp->desktop()->availableGeometry()
    ));

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->resizeSection(0, 200);
    ui->tableView->horizontalHeader()->resizeSection(1, 80);
}

TemperatureWindow::~TemperatureWindow() {
    delete ui;
    delete model;
}

void TemperatureWindow::showEvent(QShowEvent*) {
    // Enable extended sensors
    updateTimer->setExtended(true);
}

void TemperatureWindow::closeEvent(QCloseEvent*) {
    // Disable extended sensors
    updateTimer->setExtended(false);
}

void TemperatureWindow::update(TemperatureUpdate update) {
    model->updateData(update);
}


int TemperatureModel::rowCount(const QModelIndex&) const {
    // Display loading row if no data yet
    return dataLoaded ? rows.count() : 1;
}

int TemperatureModel::columnCount(const QModelIndex&) const {
    return 2;
}

QVariant TemperatureModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case 0:
                return tr("Sensor");
            case 1:
                return tr("Temperature");
            }
        case Qt::TextAlignmentRole:
            return Qt::AlignLeft;
        }
    }
    return QVariant();
}

QVariant TemperatureModel::data(const QModelIndex &index, int role) const {
    if (!dataLoaded) {
        return (role == Qt::DisplayRole && index.row() == 0 && index.column() == 0)
                ? QString("Loading...")
                : QVariant();
    }

    if (index.row() >= rows.count()) return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return rows[index.row()].name;
        case 1:
            return Preferences::tempStr(rows[index.row()].value, 2);
        }
    case Qt::TextColorRole:
        if (index.column() == 1) {
            double temp = rows[index.row()].value;
            if      (temp > HOT_TEMPERATURE)  return HOT_COLOR;
            else if (temp > WARM_TEMPERATURE) return WARM_COLOR;
            else if (temp >= 0)               return COOL_COLOR;
        }
    }

    return QVariant();
}

Qt::ItemFlags TemperatureModel::flags(const QModelIndex &) const {
    return dataLoaded ? Qt::ItemIsEnabled : 0;
}

void TemperatureModel::updateData(TemperatureUpdate data) {
    // Find the number of rows
    int newRowCount = 2 + data.sensors->count();

    if (rows.count() != newRowCount) {
        // Resize QList and model
        if (rows.count() < newRowCount) {
            beginInsertRows(QModelIndex(), dataLoaded ? rows.count() - 1 : 1, newRowCount - 1);
            TemperatureModelRow newElem;
            for (int i = rows.count(); i < newRowCount; ++i) {
                rows.append(newElem);
            }
            endInsertRows();
        } else { // rows.count() > newNumRows
            beginRemoveRows(QModelIndex(), newRowCount - 1, rows.count() - 1);
            rows.erase(rows.begin() + newRowCount - 1, rows.end());
            endRemoveRows();
        }
    }

    rows[0].name  = tr("CPU");
    rows[0].value = data.cpu;
    rows[1].name  = tr("GPU");
    rows[1].value = data.gpu;

    int i = 2;
    for (QList<SensorTemperature>::iterator it = data.sensors->begin(); it != data.sensors->end(); ++it) {
        rows[i].name  = it->name;
        rows[i].value = it->value;
        ++i;
    }

    dataLoaded = true;
    emit dataChanged(createIndex(0, 0), createIndex(newRowCount - 1, 2));
}
