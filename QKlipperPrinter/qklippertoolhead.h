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

#ifndef QKLIPPERTOOLHEAD_H
#define QKLIPPERTOOLHEAD_H

#include <QObject>
#include <QMap>

#include "qklipperfan.h"
#include "qklipperextruder.h"
#include "qklipperposition.h"

class QKlipperToolHead : public QObject
{
    Q_OBJECT
public:
    explicit QKlipperToolHead(QObject *parent = nullptr);
    ~QKlipperToolHead();

    QKlipperFan *partsFan() const;

    QMap<QString, QKlipperExtruder *> extruderMap() const;
    QKlipperExtruder *extruder(const QString &name);
    QKlipperExtruder *currentExtruder();

    QString currentExtruderName() const;

    bool zHomed() const;

    bool yHomed() const;

    bool xHomed() const;

    bool isHoming() const;

    qint32 maxAcceleration() const;

    qint32 maxVelocity() const;

    qint32 maxAccelerationToDeceleration() const;

    qint32 stalls() const;

    qint32 checks() const;

    qreal squareCornerVelocity() const;

    QKlipperPosition position() const;

    QKlipperPosition destination() const;

    QKlipperPosition maxPosition() const;

    QKlipperPosition minPosition() const;

    QKlipperConsole *console() const;

    QKlipperPrinter *printer() const;

public slots:
    void setPartsFan(QKlipperFan *partsFan);

    void setExtruderMap(const QMap<QString, QKlipperExtruder *> &extruderMap);
    void setExtruder(QString name, QKlipperExtruder *extruder);

    void setCurrentExtruderName(const QString &currentExtruderName);

    void setZHomed(bool zHomed);

    void setYHomed(bool yHomed);

    void setXHomed(bool xHomed);

    void setIsHoming(bool isHoming);

    void setMaxAcceleration(qint32 maxAcceleration);

    void setMaxVelocity(qint32 maxVelocity);

    void setMaxAccelerationToDeceleration(qint32 maxAccelerationToDeceleration);

    void setStalls(qint32 stalls);

    void setChecks(qint32 checks);

    void setSquareCornerVelocity(qreal squareCornerVelocity);

    void setPosition(const QKlipperPosition &position);

    void setDestination(const QKlipperPosition &destination);

    void setMaxPosition(const QKlipperPosition &maxPosition);

    void setMinPosition(const QKlipperPosition &minPosition);

    void setConsole(QKlipperConsole *console);

    void setPrinter(QKlipperPrinter *printer);

private slots:
    void setPositionData(const QKlipperPosition &position);

signals:

    void partsFanChanged();

    void extruderMapChanged();

    void currentExtruderNameChanged();

    void zHomedChanged();

    void yHomedChanged();

    void xHomedChanged();

    void isHomingChanged();

    void maxAccelerationChanged();

    void maxVelocityChanged();

    void maxAccelerationToDecelerationChanged();

    void stallsChanged();

    void checksChanged();

    void squareCornerVelocityChanged();

    void positionChanged();

    void destinationChanged();

    void maxPositionChanged();

    void minPositionChanged();

    void consoleChanged();

    void printerChanged();

private:

    QKlipperFan                              *m_partsFan = nullptr;
    QMap<QString, QKlipperExtruder*>          m_extruderMap;
    QKlipperPosition                          m_position;
    QKlipperPosition                          m_destination;
    QKlipperPosition                          m_maxPosition;
    QKlipperPosition                          m_minPosition;
    QString m_currentExtruderName;

    bool m_zHomed = false;
    bool m_yHomed = false;
    bool m_xHomed = false;
    bool m_isHoming = false;

    qint32 m_maxAcceleration = 0;
    qint32 m_maxVelocity = 0;
    qint32 m_maxAccelerationToDeceleration = 0;
    qint32 m_stalls = 0;
    qint32 m_checks = 0;

    qreal m_squareCornerVelocity = 0;

    QKlipperPrinter *m_printer = nullptr;
    QKlipperConsole *m_console = nullptr;

    Q_PROPERTY(QKlipperFan *partsFan READ partsFan WRITE setPartsFan NOTIFY partsFanChanged FINAL)
    Q_PROPERTY(QMap<QString, QKlipperExtruder *> extruderMap READ extruderMap WRITE setExtruderMap NOTIFY extruderMapChanged FINAL)
    Q_PROPERTY(QString currentExtruderName READ currentExtruderName WRITE setCurrentExtruderName NOTIFY currentExtruderNameChanged FINAL)
    Q_PROPERTY(bool zHomed READ zHomed WRITE setZHomed NOTIFY zHomedChanged FINAL)
    Q_PROPERTY(bool yHomed READ yHomed WRITE setYHomed NOTIFY yHomedChanged FINAL)
    Q_PROPERTY(bool xHomed READ xHomed WRITE setXHomed NOTIFY xHomedChanged FINAL)
    Q_PROPERTY(bool isHoming READ isHoming WRITE setIsHoming NOTIFY isHomingChanged FINAL)
    Q_PROPERTY(qint32 maxAcceleration READ maxAcceleration WRITE setMaxAcceleration NOTIFY maxAccelerationChanged FINAL)
    Q_PROPERTY(qint32 maxVelocity READ maxVelocity WRITE setMaxVelocity NOTIFY maxVelocityChanged FINAL)
    Q_PROPERTY(qint32 maxAccelerationToDeceleration READ maxAccelerationToDeceleration WRITE setMaxAccelerationToDeceleration NOTIFY maxAccelerationToDecelerationChanged FINAL)
    Q_PROPERTY(qint32 stalls READ stalls WRITE setStalls NOTIFY stallsChanged FINAL)
    Q_PROPERTY(qint32 checks READ checks WRITE setChecks NOTIFY checksChanged FINAL)
    Q_PROPERTY(qreal squareCornerVelocity READ squareCornerVelocity WRITE setSquareCornerVelocity NOTIFY squareCornerVelocityChanged FINAL)
    Q_PROPERTY(QKlipperPosition position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QKlipperPosition destination READ destination WRITE setDestination NOTIFY destinationChanged FINAL)
    Q_PROPERTY(QKlipperPosition maxPosition READ maxPosition WRITE setMaxPosition NOTIFY maxPositionChanged FINAL)
    Q_PROPERTY(QKlipperPosition minPosition READ minPosition WRITE setMinPosition NOTIFY minPositionChanged FINAL)
};

#endif // QKLIPPERTOOLHEAD_H
