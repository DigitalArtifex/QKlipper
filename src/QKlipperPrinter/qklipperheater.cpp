#include <QKlipper/QKlipperPrinter/qklipperheater.h>
#include <QKlipper/QKlipperPrinter/qklipperprinter.h>
#include <QKlipper/QKlipperConsole/qklipperconsole.h>

QKlipperHeater::QKlipperHeater(QObject *parent)
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

qreal QKlipperHeater::temperature() const
{
    return m_temperature;
}

void QKlipperHeater::setCurrentTemp(qreal temperature)
{
    if (qFuzzyCompare(m_temperature, temperature))
        return;

    m_temperature = temperature;
    emit temperatureChanged();
}

qreal QKlipperHeater::targetTemperature() const
{
    return m_targetTemperature;
}

void QKlipperHeater::setTargetTemperature(qreal targetTemperature)
{
    if(!m_console)
    {
        qWarning() << "Parental structure incorrect. No console for QKlipperHeater " + m_name;
        return;
    }

    QString name = m_name.split(' ').last();

    //M140 sets bed temp without waiting
    QString gcode = QString("SET_HEATER_TEMPERATURE HEATER=%1 TARGET=%2").arg(name, QString::number(targetTemperature));

    //send the G-Code
    m_console->printerGcodeScript(gcode);
}

void QKlipperHeater::calibratePid(qreal target)
{
    if(!m_console)
    {
        qWarning() << "Parental structure incorrect. No console for QKlipperHeater " + m_name;
        return;
    }

    emit pidCalibrating();

    //set to relative movement
    QString gcode = QString("PID_CALIBRATE HEATER=%1 TARGET=%2").arg(m_name, QString::number(target));

    //run calibration
    m_console->printerGcodeScript(gcode);

    emit pidCalibratingFinished();
}

void QKlipperHeater::setTargetTemperatureData(qreal targetTemperature)
{
    if (qFuzzyCompare(m_targetTemperature, targetTemperature))
        return;

    m_targetTemperature = targetTemperature;
    emit targetTemperatureChanged();
}

qreal QKlipperHeater::power() const
{
    return m_power;
}

void QKlipperHeater::setPower(qreal power)
{
    if (qFuzzyCompare(m_power, power))
        return;

    m_power = power;
    emit powerChanged();

    setWatts(power * m_maximumWatts);
}

qreal QKlipperHeater::watts() const
{
    return m_watts;
}

void QKlipperHeater::setWatts(qreal watts)
{
    if (qFuzzyCompare(m_watts, watts))
        return;

    m_watts = watts;
    emit wattsChanged();
}

qreal QKlipperHeater::maximumWatts() const
{
    return m_maximumWatts;
}

void QKlipperHeater::setMaxWatts(qreal maximumWatts)
{
    if (qFuzzyCompare(m_maximumWatts, maximumWatts))
        return;

    m_maximumWatts = maximumWatts;
    emit maximumWattsChanged();

    setWatts(m_power * m_maximumWatts);
}

qreal QKlipperHeater::inlineResistor() const
{
    return m_inlineResistor;
}

void QKlipperHeater::setInlineResistor(qreal inlineResistor)
{
    if (qFuzzyCompare(m_inlineResistor, inlineResistor))
        return;

    m_inlineResistor = inlineResistor;
    emit inlineResistorChanged();
}

qreal QKlipperHeater::maximumPower() const
{
    return m_maximumPower;
}

void QKlipperHeater::setMaxPower(qreal maximumPower)
{
    if (qFuzzyCompare(m_maximumPower, maximumPower))
        return;

    m_maximumPower = maximumPower;
    emit maximumPowerChanged();
}

qreal QKlipperHeater::maximumTemperature() const
{
    return m_maximumTemperature;
}

void QKlipperHeater::setMaxTemp(qreal maximumTemperature)
{
    if (qFuzzyCompare(m_maximumTemperature, maximumTemperature))
        return;

    m_maximumTemperature = maximumTemperature;
    emit maximumTemperatureChanged();
}

