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

qreal QKlipperHeater::currentTemp() const
{
    return m_currentTemp;
}

void QKlipperHeater::setCurrentTemp(qreal currentTemp)
{
    if (qFuzzyCompare(m_currentTemp, currentTemp))
        return;

    m_currentTemp = currentTemp;
    emit currentTempChanged();
}

qreal QKlipperHeater::targetTemp() const
{
    return m_targetTemp;
}

void QKlipperHeater::setTargetTemp(qreal targetTemp)
{
    if(!m_console)
    {
        qWarning() << "Parental structure incorrect. No console for QKlipperHeater " + m_name;
        return;
    }

    QString name = m_name.split(' ').last();

    //M140 sets bed temp without waiting
    QString gcode = QString("SET_HEATER_TEMPERATURE HEATER=%1 TARGET=%2").arg(name, QString::number(targetTemp));

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

void QKlipperHeater::setTargetTempData(qreal targetTemp)
{
    if (qFuzzyCompare(m_targetTemp, targetTemp))
        return;

    m_targetTemp = targetTemp;
    emit targetTempChanged();
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

    setWatts(power * m_maxWatts);
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

qreal QKlipperHeater::maxWatts() const
{
    return m_maxWatts;
}

void QKlipperHeater::setMaxWatts(qreal maxWatts)
{
    if (qFuzzyCompare(m_maxWatts, maxWatts))
        return;

    m_maxWatts = maxWatts;
    emit maxWattsChanged();

    setWatts(m_power * m_maxWatts);
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

qreal QKlipperHeater::maxPower() const
{
    return m_maxPower;
}

void QKlipperHeater::setMaxPower(qreal maxPower)
{
    if (qFuzzyCompare(m_maxPower, maxPower))
        return;

    m_maxPower = maxPower;
    emit maxPowerChanged();
}

qreal QKlipperHeater::maxTemp() const
{
    return m_maxTemp;
}

void QKlipperHeater::setMaxTemp(qreal maxTemp)
{
    if (qFuzzyCompare(m_maxTemp, maxTemp))
        return;

    m_maxTemp = maxTemp;
    emit maxTempChanged();
}

qreal QKlipperHeater::minTemp() const
{
    return m_minTemp;
}

void QKlipperHeater::setMinTemp(qreal minTemp)
{
    if (qFuzzyCompare(m_minTemp, minTemp))
        return;

    m_minTemp = minTemp;
    emit minTempChanged();
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
