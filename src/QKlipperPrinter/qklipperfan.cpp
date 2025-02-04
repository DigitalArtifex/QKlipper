#include <QKlipper/QKlipperPrinter/qklipperfan.h>

#include <QKlipper/QKlipperConsole/qklipperconsole.h>

QKlipperFan::QKlipperFan(QObject *parent)
    : QObject{parent}
{
    QKlipperPrinter *printer = qobject_cast<QKlipperPrinter*>(parent);

    if(printer)
    {
        QKlipperConsole *console = qobject_cast<QKlipperConsole*>(parent);

        if(console)
            m_console = console;
    }
}

QString QKlipperFan::name() const
{
    return m_name;
}

void QKlipperFan::resetName()
{
    setNameData({}); // TODO: Adapt to use your actual default value
}

void QKlipperFan::setSpeedData(qreal speed)
{
    if (qFuzzyCompare(m_speed, speed))
        return;

    m_speed = speed;
    emit speedChanged();
}

qreal QKlipperFan::speed() const
{
    return m_speed;
}

void QKlipperFan::setSpeed(qreal speed)
{
    //set the fan speed
    QString gcode = QString("SET_FAN_SPEED FAN=%1 SPEED=%2").arg(m_name, QString::number(speed));
    m_console->printerGcodeScript(gcode);
}

void QKlipperFan::resetSpeed()
{
    setSpeed({}); // TODO: Adapt to use your actual default value
}

void QKlipperFan::setRpmData(qreal rpm)
{
    if (qFuzzyCompare(m_rpm, rpm))
        return;

    m_rpm = rpm;
    emit rpmChanged();
}

qreal QKlipperFan::rpm() const
{
    return m_rpm;
}

void QKlipperFan::resetRpm()
{
    setRpmData({}); // TODO: Adapt to use your actual default value
}

bool QKlipperFan::isControllable() const
{
    return m_isControllable;
}

void QKlipperFan::setIsControllable(bool isControllable)
{
    if (m_isControllable == isControllable)
        return;

    m_isControllable = isControllable;
    emit isControllableChanged();
}

void QKlipperFan::resetIsControllable()
{
    setIsControllable({}); // TODO: Adapt to use your actual default value
}

bool QKlipperFan::hardwarePin() const
{
    return m_hardwarePin;
}

void QKlipperFan::setHardwarePin(bool hardwarePin)
{
    if (m_hardwarePin == hardwarePin)
        return;
    m_hardwarePin = hardwarePin;
    emit hardwarePinChanged();
}

QKlipperFan::Tachometer QKlipperFan::tachometer() const
{
    return m_tachometer;
}

void QKlipperFan::setTachometer(const Tachometer &tachometer)
{
    m_tachometer = tachometer;
    emit tachometerChanged();
}

qreal QKlipperFan::offBelow() const
{
    return m_offBelow;
}

void QKlipperFan::setOffBelow(qreal offBelow)
{
    if (qFuzzyCompare(m_offBelow, offBelow))
        return;
    m_offBelow = offBelow;
    emit offBelowChanged();
}

qreal QKlipperFan::kickStartTime() const
{
    return m_kickStartTime;
}

void QKlipperFan::setKickStartTime(qreal kickStartTime)
{
    if (qFuzzyCompare(m_kickStartTime, kickStartTime))
        return;
    m_kickStartTime = kickStartTime;
    emit kickStartTimeChanged();
}

qreal QKlipperFan::cycleTime() const
{
    return m_cycleTime;
}

void QKlipperFan::setCycleTime(qreal cycleTime)
{
    if (qFuzzyCompare(m_cycleTime, cycleTime))
        return;
    m_cycleTime = cycleTime;
    emit cycleTimeChanged();
}

qreal QKlipperFan::shutdownSpeed() const
{
    return m_shutdownSpeed;
}

void QKlipperFan::setShutdownSpeed(qreal shutdownSpeed)
{
    if (qFuzzyCompare(m_shutdownSpeed, shutdownSpeed))
        return;
    m_shutdownSpeed = shutdownSpeed;
    emit shutdownSpeedChanged();
}

qreal QKlipperFan::maxPower() const
{
    return m_maxPower;
}

void QKlipperFan::setMaxPower(qreal maxPower)
{
    if (qFuzzyCompare(m_maxPower, maxPower))
        return;
    m_maxPower = maxPower;
    emit maxPowerChanged();
}

QString QKlipperFan::enablePin() const
{
    return m_enablePin;
}

void QKlipperFan::setEnablePin(const QString &enablePin)
{
    if (m_enablePin == enablePin)
        return;
    m_enablePin = enablePin;
    emit enablePinChanged();
}

QString QKlipperFan::pin() const
{
    return m_pin;
}

void QKlipperFan::setPin(const QString &pin)
{
    if (m_pin == pin)
        return;
    m_pin = pin;
    emit pinChanged();
}

void QKlipperFan::setNameData(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}
