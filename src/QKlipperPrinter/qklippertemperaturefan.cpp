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

qreal QKlipperTemperatureFan::minimumTemperatureerature() const
{
    return m_minimumTemperatureerature;
}

void QKlipperTemperatureFan::setMinTemperature(qreal minimumTemperatureerature)
{
    if (qFuzzyCompare(m_minimumTemperatureerature, minimumTemperatureerature))
        return;
    m_minimumTemperatureerature = minimumTemperatureerature;
    emit minimumTemperatureeratureChanged();
}

qreal QKlipperTemperatureFan::maximumTemperatureerature() const
{
    return m_maximumTemperatureerature;
}

void QKlipperTemperatureFan::setMaxTemperature(qreal maximumTemperatureerature)
{
    if (qFuzzyCompare(m_maximumTemperatureerature, maximumTemperatureerature))
        return;
    m_maximumTemperatureerature = maximumTemperatureerature;
    emit maximumTemperatureeratureChanged();
}

qreal QKlipperTemperatureFan::targetTemperatureerature() const
{
    return m_targetTemperatureerature;
}

void QKlipperTemperatureFan::setTargetTemperatureerature(qreal targetTemperatureerature)
{
    if (qFuzzyCompare(m_targetTemperatureerature, targetTemperatureerature))
        return;
    m_targetTemperatureerature = targetTemperatureerature;
    emit targetTemperatureeratureChanged();
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
