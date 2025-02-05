#ifndef QKLIPPERCONTROLLERFAN_H
#define QKLIPPERCONTROLLERFAN_H

#include "qklipperfan.h"
#include "QKlipper/dalib_global.h"

class QKlipperPrinter;
class QKlipperConsole;

//!  QKlipperHeaterFan class
/*!
  This class is responsible for monitoring anheater fan objects
*/

class DA_EXPORT QKlipperControllerFan : public QKlipperFan
{
    Q_OBJECT
    friend QKlipperConsole;
    friend QKlipperPrinter;

public:
    explicit QKlipperControllerFan(QObject *parent);

    QString stepper() const;

    qreal idleTimeout() const;

    qreal idleSpeed() const;

    QString heater() const;

    qreal fanSpeed() const;

private slots:

    void setStepper(const QString &stepper);

    void setIdleTimeout(qreal idleTimeout);

    void setIdleSpeed(qreal idleSpeed);

    void setHeater(const QString &heater);

    void setFanSpeed(qreal fanSpeed);

signals:

    void fanSpeedChanged();

    void stepperChanged();

    void idleTimeoutChanged();

    void idleSpeedChanged();

    void heaterChanged();

private:
    QString m_heater;
    QString m_stepper;

    qreal m_fanSpeed = 0;
    qreal m_idleTimeout = 0;
    qreal m_idleSpeed = 0;

    Q_PROPERTY(QString stepper READ stepper WRITE setStepper NOTIFY stepperChanged FINAL)
    Q_PROPERTY(qreal idleTimeout READ idleTimeout WRITE setIdleTimeout NOTIFY idleTimeoutChanged FINAL)
    Q_PROPERTY(qreal idleSpeed READ idleSpeed WRITE setIdleSpeed NOTIFY idleSpeedChanged FINAL)
    Q_PROPERTY(QString heater READ heater WRITE setHeater NOTIFY heaterChanged FINAL)
    Q_PROPERTY(qreal fanSpeed READ fanSpeed WRITE setFanSpeed NOTIFY fanSpeedChanged FINAL)
};

#endif // QKLIPPERCONTROLLERFAN_H
