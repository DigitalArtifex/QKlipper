#ifndef QKLIPPERHEATERFAN_H
#define QKLIPPERHEATERFAN_H

#include "qklipperfan.h"
#include "QKlipper/dalib_global.h"

class QKlipperPrinter;
class QKlipperConsole;

//!  QKlipperHeaterFan class
/*!
  This class is responsible for monitoring anheater fan objects
*/

class DA_EXPORT QKlipperHeaterFan : public QKlipperFan
{
    Q_OBJECT
    friend QKlipperConsole;
    friend QKlipperPrinter;

public:
    explicit QKlipperHeaterFan(QObject *parent);

    QString heater() const;
    qreal fanSpeed() const;

    qreal heaterTemperature() const;

public slots:
    void setHeater(const QString &heater);

    void setFanSpeed(qreal fanSpeed);

    void setHeaterTemperature(qreal heaterTemperature);

signals:
    void heaterChanged();

    void fanSpeedChanged();

    void heaterTemperatureChanged();

private:
    QString m_heater;

    qreal m_heaterTemperature = 0;
    qreal m_fanSpeed = 0;

    Q_PROPERTY(QString heater READ heater WRITE setHeater NOTIFY heaterChanged FINAL)
    Q_PROPERTY(qreal fanSpeed READ fanSpeed WRITE setFanSpeed NOTIFY fanSpeedChanged FINAL)
    Q_PROPERTY(qreal heaterTemperature READ heaterTemperature WRITE setHeaterTemperature NOTIFY heaterTemperatureChanged FINAL)
};
#endif // QKLIPPERHEATERFAN_H
