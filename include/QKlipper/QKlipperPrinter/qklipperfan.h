/*!
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

#ifndef QKLIPPERFAN_H
#define QKLIPPERFAN_H

#include <QObject>

class QKlipperPrinter;
class QKlipperConsole;

//!  QKlipperFan class
/*!
  This class is responsible for monitoring and controlling fan objects.

  Not all fans are controllable.
*/

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperFan : public QObject
{
    Q_OBJECT

    friend QKlipperConsole;
    friend QKlipperPrinter;
public:
    //!  State enum
    /*!
      This enum provides flags for the on/off states
    */
    enum State
    {
        Off = 0, /*!!< Fan is off */
        On = 1 /*!!< Fan is on */
    };

    struct Tachometer
    {
        QString pin;
        qreal pollInterval = 0;
        qint8 pulsesPerRotation = 0;
        bool enabled = false;
    };

    /*!
     * Constructor
     *
     * \param parent The parent object
     */
    explicit QKlipperFan(QObject *parent = nullptr);

    /*!
     * Name of the fan object
     *
     * \returns The current name of the fan
     */
    QString name() const;

    /*!
     * Current speed of the fan
     *
     * \returns The current speed of the fan
     */
    qreal speed() const;

    /*!
     * Current RPM
     *
     * \returns The current RPM of the fan (if available)
     */
    qreal rpm() const;

    /*!
     * Controllable state of the fan object
     *
     * \returns True if the fan can be controlled
     */
    bool isControllable() const;

    QString pin() const;

    QString enablePin() const;

    qreal maxPower() const;

    qreal shutdownSpeed() const;

    qreal cycleTime() const;

    qreal kickStartTime() const;

    qreal offBelow() const;

    bool hardwarePin() const;

    Tachometer tachometer() const;

public slots:
    /*!
     * Sends a gcode script to set the requested speed
     *
     * \param speed The value to set (0-100)
     */
    void setSpeed(qreal speed);

    void setTachometer(const Tachometer &tachometer);

private slots:
    /*!
     * Sets the name value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     *
     * \param name Name of the fan object
     */
    void setNameData(const QString &name);

    /*!
     * Resets the name value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     */
    void resetName();

    /*!
     * Sets the speed value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     *
     * \param speed Speed of the fan object
     */
    void setSpeedData(qreal speed);

    /*!
     * Resets the speed value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     */
    void resetSpeed();

    /*!
     * Sets the rpm value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     *
     * \param rpm RPM of the fan object
     */
    void setRpmData(qreal rpm);

    /*!
     * Resets the rpm value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     */
    void resetRpm();

    /*!
     * Sets the controllable value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     *
     * \param isControllable State of controllability
     */
    void setIsControllable(bool isControllable);

    /*!
     * Resets the controllable value of the fan object ONLY. It does not
     * send any klipper commands. Primarily used by QKlipperConsole
     */
    void resetIsControllable();

    void setPin(const QString &pin);

    void setEnablePin(const QString &enablePin);

    void setMaxPower(qreal maxPower);

    void setShutdownSpeed(qreal shutdownSpeed);

    void setCycleTime(qreal cycleTime);

    void setKickStartTime(qreal kickStartTime);

    void setOffBelow(qreal offBelow);

    void setHardwarePin(bool hardwarePin);

signals:

    /*!
     * Signal fired when setNameData has been called with new data
     */
    void nameChanged();

    //Signal fired when setSpeedData has been called with new data
    void speedChanged();

    //Signal fired when setRpmData has been called with new data
    void rpmChanged();

    //Signal fired when setIsControllableData has been called with new data
    void isControllableChanged();

    void pinChanged();

    void enablePinChanged();

    void maxPowerChanged();

    void shutdownSpeedChanged();

    void cycleTimeChanged();

    void kickStartTimeChanged();

    void offBelowChanged();

    void hardwarePinChanged();

    void tachometerChanged();

private:
    QString m_name;
    QString m_pin;
    QString m_enablePin;

    qreal m_speed = 0;
    qreal m_rpm = 0;
    qreal m_maxPower = 0;
    qreal m_shutdownSpeed = 0;
    qreal m_cycleTime = 0;
    qreal m_kickStartTime = 0;
    qreal m_offBelow = 0;

    bool m_isControllable = 0;
    bool m_hardwarePin = false;

    QKlipperConsole *m_console = nullptr;

    Tachometer m_tachometer;

    Q_PROPERTY(QString name READ name RESET resetName NOTIFY nameChanged FINAL)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed RESET resetSpeed NOTIFY speedChanged FINAL)
    Q_PROPERTY(qreal rpm READ rpm RESET resetRpm NOTIFY rpmChanged FINAL)
    Q_PROPERTY(bool isControllable READ isControllable RESET resetIsControllable NOTIFY isControllableChanged FINAL)
    Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY pinChanged FINAL)
    Q_PROPERTY(QString enablePin READ enablePin WRITE setEnablePin NOTIFY enablePinChanged FINAL)
    Q_PROPERTY(qreal maxPower READ maxPower WRITE setMaxPower NOTIFY maxPowerChanged FINAL)
    Q_PROPERTY(qreal shutdownSpeed READ shutdownSpeed WRITE setShutdownSpeed NOTIFY
                   shutdownSpeedChanged FINAL)
    Q_PROPERTY(qreal cycleTime READ cycleTime WRITE setCycleTime NOTIFY cycleTimeChanged FINAL)
    Q_PROPERTY(qreal kickStartTime READ kickStartTime WRITE setKickStartTime NOTIFY
                   kickStartTimeChanged FINAL)
    Q_PROPERTY(qreal offBelow READ offBelow WRITE setOffBelow NOTIFY offBelowChanged FINAL)
    Q_PROPERTY(bool hardwarePin READ hardwarePin WRITE setHardwarePin NOTIFY hardwarePinChanged FINAL)
    Q_PROPERTY(
        Tachometer tachometer READ tachometer WRITE setTachometer NOTIFY tachometerChanged FINAL)
};

#endif // QKLIPPERFAN_H
