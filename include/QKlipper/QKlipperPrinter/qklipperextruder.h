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

#ifndef QKLIPPEREXTRUDER_H
#define QKLIPPEREXTRUDER_H

#include <QObject>
#include <QList>

#include "qklipperposition.h"
#include "qklipperfan.h"
#include "qklipperheater.h"

#include "qklippertemperaturestore.h"

typedef QPair<qint32,qint32> QKlipperGearRatio;

class QKlipperConsole;
class QKlipperToolHead;

//!  QKlipperExtruder class
/*!
  This class provides various methods for monitoring and controlling the state of an extruder
  registered in the klipper instance.
*/

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperExtruder : public QKlipperHeater
{
    Q_OBJECT

    friend QKlipperConsole;
    friend QKlipperToolHead;
public:

    /*!
     * Constructor
     *
     * \param parent The parent object
     */
    explicit QKlipperExtruder(QObject *parent = nullptr);

    /*!
     * Destructor
     */
    ~QKlipperExtruder();

    /*!
     * The fan object associated with the extruder is determined during
     * the QKlipperConsole startup sequence. This object will change at least once,
     * so be sure to subscribe to it's changed signal.
     *
     * \returns The current fan object
     */
    QKlipperFan *fan() const;

    /*!
     * Toolhead index of the extruder object
     *
     * \returns The index of the extruder
     */
    qint32 extruderNumber() const;

    /*!
     * The gear ratio of the extruder
     *
     * \returns A QKlipperGearRatio object
     */
    QKlipperGearRatio gearRatio() const;

    /*!
     * The number of steps it takes the stepper to complete 1 full 360 degree rotation
     *
     * \returns The current steps-per-rotation
     */
    qint32 fullStepsPerRotation() const;

    /*!
     * Current number of microsteps
     *
     * \returns The current console object
     */
    qint32 microsteps() const;

    /*!
     * Current pressure advance value
     *
     * \returns The current pressure advance
     */
    qreal pressureAdvance() const;

    /*!
     * Current pressure advance smooth time value
     *
     * \returns The current pressure advance smooth time
     */
    qreal pressureAdvanceSmoothTime() const;

    /*!
     * Current extrusion factor (flow rate) of the extruder
     *
     * \returns The current extrusion factor
     */
    qreal extrusionFactor() const;

    /*!
     * Current filament diameter for the extruder
     *
     * \returns The current filament diameter
     */
    qreal filamentDiameter() const;

    /*!
     * Current nozzle diameter for the extruder
     *
     * \returns The current nozzle diameter
     */
    qreal nozzleDiameter() const;

    /*!
     * Inline resistor value set in the configuration
     *
     * \returns The current inline resistor value
     */
    qreal inlineResistor() const;

    /*!
     * Pullup resistor value set in the configuration
     *
     * \returns The current pullup resistor value
     */
    qreal pullupResistor() const;

    /*!
     * Rotation distance value set in the configuration
     *
     * \returns The current Rotation distance value
     */
    qreal rotationDistance() const;

    /*!
     * Rotation distance value set in the configuration
     *
     * \returns The current Rotation distance value
     */
    qreal instantCornerVelocity() const;

    /*!
     * Max extrude cross section value set in the configuration
     *
     * \returns The current Max extrude cross section value
     */
    qreal maxExtrudeCrossSection() const;

    /*!
     * Max extrude only acceleration value set in the configuration
     *
     * \returns The current Max extrude only acceleration value
     */
    qreal maxExtrudeOnlyAcceleration() const;

    /*!
     * Max extrude only distance value set in the configuration
     *
     * \returns The current Max extrude only distance value
     */
    qreal maxExtrudeOnlyDistance() const;

    /*!
     * Max extrude only velocity value set in the configuration
     *
     * \returns The current Max extrude only velocity value
     */
    qreal maxExtrudeOnlyVelocity() const;

    /*!
     * Minimum temperature the extruder can extrude at
     *
     * \returns The minimum extrusion temperature
     */
    qreal minExtrudeTemp() const;

    /*!
     * If the extruder can extrude material or not
     *
     * \returns True if the extruder is ready to extrude
     */
    bool canExtrude() const;

    /*!
     * Name for the extruder (extruder, extruder1, etc)
     *
     * \returns The name of the extruder
     */
    QString name() const;

    /*!
     * Direction pin for the extruder, set in the configuration
     *
     * \returns The direction pin label of the extruder
     */
    QString dirPin() const;

    /*!
     * Enable pin for the extruder, set in the configuration
     *
     * \returns The enable pin label of the extruder
     */
    QString enablePin() const;

    /*!
     * Step pin for the extruder, set in the configuration
     *
     * \returns The step pin label of the extruder
     */
    QString stepPin() const;

    /*!
     * The temperature store provides a history of the sampled
     * temperatures.
     *
     * \returns The temperature store object
     */
    QKlipperTemperatureStore temperatureStore() const;

    /*!
     * Offset value for the extruder as set in the configuration
     *
     * \returns The current offset value
     */
    QKlipperPosition offset() const;

    /*!
     * The console object being used to send
     *
     * \returns The console object
     */
    QKlipperConsole *console() const;

public slots:

    /*!
     * Sends a gcode script to set the requested pressure advance. Also sends
     * the currently configured smooth time value.
     *
     * \param pressureAdvance The pressure advance value to set
     */
    void setPressureAdvance(qreal pressureAdvance);

    /*!
     * Sends a gcode script to set the requested pressure advance smooth time.
     *
     * \param pressureAdvanceSmoothTime The value to set
     */
    void setPressureAdvanceSmoothTime(qreal pressureAdvanceSmoothTime);

    /*!
     * Sends a gcode script to set the requested extrusion factor (flow rate)
     *
     * \param extrusionFactor The value to set (0.5 is 50%)
     */
    void setExtrusionFactor(qreal extrusionFactor);

    /*!
     * Sends a gcode script to extrude the requested length of material
     * at the requested speed
     *
     * \param amount The amount to extrude (in mm)
     * \param speed The speed of the extrusion
     */
    void extrude(qreal amount, qreal speed);

    /*!
     * Sends a gcode script to retract the requested length of material
     * at the requested speed
     *
     * \param amount The amount to extrude (in mm)
     * \param speed The speed of the extrusion
     */
    void retract(qreal amount, qreal speed);

private slots:

    //TODO: rename all functions here to end with Data
    void setGearRatio(const QKlipperGearRatio &gearRatio);
    void setFullStepsPerRotation(qint32 fullStepsPerRotation);
    void setMicrosteps(qint32 microsteps);
    void setFan(QKlipperFan *fan);

    void setExtruderNumber(qint32 extruderNumber);

    void setPressureAdvanceData(qreal pressureAdvance);
    void setPressureAdvanceSmoothTimeData(qreal pressureAdvanceSmoothTime);
    void setExtrusionFactorData(qreal extrusionFactor);

    void setFilamentDiameter(qreal filamentDiameter);

    void setNozzleDiameter(qreal nozzleDiameter);

    void setInlineResistor(qreal inlineResistor);

    void setPullupResistor(qreal pullupResistor);

    void setRotationDistance(qreal rotationDistance);

    void setInstantCornerVelocity(qreal instantCornerVelocity);

    void setMaxExtrudeCrossSection(qreal maxExtrudeCrossSection);

    void setMaxExtrudeOnlyAcceleration(qreal maxExtrudeOnlyAcceleration);

    void setMaxExtrudeOnlyDistance(qreal maxExtrudeOnlyDistance);

    void setMaxExtrudeOnlyVelocity(qreal maxExtrudeOnlyVelocity);

    void setMinExtrudeTemp(qreal minExtrudeTemp);

    void setCanExtrude(bool canExtrude);

    void setName(const QString &name);

    void setDirPin(const QString &dirPin);

    void setEnablePin(const QString &enablePin);

    void setStepPin(const QString &stepPin);

    void setTemperatureStore(const QKlipperTemperatureStore &temperatureStore);
    void setTemperatureStoreValue(const QKlipperTemperatureStoreValue &value);

    void setOffsetData(const QKlipperPosition &offset);

    void setConsole(QKlipperConsole *console);

signals:

    void fanChanged();

    void extruderNumberChanged();

    void gearRatioChanged();

    void fullStepsPerRotationChanged();

    void microstepsChanged();

    void pressureAdvanceChanged();

    void pressureAdvanceSmoothTimeChanged();

    void extrusionFactorChanged();

    void filamentDiameterChanged();

    void nozzleDiameterChanged();

    void inlineResistorChanged();

    void pullupResistorChanged();

    void rotationDistanceChanged();

    void instantCornerVelocityChanged();

    void maxExtrudeCrossSectionChanged();

    void maxExtrudeOnlyAccelerationChanged();

    void maxExtrudeOnlyDistanceChanged();

    void maxExtrudeOnlyVelocityChanged();

    void minExtrudeTempChanged();

    void canExtrudeChanged();

    void nameChanged();

    void dirPinChanged();

    void enablePinChanged();

    void stepPinChanged();

    void printerChanged();

    void offsetChanged();

    void consoleChanged();

    void temperatureStoreChanged();

private:
    QKlipperPosition     m_offset;
    QKlipperFan         *m_fan;

    QKlipperGearRatio m_gearRatio;

    qint32 m_extruderNumber = 0;
    qint32 m_fullStepsPerRotation = 0;
    qint32 m_microsteps = 0;
    qreal m_pressureAdvance = 0;
    qreal m_pressureAdvanceSmoothTime = 0;
    qreal m_extrusionFactor = 0;
    qreal m_filamentDiameter = 0;
    qreal m_nozzleDiameter = 0;
    qreal m_inlineResistor = 0;
    qreal m_pullupResistor = 0;
    qreal m_rotationDistance = 0;
    qreal m_instantCornerVelocity = 0;
    qreal m_minExtrudeTemp = 0;

    //maximums
    qreal m_maxExtrudeCrossSection = 0;
    qreal m_maxExtrudeOnlyAcceleration = 0;
    qreal m_maxExtrudeOnlyDistance = 0;
    qreal m_maxExtrudeOnlyVelocity = 0;

    bool m_canExtrude = false;

    QString m_name;
    QString m_dirPin;
    QString m_enablePin;
    QString m_stepPin;

    QKlipperTemperatureStore m_temperatureStore;

    //Parent printer object
    QKlipperConsole *m_console = nullptr;
};

#endif // QKLIPPEREXTRUDER_H
