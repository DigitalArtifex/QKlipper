#include <QKlipper/QKlipperPrinter/qklipperchamber.h>

QKlipperChamber::QKlipperChamber(QObject *parent)
    : QObject{parent}
{}

qreal QKlipperChamber::temperatureerature() const
{
    return m_temperatureerature;
}

void QKlipperChamber::setCurrentTemperature(qreal temperatureerature)
{
    if (qFuzzyCompare(m_temperatureerature, temperatureerature))
        return;
    m_temperatureerature = temperatureerature;
    emit temperatureeratureChanged();
}

qreal QKlipperChamber::targetTemperatureerature() const
{
    return m_targetTemperatureerature;
}

void QKlipperChamber::setTargetTemperatureerature(qreal targetTemperatureerature)
{
    if (qFuzzyCompare(m_targetTemperatureerature, targetTemperatureerature))
        return;
    m_targetTemperatureerature = targetTemperatureerature;
    emit targetTemperatureeratureChanged();
}

qreal QKlipperChamber::maximumWatts() const
{
    return m_maximumWatts;
}

void QKlipperChamber::setMaxWatts(qreal maximumWatts)
{
    if (qFuzzyCompare(m_maximumWatts, maximumWatts))
        return;
    m_maximumWatts = maximumWatts;
    emit maximumWattsChanged();
}
