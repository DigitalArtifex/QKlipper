#ifndef QKLIPPERHEATER_H
#define QKLIPPERHEATER_H

#include <QObject>
#include "QKlipper/dalib_global.h"

class QKlipperConsole;
class QKlipperPrinter;

class DA_EXPORT QKlipperHeater : public QObject
{
    Q_OBJECT
    friend QKlipperConsole;
    friend QKlipperPrinter;

public:
    explicit QKlipperHeater(QObject *parent = nullptr);

    qreal currentTemp() const;

    qreal targetTemp() const;

    qreal power() const;

    qreal watts() const;

    qreal maxWatts() const;

    qreal inlineResistor() const;

    qreal maxPower() const;

    qreal maxTemp() const;

    qreal minTemp() const;

    qreal pidKD() const;

    qreal pidKI() const;

    qreal pidKP() const;

    qreal pullupResistor() const;

    qreal pwmCycleTime() const;

    qreal smoothTime() const;

    QString control() const;

    QString sensorPin() const;

    QString heaterPin() const;

    QString sensorType() const;

    QString name() const;

    QString gcodeId() const;

public slots:
    /*!
     * Sends a gcode script to set the requested temperature
     *
     * \param targetTemp The value (in Celsius) to set (0-maxTemp)
     */
    virtual void setTargetTemp(qreal targetTemp);

    /*!
     * Sends a gcode script to calibrate the extruder at the target temperature.
     *
     * \param target The temperature to use in the PID calibration
     */
    virtual void calibratePid(qreal target);

    void setMaxWatts(qreal maxWatts);

private slots:
    void setName(const QString &name);
    void setGcodeId(const QString &gcodeId);
    void setCurrentTemp(qreal currentTemp);
    void setTargetTempData(qreal targetTemp);
    void setPower(qreal power);
    void setWatts(qreal watts);
    void setInlineResistor(qreal inlineResistor);
    void setMaxPower(qreal maxPower);
    void setMaxTemp(qreal maxTemp);
    void setMinTemp(qreal minTemp);
    void setPidKD(qreal pidKD);
    void setPidKI(qreal pidKI);
    void setPidKP(qreal pidKP);
    void setPullupResistor(qreal pullupResistor);
    void setPwmCycleTime(qreal pwmCycleTime);
    void setSmoothTime(qreal smoothTime);
    void setControl(const QString &control);
    void setSensorPin(const QString &sensorPin);
    void setHeaterPin(const QString &heaterPin);
    void setSensorType(const QString &sensorType);

signals:
    void currentTempChanged();
    void targetTempChanged();
    void powerChanged();
    void wattsChanged();
    void maxWattsChanged();
    void inlineResistorChanged();
    void maxPowerChanged();
    void maxTempChanged();
    void minTempChanged();
    void pidKDChanged();
    void pidKIChanged();
    void pidKPChanged();
    void pullupResistorChanged();
    void pwmCycleTimeChanged();
    void smoothTimeChanged();
    void controlChanged();
    void sensorPinChanged();
    void heaterPinChanged();
    void sensorTypeChanged();
    void pidCalibrating();
    void pidCalibratingFinished();

    void nameChanged();

    void gcodeIdChanged();

protected:
    QKlipperConsole *m_console = nullptr;

private:
    qreal m_currentTemp = 0;
    qreal m_targetTemp = 0;
    qreal m_power = 0;
    qreal m_watts = 0;
    qreal m_maxWatts = 0;
    qreal m_inlineResistor = 0;
    qreal m_maxPower = 0;
    qreal m_maxTemp= 0;
    qreal m_minTemp = 0;
    qreal m_pidKD = 0;
    qreal m_pidKI = 0;
    qreal m_pidKP = 0;
    qreal m_pullupResistor = 0;
    qreal m_pwmCycleTime = 0;
    qreal m_smoothTime = 0;

    QString m_control;
    QString m_heaterPin;
    QString m_sensorPin;
    QString m_sensorType;
    QString m_name;
    QString m_gcodeId;

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString sensorType READ sensorType WRITE setSensorType NOTIFY sensorTypeChanged FINAL)
    Q_PROPERTY(QString sensorPin READ sensorPin WRITE setSensorPin NOTIFY sensorPinChanged FINAL)
    Q_PROPERTY(QString heaterPin READ heaterPin WRITE setHeaterPin NOTIFY heaterPinChanged FINAL)
    Q_PROPERTY(QString control READ control WRITE setControl NOTIFY controlChanged FINAL)
    Q_PROPERTY(qreal currentTemp READ currentTemp WRITE setCurrentTemp NOTIFY currentTempChanged FINAL)
    Q_PROPERTY(qreal targetTemp READ targetTemp WRITE setTargetTemp NOTIFY targetTempChanged FINAL)
    Q_PROPERTY(QString gcodeId READ gcodeId WRITE setGcodeId NOTIFY gcodeIdChanged FINAL)
};

#endif // QKLIPPERHEATER_H
