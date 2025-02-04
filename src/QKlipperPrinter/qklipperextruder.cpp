#include <QKlipper/QKlipperPrinter/qklipperextruder.h>
#include <QKlipper/QKlipperConsole/qklipperconsole.h>

QKlipperExtruder::QKlipperExtruder(QObject *parent)
    : QKlipperHeater{parent}
{
    m_fan = new QKlipperFan(this);
}

QKlipperExtruder::~QKlipperExtruder()
{

}

QKlipperFan *QKlipperExtruder::fan() const
{
    return m_fan;
}

void QKlipperExtruder::setFan(QKlipperFan *fan)
{
    if (m_fan == fan)
        return;

    m_fan = fan;
    emit fanChanged();
}

qint32 QKlipperExtruder::extruderNumber() const
{
    return m_extruderNumber;
}

void QKlipperExtruder::setExtruderNumber(qint32 extruderNumber)
{
    if (m_extruderNumber == extruderNumber)
        return;
    m_extruderNumber = extruderNumber;
    emit extruderNumberChanged();
}

QKlipperGearRatio QKlipperExtruder::gearRatio() const
{
    return m_gearRatio;
}

void QKlipperExtruder::setGearRatio(const QKlipperGearRatio &gearRatio)
{
    if (m_gearRatio == gearRatio)
        return;

    m_gearRatio = gearRatio;
    emit gearRatioChanged();
}

qint32 QKlipperExtruder::fullStepsPerRotation() const
{
    return m_fullStepsPerRotation;
}

void QKlipperExtruder::setFullStepsPerRotation(qint32 fullStepsPerRotation)
{
    if (m_fullStepsPerRotation == fullStepsPerRotation)
        return;
    m_fullStepsPerRotation = fullStepsPerRotation;
    emit fullStepsPerRotationChanged();
}

qint32 QKlipperExtruder::microsteps() const
{
    return m_microsteps;
}

void QKlipperExtruder::setMicrosteps(qint32 microsteps)
{
    if (m_microsteps == microsteps)
        return;
    m_microsteps = microsteps;
    emit microstepsChanged();
}

qreal QKlipperExtruder::pressureAdvance() const
{
    return m_pressureAdvance;
}

void QKlipperExtruder::setPressureAdvance(qreal pressureAdvance)
{
    //set to relative movement
    QString gcode("SET_PRESSURE_ADVANCE EXTRUDER=");
    gcode += m_name + QString(" ADVANCE=") + QString::number(pressureAdvance);
    gcode += QString(" SMOOTH_TIME=") + QString::number(m_pressureAdvanceSmoothTime);

    m_console->printerGcodeScript(gcode);
}

qreal QKlipperExtruder::pressureAdvanceSmoothTime() const
{
    return m_pressureAdvanceSmoothTime;
}

void QKlipperExtruder::setPressureAdvanceSmoothTime(qreal pressureAdvanceSmoothTime)
{
    if (qFuzzyCompare(m_pressureAdvanceSmoothTime, pressureAdvanceSmoothTime))
        return;

    m_pressureAdvanceSmoothTime = pressureAdvanceSmoothTime;
    emit pressureAdvanceSmoothTimeChanged();
}

qreal QKlipperExtruder::extrusionFactor() const
{
    return m_extrusionFactor;
}

void QKlipperExtruder::setExtrusionFactor(qreal extrusionFactor)
{
    //convert to percentage
    extrusionFactor *= 100;

    //M221 to set extrusion factor
    QString gcode("M221 S");

    //add the value as S
    gcode += QString::number(extrusionFactor);

    //set the extruder number as T
    gcode += QString(" T") + QString::number(m_extruderNumber);

    //send the gcode
    m_console->printerGcodeScript(gcode);
}

void QKlipperExtruder::extrude(qreal amount, qreal speed)
{
    if(m_canExtrude)
    {
        //set to relative movement
        QString gcode("G91");
        m_console->printerGcodeScript(gcode);

        //set active extruder
        gcode = QString("M6 T") + QString::number(m_extruderNumber);
        m_console->printerGcodeScript(gcode);

        //extrude the requested amount
        gcode = QString("G1 E") + QString::number(amount);
        gcode += QString(" F") + QString::number(speed);
        m_console->printerGcodeScript(gcode);
    }
}

