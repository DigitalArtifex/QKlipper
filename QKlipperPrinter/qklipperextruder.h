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

#ifndef QKLIPPEREXTRUDER_H
#define QKLIPPEREXTRUDER_H

#include <QObject>
#include <QList>

#include "qklipperposition.h"
#include "qklipperfan.h"

#include "qklippertemperaturestore.h"

typedef QPair<qint32,qint32> QKlipperGearRatio;

class QKlipperConsole;
class QKlipperToolHead;

class QKlipperExtruder : public QObject
{
    Q_OBJECT

    friend QKlipperConsole;
    friend QKlipperToolHead;
public:
    explicit QKlipperExtruder(QObject *parent = nullptr);
    ~QKlipperExtruder();

    QKlipperFan *fan() const;

    qint32 extruderNumber() const;

    QKlipperGearRatio gearRatio() const;

    qint32 fullStepsPerRotation() const;

    qint32 microsteps() const;

    qreal currentTemp() const;

    qreal targetTemp() const;

    qreal smoothTime() const;

    qreal pressureAdvance() const;

    qreal pressureAdvanceSmoothTime() const;

    qreal extrusionFactor() const;

    qreal filamentDiameter() const;

    qreal nozzleDiameter() const;

    qreal power() const;

    qreal watts() const;

    qreal maxWatts() const;

    qreal inlineResistor() const;

    qreal pullupResistor() const;

    qreal pwmCycle() const;

    qreal rotationDistance() const;

    qreal instantCornerVelocity() const;

    qreal pidKD() const;

    qreal pidKI() const;

    qreal pidKP() const;

    qreal maxExtrudeCrossSection() const;

    qreal maxExtrudeOnlyAcceleration() const;

    qreal maxExtrudeOnlyDistance() const;

    qreal maxExtrudeOnlyVelocity() const;

    qreal maxTemp() const;

    qreal maxPower() const;

    qreal minExtrudeTemp() const;

    qreal minTemp() const;

    bool canExtrude() const;

    QString name() const;

    QString control() const;

    QString dirPin() const;

    QString enablePin() const;

    QString heaterPin() const;

    QString sensorPin() const;

    QString sensorType() const;

    QString stepPin() const;

    QKlipperTemperatureStore temperatureStore() const;

    QKlipperPrinter *printer() const;

    QKlipperPosition offset() const;

    QKlipperConsole *console() const;

public slots:

    void setGearRatio(const QKlipperGearRatio &gearRatio);
    void setFullStepsPerRotation(qint32 fullStepsPerRotation);
    void setMicrosteps(qint32 microsteps);
    void setTargetTemp(qreal targetTemp);
    void setSmoothTime(qreal smoothTime);
    void setPressureAdvance(qreal pressureAdvance);
    void setPressureAdvanceSmoothTime(qreal pressureAdvanceSmoothTime);
    void setExtrusionFactor(qreal extrusionFactor);
    void extrude(qreal amount, qreal speed);
    void pidTune(qreal target);

private slots:

    void setFan(QKlipperFan *fan);

    void setExtruderNumber(qint32 extruderNumber);

    void setCurrentTemp(qreal currentTemp);
    void setPressureAdvanceData(qreal pressureAdvance);
    void setSmoothTimeData(qreal smoothTime);
    void setPressureAdvanceSmoothTimeData(qreal pressureAdvanceSmoothTime);
    void setTargetTempData(qreal targetTemp);
    void setExtrusionFactorData(qreal extrusionFactor);

    void setFilamentDiameter(qreal filamentDiameter);

    void setNozzleDiameter(qreal nozzleDiameter);

    void setPower(qreal power);

    void setWatts(qreal watts);

    void setMaxWatts(qreal maxWatts);

    void setInlineResistor(qreal inlineResistor);

    void setPullupResistor(qreal pullupResistor);

    void setPwmCycle(qreal pwmCycle);

    void setRotationDistance(qreal rotationDistance);

    void setInstantCornerVelocity(qreal instantCornerVelocity);

    void setPidKD(qreal pidKD);

    void setPidKI(qreal pidKI);

    void setPidKP(qreal pidKP);

    void setMaxExtrudeCrossSection(qreal maxExtrudeCrossSection);

    void setMaxExtrudeOnlyAcceleration(qreal maxExtrudeOnlyAcceleration);

    void setMaxExtrudeOnlyDistance(qreal maxExtrudeOnlyDistance);

    void setMaxExtrudeOnlyVelocity(qreal maxExtrudeOnlyVelocity);

