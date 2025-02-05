#include <QKlipper/QKlipperPrinter/qklipperheaterfan.h>

QKlipperHeaterFan::QKlipperHeaterFan(QObject *parent)
    : QKlipperFan(parent)
{

}

QString QKlipperHeaterFan::heater() const
{
    return m_heater;
}

void QKlipperHeaterFan::setHeater(const QString &heater)
{
    if (m_heater == heater)
        return;

    m_heater = heater;
    emit heaterChanged();
}

qreal QKlipperHeaterFan::fanSpeed() const
{
    return m_fanSpeed;
}

void QKlipperHeaterFan::setFanSpeed(qreal fanSpeed)
{
    if (qFuzzyCompare(m_fanSpeed, fanSpeed))
        return;

    m_fanSpeed = fanSpeed;
    emit fanSpeedChanged();
}

qreal QKlipperHeaterFan::heaterTemperature() const
{
    return m_heaterTemperature;
}

void QKlipperHeaterFan::setHeaterTemperature(qreal heaterTemperature)
{
    if (qFuzzyCompare(m_heaterTemperature, heaterTemperature))
        return;
    m_heaterTemperature = heaterTemperature;
    emit heaterTemperatureChanged();
}