void QKlipperExtruder::retract(qreal amount, qreal speed)
{
    extrude(amount * -1, speed);
}

void QKlipperExtruder::setExtrusionFactorData(qreal extrusionFactor)
{
    if (qFuzzyCompare(m_extrusionFactor, extrusionFactor))
        return;

    m_extrusionFactor = extrusionFactor;
    emit extrusionFactorChanged();
}

qreal QKlipperExtruder::filamentDiameter() const
{
    return m_filamentDiameter;
}

void QKlipperExtruder::setFilamentDiameter(qreal filamentDiameter)
{
    if (qFuzzyCompare(m_filamentDiameter, filamentDiameter))
        return;
    m_filamentDiameter = filamentDiameter;
    emit filamentDiameterChanged();
}

qreal QKlipperExtruder::nozzleDiameter() const
{
    return m_nozzleDiameter;
}

void QKlipperExtruder::setNozzleDiameter(qreal nozzleDiameter)
{
    if (qFuzzyCompare(m_nozzleDiameter, nozzleDiameter))
        return;
    m_nozzleDiameter = nozzleDiameter;
    emit nozzleDiameterChanged();
}

qreal QKlipperExtruder::inlineResistor() const
{
    return m_inlineResistor;
}

void QKlipperExtruder::setInlineResistor(qreal inlineResistor)
{
    if (qFuzzyCompare(m_inlineResistor, inlineResistor))
        return;
    m_inlineResistor = inlineResistor;
    emit inlineResistorChanged();
}

qreal QKlipperExtruder::pullupResistor() const
{
    return m_pullupResistor;
}

void QKlipperExtruder::setPullupResistor(qreal pullupResistor)
{
    if (qFuzzyCompare(m_pullupResistor, pullupResistor))
        return;
    m_pullupResistor = pullupResistor;
    emit pullupResistorChanged();
}

qreal QKlipperExtruder::rotationDistance() const
{
    return m_rotationDistance;
}

void QKlipperExtruder::setRotationDistance(qreal rotationDistance)
{
    if (qFuzzyCompare(m_rotationDistance, rotationDistance))
        return;
    m_rotationDistance = rotationDistance;
    emit rotationDistanceChanged();
}

qreal QKlipperExtruder::instantCornerVelocity() const
{
    return m_instantCornerVelocity;
}

void QKlipperExtruder::setInstantCornerVelocity(qreal instantCornerVelocity)
{
    if (qFuzzyCompare(m_instantCornerVelocity, instantCornerVelocity))
        return;
    m_instantCornerVelocity = instantCornerVelocity;
    emit instantCornerVelocityChanged();
}

qreal QKlipperExtruder::maxExtrudeCrossSection() const
{
    return m_maxExtrudeCrossSection;
}

void QKlipperExtruder::setMaxExtrudeCrossSection(qreal maxExtrudeCrossSection)
{
    if (qFuzzyCompare(m_maxExtrudeCrossSection, maxExtrudeCrossSection))
        return;
    m_maxExtrudeCrossSection = maxExtrudeCrossSection;
    emit maxExtrudeCrossSectionChanged();
}

qreal QKlipperExtruder::maxExtrudeOnlyAcceleration() const
{
    return m_maxExtrudeOnlyAcceleration;
}

void QKlipperExtruder::setMaxExtrudeOnlyAcceleration(qreal maxExtrudeOnlyAcceleration)
{
    if (qFuzzyCompare(m_maxExtrudeOnlyAcceleration, maxExtrudeOnlyAcceleration))
        return;
    m_maxExtrudeOnlyAcceleration = maxExtrudeOnlyAcceleration;
    emit maxExtrudeOnlyAccelerationChanged();
}

qreal QKlipperExtruder::maxExtrudeOnlyDistance() const
{
    return m_maxExtrudeOnlyDistance;
}

void QKlipperExtruder::setMaxExtrudeOnlyDistance(qreal maxExtrudeOnlyDistance)
{
    if (qFuzzyCompare(m_maxExtrudeOnlyDistance, maxExtrudeOnlyDistance))
        return;
    m_maxExtrudeOnlyDistance = maxExtrudeOnlyDistance;
    emit maxExtrudeOnlyDistanceChanged();
}