    void setMaxTemp(qreal maxTemp);

    void setMaxPower(qreal maxPower);

    void setMinExtrudeTemp(qreal minExtrudeTemp);

    void setMinTemp(qreal minTemp);

    void setCanExtrude(bool canExtrude);

    void setName(const QString &name);

    void setControl(const QString &control);

    void setDirPin(const QString &dirPin);

    void setEnablePin(const QString &enablePin);

    void setHeaterPin(const QString &heaterPin);

    void setSensorPin(const QString &sensorPin);

    void setSensorType(const QString &sensorType);

    void setStepPin(const QString &stepPin);

    void setTemperatureStore(const QKlipperTemperatureStore &temperatureStore);
    void setTemperatureStoreValue(const QKlipperTemperatureStoreValue &value);

    void setPrinter(QKlipperPrinter *printer);

    void setOffset(const QKlipperPosition &offset);

    void setConsole(QKlipperConsole *console);

signals:

    void fanChanged();

    void extruderNumberChanged();

    void gearRatioChanged();

    void fullStepsPerRotationChanged();

    void microstepsChanged();

    void currentTempChanged();

    void targetTempChanged();

    void smoothTimeChanged();

    void pressureAdvanceChanged();

    void pressureAdvanceSmoothTimeChanged();

    void extrusionFactorChanged();

    void filamentDiameterChanged();

    void nozzleDiameterChanged();

    void powerChanged();

    void wattsChanged();

    void maxWattsChanged();

    void inlineResistorChanged();

    void pullupResistorChanged();

    void pwmCycleChanged();

    void rotationDistanceChanged();

    void instantCornerVelocityChanged();

    void pidKDChanged();

    void pidKIChanged();

    void pidKPChanged();

    void maxExtrudeCrossSectionChanged();

    void maxExtrudeOnlyAccelerationChanged();

    void maxExtrudeOnlyDistanceChanged();

    void maxExtrudeOnlyVelocityChanged();

    void maxTempChanged();

    void maxPowerChanged();

    void minExtrudeTempChanged();

    void minTempChanged();

    void canExtrudeChanged();

    void nameChanged();

    void controlChanged();

    void dirPinChanged();

    void enablePinChanged();

    void heaterPinChanged();

    void sensorPinChanged();

    void sensorTypeChanged();

    void stepPinChanged();

    void temperatureStoreChanged();

    void printerChanged();

    void offsetChanged();

    void consoleChanged();

private:
    QKlipperPosition    m_offset;
    QKlipperFan         *m_fan;

    QKlipperGearRatio m_gearRatio;

    qint32 m_extruderNumber = 0;
    qint32 m_fullStepsPerRotation = 0;
    qint32 m_microsteps = 0;

    qreal m_currentTemp = 0;
    qreal m_targetTemp = 0;
    qreal m_smoothTime = 0;
    qreal m_pressureAdvance = 0;
    qreal m_pressureAdvanceSmoothTime = 0;
    qreal m_extrusionFactor = 0;
    qreal m_filamentDiameter = 0;
    qreal m_nozzleDiameter = 0;
    qreal m_power = 0;
    qreal m_watts = 0;
    qreal m_maxWatts;
    qreal m_inlineResistor = 0;
    qreal m_pullupResistor = 0;
    qreal m_pwmCycle = 0;
    qreal m_rotationDistance = 0;
    qreal m_instantCornerVelocity = 0;

    //pid settings
    qreal m_pidKD = 0;
    qreal m_pidKI = 0;
    qreal m_pidKP = 0;

    //maximums
    qreal m_maxExtrudeCrossSection = 0;
    qreal m_maxExtrudeOnlyAcceleration = 0;
    qreal m_maxExtrudeOnlyDistance = 0;
    qreal m_maxExtrudeOnlyVelocity = 0;
    qreal m_maxTemp = 0;
    qreal m_maxPower = 0;

    //minimums
    qreal m_minExtrudeTemp = 0;
    qreal m_minTemp = 0;

    bool m_canExtrude = false;

    QString m_name;
    QString m_control;
    QString m_dirPin;
    QString m_enablePin;
    QString m_heaterPin;
    QString m_sensorPin;
    QString m_sensorType;
    QString m_stepPin;

    QKlipperTemperatureStore m_temperatureStore;

    //Parent printer object
    QKlipperPrinter *m_printer = nullptr;
    QKlipperConsole *m_console = nullptr;
};

#endif // QKLIPPEREXTRUDER_H
