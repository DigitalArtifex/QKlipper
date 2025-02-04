#ifndef QKLIPPERTEMPERATUREFAN_H
#define QKLIPPERTEMPERATUREFAN_H

#include "qklipperfan.h"

class QKlipperPrinter;
class QKlipperConsole;

//!  QKlipperFan class
/*!
  This class is responsible for monitoring and controlling fan objects.

  Not all fans are controllable.
*/

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperTemperatureFan : public QKlipperFan
{
    Q_OBJECT
    friend QKlipperConsole;
    friend QKlipperPrinter;

public:
    explicit QKlipperTemperatureFan(QObject *parent);

    QString sensorPin() const;

    QString sensorType() const;

    QString gcodeId() const;

    qreal maxDelta() const;

    qreal minTemperature() const;

    qreal maxTemperature() const;

    qreal targetTemperature() const;

    qreal pidKp() const;

    qreal pidKi() const;

    qreal pidKd() const;

    qreal pidDerivationTime() const;

    qreal minSpeed() const;

    qreal maxSpeed() const;

private slots:
    void setSensorPin(const QString &sensorPin);

    void setSensorType(const QString &sensorType);

    void setGcodeId(const QString &gcodeId);

    void setMaxDelta(qreal maxDelta);

    void setMinTemperature(qreal minTemperature);

    void setMaxTemperature(qreal maxTemperature);

    void setTargetTemperature(qreal targetTemperature);

    void setPidKp(qreal pidKp);

    void setPidKi(qreal pidKi);

    void setPidKd(qreal pidKd);

    void setPidDerivationTime(qreal pidDerivationTime);

    void setMinSpeed(qreal minSpeed);

    void setMaxSpeed(qreal maxSpeed);

signals:
    void sensorPinChanged();

    void sensorTypeChanged();

    void gcodeIdChanged();

    void maxDeltaChanged();

    void minTemperatureChanged();

    void maxTemperatureChanged();

    void targetTemperatureChanged();

    void pidKpChanged();

    void pidKiChanged();

    void pidKdChanged();

    void pidDerivationTimeChanged();

    void minSpeedChanged();

    void maxSpeedChanged();

private:
    QString m_sensorPin;
    QString m_sensorType;
    QString m_gcodeId;

    qreal m_maxDelta = 0;
    qreal m_minTemperature = 0;
    qreal m_maxTemperature = 0;
    qreal m_targetTemperature = 0;

    qreal m_pidKp = 0;
    qreal m_pidKi = 0;
    qreal m_pidKd = 0;
    qreal m_pidDerivationTime = 0;

    qreal m_minSpeed = 0;
    qreal m_maxSpeed = 0;

    Q_PROPERTY(QString sensorPin READ sensorPin WRITE setSensorPin NOTIFY sensorPinChanged FINAL)
    Q_PROPERTY(QString sensorType READ sensorType WRITE setSensorType NOTIFY sensorTypeChanged FINAL)
    Q_PROPERTY(QString gcodeId READ gcodeId WRITE setGcodeId NOTIFY gcodeIdChanged FINAL)
    Q_PROPERTY(qreal maxDelta READ maxDelta WRITE setMaxDelta NOTIFY maxDeltaChanged FINAL)
    Q_PROPERTY(qreal minTemperature READ minTemperature WRITE setMinTemperature NOTIFY minTemperatureChanged FINAL)
    Q_PROPERTY(qreal maxTemperature READ maxTemperature WRITE setMaxTemperature NOTIFY maxTemperatureChanged FINAL)
    Q_PROPERTY(qreal targetTemperature READ targetTemperature WRITE setTargetTemperature NOTIFY targetTemperatureChanged FINAL)
    Q_PROPERTY(qreal pidKp READ pidKp WRITE setPidKp NOTIFY pidKpChanged FINAL)
    Q_PROPERTY(qreal pidKi READ pidKi WRITE setPidKi NOTIFY pidKiChanged FINAL)
    Q_PROPERTY(qreal pidKd READ pidKd WRITE setPidKd NOTIFY pidKdChanged FINAL)
    Q_PROPERTY(qreal pidDerivationTime READ pidDerivationTime WRITE setPidDerivationTime NOTIFY pidDerivationTimeChanged FINAL)
    Q_PROPERTY(qreal minSpeed READ minSpeed WRITE setMinSpeed NOTIFY minSpeedChanged FINAL)
    Q_PROPERTY(qreal maxSpeed READ maxSpeed WRITE setMaxSpeed NOTIFY maxSpeedChanged FINAL)
};

#endif // QKLIPPERTEMPERATUREFAN_H