qreal QKlipperExtruder::maxExtrudeOnlyVelocity() const
{
    return m_maxExtrudeOnlyVelocity;
}

void QKlipperExtruder::setMaxExtrudeOnlyVelocity(qreal maxExtrudeOnlyVelocity)
{
    if (qFuzzyCompare(m_maxExtrudeOnlyVelocity, maxExtrudeOnlyVelocity))
        return;
    m_maxExtrudeOnlyVelocity = maxExtrudeOnlyVelocity;
    emit maxExtrudeOnlyVelocityChanged();
}

qreal QKlipperExtruder::minExtrudeTemp() const
{
    return m_minExtrudeTemp;
}

void QKlipperExtruder::setMinExtrudeTemp(qreal minExtrudeTemp)
{
    if (qFuzzyCompare(m_minExtrudeTemp, minExtrudeTemp))
        return;
    m_minExtrudeTemp = minExtrudeTemp;
    emit minExtrudeTempChanged();
}

bool QKlipperExtruder::canExtrude() const
{
    return m_canExtrude;
}

void QKlipperExtruder::setCanExtrude(bool canExtrude)
{
    if (m_canExtrude == canExtrude)
        return;
    m_canExtrude = canExtrude;
    emit canExtrudeChanged();
}

QString QKlipperExtruder::name() const
{
    return m_name;
}

void QKlipperExtruder::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged();
}

QString QKlipperExtruder::dirPin() const
{
    return m_dirPin;
}

void QKlipperExtruder::setDirPin(const QString &dirPin)
{
    if (m_dirPin == dirPin)
        return;
    m_dirPin = dirPin;
    emit dirPinChanged();
}

QString QKlipperExtruder::enablePin() const
{
    return m_enablePin;
}

void QKlipperExtruder::setEnablePin(const QString &enablePin)
{
    if (m_enablePin == enablePin)
        return;
    m_enablePin = enablePin;
    emit enablePinChanged();
}

QString QKlipperExtruder::stepPin() const
{
    return m_stepPin;
}

void QKlipperExtruder::setStepPin(const QString &stepPin)
{
    if (m_stepPin == stepPin)
        return;

    m_stepPin = stepPin;
    emit stepPinChanged();
}

QKlipperTemperatureStore QKlipperExtruder::temperatureStore() const
{
    return m_temperatureStore;
}

void QKlipperExtruder::setTemperatureStore(const QKlipperTemperatureStore &temperatureStore)
{
    bool changed = false;

    for(const QKlipperTemperatureStoreValue &value : temperatureStore)
    {
        if(!m_temperatureStore.contains(value))
        {
            changed = true;
            break;
        }
    }

    if(!changed)
        return;

    m_temperatureStore = temperatureStore;
    emit temperatureStoreChanged();
}

void QKlipperExtruder::setTemperatureStoreValue(const QKlipperTemperatureStoreValue &value)
{
    if(m_temperatureStore.contains(value))
        return;

    m_temperatureStore.append(value);
    emit temperatureStoreChanged();
}

void QKlipperExtruder::setPressureAdvanceSmoothTimeData(qreal pressureAdvanceSmoothTime)
{
    if (qFuzzyCompare(m_pressureAdvanceSmoothTime, pressureAdvanceSmoothTime))
        return;
    m_pressureAdvanceSmoothTime = pressureAdvanceSmoothTime;
    emit pressureAdvanceSmoothTimeChanged();
}

QKlipperConsole *QKlipperExtruder::console() const
{
    return m_console;
}

void QKlipperExtruder::setConsole(QKlipperConsole *console)
{
    if (m_console == console)
        return;
    m_console = console;
    emit consoleChanged();
}

QKlipperPosition QKlipperExtruder::offset() const
{
    return m_offset;
}

void QKlipperExtruder::setOffsetData(const QKlipperPosition &offset)
{
    if (m_offset == offset)
        return;
    m_offset = offset;
    emit offsetChanged();
}

void QKlipperExtruder::setPressureAdvanceData(qreal pressureAdvance)
{
    if (qFuzzyCompare(m_pressureAdvance, pressureAdvance))
        return;
    m_pressureAdvance = pressureAdvance;
    emit pressureAdvanceChanged();
}
