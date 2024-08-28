#include "qklipperfan.h"

QKlipperFan::QKlipperFan(QObject *parent)
    : QObject{parent}
{

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
    if (qFuzzyCompare(m_speed, speed))
        return;

    m_speed = speed;
    emit speedChanged();
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

void QKlipperFan::setNameData(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}

QKlipperPrinter *QKlipperFan::printer() const
{
    return m_printer;
}

void QKlipperFan::setPrinter(QKlipperPrinter *printer)
{
    m_printer = printer;
}
