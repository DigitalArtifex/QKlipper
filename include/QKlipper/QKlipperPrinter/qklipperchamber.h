/*
 * QKlipper - A Qt library for the Klipper/Moonraker API
 * Copyright (C) 2024 James Dudeck
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QKLIPPERCHAMBER_H
#define QKLIPPERCHAMBER_H

#include <QObject>

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperChamber : public QObject
{
    Q_OBJECT
public:
    explicit QKlipperChamber(QObject *parent = nullptr);

    qreal temperatureerature() const;
    qreal targetTemperatureerature() const;

    qreal maximumWatts() const;

public slots:
    void setCurrentTemperature(qreal temperatureerature);
    void setTargetTemperatureerature(qreal targetTemperatureerature);

    void setMaxWatts(qreal maximumWatts);

signals:
    void temperatureeratureChanged();
    void targetTemperatureeratureChanged();

    void maximumWattsChanged();

private:
    qreal m_temperatureerature = 0.0;
    qreal m_targetTemperatureerature = 0.0;
    qreal m_maximumWatts = 0.0;
    qreal m_power = 0.0;

    Q_PROPERTY(qreal temperatureerature READ temperatureerature WRITE setCurrentTemperature NOTIFY temperatureeratureChanged FINAL)
    Q_PROPERTY(qreal targetTemperatureerature READ targetTemperatureerature WRITE setTargetTemperatureerature NOTIFY targetTemperatureeratureChanged FINAL)
    Q_PROPERTY(qreal maximumWatts READ maximumWatts WRITE setMaxWatts NOTIFY maximumWattsChanged FINAL)
};

#endif // QKLIPPERCHAMBER_H