qreal QKlipperHeater::minimumTemperature() const
{
    return m_minimumTemperature;
}

void QKlipperHeater::setMinTemp(qreal minimumTemperature)
{
    if (qFuzzyCompare(m_minimumTemperature, minimumTemperature))
        return;

    m_minimumTemperature = minimumTemperature;
    emit minimumTemperatureChanged();
}

qreal QKlipperHeater::pidKD() const
{
    return m_pidKD;
}

void QKlipperHeater::setPidKD(qreal pidKD)
{
    if (qFuzzyCompare(m_pidKD, pidKD))
        return;

    m_pidKD = pidKD;
    emit pidKDChanged();
}

qreal QKlipperHeater::pidKI() const
{
    return m_pidKI;
}

void QKlipperHeater::setPidKI(qreal pidKI)
{
    if (qFuzzyCompare(m_pidKI, pidKI))
        return;

    m_pidKI = pidKI;
    emit pidKIChanged();
}

qreal QKlipperHeater::pidKP() const
{
    return m_pidKP;
}

void QKlipperHeater::setPidKP(qreal pidKP)
{
    if (qFuzzyCompare(m_pidKP, pidKP))
        return;

    m_pidKP = pidKP;
    emit pidKPChanged();
}

qreal QKlipperHeater::pullupResistor() const
{
    return m_pullupResistor;
}

void QKlipperHeater::setPullupResistor(qreal pullupResistor)
{
    if (qFuzzyCompare(m_pullupResistor, pullupResistor))
        return;

    m_pullupResistor = pullupResistor;
    emit pullupResistorChanged();
}

qreal QKlipperHeater::pwmCycleTime() const
{
    return m_pwmCycleTime;
}

void QKlipperHeater::setPwmCycleTime(qreal pwmCycleTime)
{
    if (qFuzzyCompare(m_pwmCycleTime, pwmCycleTime))
        return;

    m_pwmCycleTime = pwmCycleTime;
    emit pwmCycleTimeChanged();
}

qreal QKlipperHeater::smoothTime() const
{
    return m_smoothTime;
}

void QKlipperHeater::setSmoothTime(qreal smoothTime)
{
    if (qFuzzyCompare(m_smoothTime, smoothTime))
        return;

    m_smoothTime = smoothTime;
    emit smoothTimeChanged();
}
QString QKlipperHeater::control() const
{
    return m_control;
}

void QKlipperHeater::setControl(const QString &control)
{
    if (m_control == control)
        return;

    m_control = control;
    emit controlChanged();
}

QString QKlipperHeater::sensorPin() const
{
    return m_sensorPin;
}

void QKlipperHeater::setSensorPin(const QString &sensorPin)
{
    if (m_sensorPin == sensorPin)
        return;

    m_sensorPin = sensorPin;
    emit sensorPinChanged();
}

QString QKlipperHeater::heaterPin() const
{
    return m_heaterPin;
}

void QKlipperHeater::setHeaterPin(const QString &heaterPin)
{
    if (m_heaterPin == heaterPin)
        return;

    m_heaterPin = heaterPin;
    emit heaterPinChanged();
}

QString QKlipperHeater::sensorType() const
{
    return m_sensorType;
}

void QKlipperHeater::setSensorType(const QString &sensorType)
{
    if (m_sensorType == sensorType)
        return;

    m_sensorType = sensorType;
    emit sensorTypeChanged();
}

QString QKlipperHeater::gcodeId() const
{
    return m_gcodeId;
}

void QKlipperHeater::setGcodeId(const QString &gcodeId)
{
    if (m_gcodeId == gcodeId)
        return;
    m_gcodeId = gcodeId;
    emit gcodeIdChanged();
}

QString QKlipperHeater::name() const
{
    return m_name;
}

void QKlipperHeater::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}
