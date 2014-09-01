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

#include "temperatureupdatetimer.h"

#define UPDATE_INTERVAL 1000

TemperatureUpdateTimer::TemperatureUpdateTimer(QObject *parent)
    : QObject(parent),
      timer(new QTimer),
      thread(new QThread) {

    this->moveToThread(thread);
    timer->moveToThread(thread);
    timer->setInterval(UPDATE_INTERVAL);

    connect(timer,  SIGNAL(timeout()),  this,  SLOT(update()));
    connect(thread, SIGNAL(started()),  timer, SLOT(start()));
    connect(thread, SIGNAL(finished()), timer, SLOT(stop()));
    connect(thread, SIGNAL(finished()), timer, SLOT(deleteLater()));
}

TemperatureUpdateTimer::~TemperatureUpdateTimer() {
    thread->terminate();
    thread->wait();
    delete thread;

    // timer will deleteLater()
}

void TemperatureUpdateTimer::start() {
    thread->start(QThread::LowPriority);
    update();
}

void TemperatureUpdateTimer::update() {
    // Blocking call
    TemperatureUpdate update = TemperatureReader::getTemps(m_extended);
    emit updated(update);
}

bool TemperatureUpdateTimer::extended() {
    return m_extended;
}

void TemperatureUpdateTimer::setExtended(bool extended) {
    m_extended = extended;
}
