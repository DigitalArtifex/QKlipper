#include <QKlipper/QKlipperPrinter/qklippertemperaturefan.h>

QKlipperTemperatureFan::QKlipperTemperatureFan(QObject *parent)
    : QKlipperFan(parent)
{

}

QString QKlipperTemperatureFan::sensorPin() const
{
    return m_sensorPin;
}

void QKlipperTemperatureFan::setSensorPin(const QString &sensorPin)
{
    if (m_sensorPin == sensorPin)
        return;

    m_sensorPin = sensorPin;
    emit sensorPinChanged();
}

QString QKlipperTemperatureFan::sensorType() const
{
    return m_sensorType;
}

void QKlipperTemperatureFan::setSensorType(const QString &sensorType)
{
    if (m_sensorType == sensorType)
        return;
    m_sensorType = sensorType;
    emit sensorTypeChanged();
}

QString QKlipperTemperatureFan::gcodeId() const
{
    return m_gcodeId;
}

void QKlipperTemperatureFan::setGcodeId(const QString &gcodeId)
{
    if (m_gcodeId == gcodeId)
        return;
    m_gcodeId = gcodeId;
    emit gcodeIdChanged();
}

qreal QKlipperTemperatureFan::maxDelta() const
{
    return m_maxDelta;
}

void QKlipperTemperatureFan::setMaxDelta(qreal maxDelta)
{
    if (qFuzzyCompare(m_maxDelta, maxDelta))
        return;
    m_maxDelta = maxDelta;
    emit maxDeltaChanged();
}

qreal QKlipperTemperatureFan::minTemperature() const
{
    return m_minTemperature;
}

void QKlipperTemperatureFan::setMinTemperature(qreal minTemperature)
{
    if (qFuzzyCompare(m_minTemperature, minTemperature))
        return;
    m_minTemperature = minTemperature;
    emit minTemperatureChanged();
}

qreal QKlipperTemperatureFan::maxTemperature() const
{
    return m_maxTemperature;
}

void QKlipperTemperatureFan::setMaxTemperature(qreal maxTemperature)
{
    if (qFuzzyCompare(m_maxTemperature, maxTemperature))
        return;
    m_maxTemperature = maxTemperature;
    emit maxTemperatureChanged();
}

qreal QKlipperTemperatureFan::targetTemperature() const
{
    return m_targetTemperature;
}

void QKlipperTemperatureFan::setTargetTemperature(qreal targetTemperature)
{
    if (qFuzzyCompare(m_targetTemperature, targetTemperature))
        return;
    m_targetTemperature = targetTemperature;
    emit targetTemperatureChanged();
}

qreal QKlipperTemperatureFan::pidKp() const
{
    return m_pidKp;
}

void QKlipperTemperatureFan::setPidKp(qreal pidKp)
{
    if (qFuzzyCompare(m_pidKp, pidKp))
        return;
    m_pidKp = pidKp;
    emit pidKpChanged();
}

qreal QKlipperTemperatureFan::pidKi() const
{
    return m_pidKi;
}

void QKlipperTemperatureFan::setPidKi(qreal pidKi)
{
    if (qFuzzyCompare(m_pidKi, pidKi))
        return;
    m_pidKi = pidKi;
    emit pidKiChanged();
}

qreal QKlipperTemperatureFan::pidKd() const
{
    return m_pidKd;
}

void QKlipperTemperatureFan::setPidKd(qreal pidKd)
{
    if (qFuzzyCompare(m_pidKd, pidKd))
        return;
    m_pidKd = pidKd;
    emit pidKdChanged();
}

qreal QKlipperTemperatureFan::pidDerivationTime() const
{
    return m_pidDerivationTime;
}

void QKlipperTemperatureFan::setPidDerivationTime(qreal pidDerivationTime)
{
    if (qFuzzyCompare(m_pidDerivationTime, pidDerivationTime))
        return;

    m_pidDerivationTime = pidDerivationTime;
    emit pidDerivationTimeChanged();
}

qreal QKlipperTemperatureFan::minSpeed() const
{
    return m_minSpeed;
}

void QKlipperTemperatureFan::setMinSpeed(qreal minSpeed)
{
    if (qFuzzyCompare(m_minSpeed, minSpeed))
        return;
    m_minSpeed = minSpeed;
    emit minSpeedChanged();
}

qreal QKlipperTemperatureFan::maxSpeed() const
{
    return m_maxSpeed;
}

void QKlipperTemperatureFan::setMaxSpeed(qreal maxSpeed)
{
    if (qFuzzyCompare(m_maxSpeed, maxSpeed))
        return;
    m_maxSpeed = maxSpeed;
    emit maxSpeedChanged();
}
