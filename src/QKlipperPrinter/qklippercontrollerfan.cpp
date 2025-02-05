#include <QKlipper/QKlipperPrinter/qklippercontrollerfan.h>

QKlipperControllerFan::QKlipperControllerFan(QObject *parent)
    : QKlipperFan(parent)
{

}

QString QKlipperControllerFan::stepper() const
{
    return m_stepper;
}

void QKlipperControllerFan::setStepper(const QString &stepper)
{
    if (m_stepper == stepper)
        return;
    m_stepper = stepper;
    emit stepperChanged();
}

qreal QKlipperControllerFan::idleTimeout() const
{
    return m_idleTimeout;
}

void QKlipperControllerFan::setIdleTimeout(qreal idleTimeout)
{
    if (qFuzzyCompare(m_idleTimeout, idleTimeout))
        return;
    m_idleTimeout = idleTimeout;
    emit idleTimeoutChanged();
}

qreal QKlipperControllerFan::idleSpeed() const
{
    return m_idleSpeed;
}

void QKlipperControllerFan::setIdleSpeed(qreal idleSpeed)
{
    if (qFuzzyCompare(m_idleSpeed, idleSpeed))
        return;
    m_idleSpeed = idleSpeed;
    emit idleSpeedChanged();
}

QString QKlipperControllerFan::heater() const
{
    return m_heater;
}

void QKlipperControllerFan::setHeater(const QString &heater)
{
    if (m_heater == heater)
        return;
    m_heater = heater;
    emit heaterChanged();
}

qreal QKlipperControllerFan::fanSpeed() const
{
    return m_fanSpeed;
}

void QKlipperControllerFan::setFanSpeed(qreal fanSpeed)
{
    if (qFuzzyCompare(m_fanSpeed, fanSpeed))
        return;
    m_fanSpeed = fanSpeed;
    emit fanSpeedChanged();
}
