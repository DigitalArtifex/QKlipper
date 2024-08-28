#include "qklippertoolhead.h"
#include "qklipperprinter.h"

QKlipperToolHead::QKlipperToolHead(QObject *parent)
    : QObject{parent}
{
    m_partsFan = new QKlipperFan(this);

    QKlipperPrinter* printer = qobject_cast<QKlipperPrinter*>(parent);

    if(printer)
        m_printer = printer;
}

QKlipperToolHead::~QKlipperToolHead()
{
    if(m_partsFan)
        delete m_partsFan;

    foreach(QKlipperExtruder *extruder, m_extruderMap)
        delete extruder;
}

QKlipperFan *QKlipperToolHead::partsFan() const
{
    return m_partsFan;
}

void QKlipperToolHead::setPartsFan(QKlipperFan *partsFan)
{
    if (m_partsFan == partsFan)
        return;

    m_partsFan = partsFan;
    emit partsFanChanged();
}

QMap<QString, QKlipperExtruder *> QKlipperToolHead::extruderMap() const
{
    return m_extruderMap;
}

QKlipperExtruder *QKlipperToolHead::extruder(const QString &name)
{
    if(m_extruderMap.contains(name))
        return m_extruderMap[name];

    return nullptr;
}

QKlipperExtruder *QKlipperToolHead::currentExtruder()
{
    if(m_extruderMap.contains(m_currentExtruderName))
        return m_extruderMap[m_currentExtruderName];

    return nullptr;
}

void QKlipperToolHead::setExtruderMap(const QMap<QString, QKlipperExtruder *> &extruderMap)
{
    if (m_extruderMap == extruderMap)
        return;
    m_extruderMap = extruderMap;
    emit extruderMapChanged();
}

void QKlipperToolHead::setExtruder(QString name, QKlipperExtruder *extruder)
{
    if(m_extruderMap.contains(name))
    {
        if(extruder == m_extruderMap[name])
            return;

        QKlipperExtruder *current = m_extruderMap[name];
        m_extruderMap.remove(name);

        delete current;
    }

    m_extruderMap[name] = extruder;
    emit extruderMapChanged();
}

QKlipperPosition QKlipperToolHead::minPosition() const
{
    return m_minPosition;
}

void QKlipperToolHead::setMinPosition(const QKlipperPosition &minPosition)
{
    if (m_minPosition == minPosition)
        return;
    m_minPosition = minPosition;
    emit minPositionChanged();
}

void QKlipperToolHead::setPositionData(const QKlipperPosition &position)
{
    if (m_position == position)
        return;

    m_position = position;
    emit positionChanged();
}

QKlipperPrinter *QKlipperToolHead::printer() const
{
    return m_printer;
}

void QKlipperToolHead::setPrinter(QKlipperPrinter *printer)
{
    if (m_printer == printer)
        return;
    m_printer = printer;
    emit printerChanged();
}

QKlipperConsole *QKlipperToolHead::console() const
{
    return m_console;
}

void QKlipperToolHead::setConsole(QKlipperConsole *console)
{
    if (m_console == console)
        return;

    QMapIterator<QString,QKlipperExtruder*> iterator(m_extruderMap);

    while(iterator.hasNext())
    {
        iterator.next();
        iterator.value()->setConsole(console);
    }

    m_console = console;
    emit consoleChanged();
}

QKlipperPosition QKlipperToolHead::maxPosition() const
{
    return m_maxPosition;
}

void QKlipperToolHead::setMaxPosition(const QKlipperPosition &maxPosition)
{
    if (m_maxPosition == maxPosition)
        return;
    m_maxPosition = maxPosition;
    emit maxPositionChanged();
}

QKlipperPosition QKlipperToolHead::destination() const
{
    return m_destination;
}

void QKlipperToolHead::setDestination(const QKlipperPosition &destination)
{
    if (m_destination == destination)
        return;
    m_destination = destination;
    emit destinationChanged();
}

QKlipperPosition QKlipperToolHead::position() const
{
    return m_position;
}

void QKlipperToolHead::setPosition(const QKlipperPosition &position)
{
    if (m_position == position)
        return;
    m_position = position;
    emit positionChanged();
}

qreal QKlipperToolHead::squareCornerVelocity() const
{
    return m_squareCornerVelocity;
}

void QKlipperToolHead::setSquareCornerVelocity(qreal squareCornerVelocity)
{
    if (qFuzzyCompare(m_squareCornerVelocity, squareCornerVelocity))
        return;
    m_squareCornerVelocity = squareCornerVelocity;
    emit squareCornerVelocityChanged();
}

qint32 QKlipperToolHead::checks() const
{
    return m_checks;
}

void QKlipperToolHead::setChecks(qint32 checks)
{
    if (m_checks == checks)
        return;
    m_checks = checks;
    emit checksChanged();
}

qint32 QKlipperToolHead::stalls() const
{
    return m_stalls;
}

void QKlipperToolHead::setStalls(qint32 stalls)
{
    if (m_stalls == stalls)
        return;
    m_stalls = stalls;
    emit stallsChanged();
}

qint32 QKlipperToolHead::maxAccelerationToDeceleration() const
{
    return m_maxAccelerationToDeceleration;
}

void QKlipperToolHead::setMaxAccelerationToDeceleration(qint32 maxAccelerationToDeceleration)
{
    if (m_maxAccelerationToDeceleration == maxAccelerationToDeceleration)
        return;
    m_maxAccelerationToDeceleration = maxAccelerationToDeceleration;
    emit maxAccelerationToDecelerationChanged();
}

qint32 QKlipperToolHead::maxVelocity() const
{
    return m_maxVelocity;
}

void QKlipperToolHead::setMaxVelocity(qint32 maxVelocity)
{
    if (m_maxVelocity == maxVelocity)
        return;
    m_maxVelocity = maxVelocity;
    emit maxVelocityChanged();
}

qint32 QKlipperToolHead::maxAcceleration() const
{
    return m_maxAcceleration;
}

void QKlipperToolHead::setMaxAcceleration(qint32 maxAcceleration)
{
    if (m_maxAcceleration == maxAcceleration)
        return;
    m_maxAcceleration = maxAcceleration;
    emit maxAccelerationChanged();
}

bool QKlipperToolHead::isHoming() const
{
    return m_isHoming;
}

void QKlipperToolHead::setIsHoming(bool isHoming)
{
    if (m_isHoming == isHoming)
        return;
    m_isHoming = isHoming;
    emit isHomingChanged();
}

bool QKlipperToolHead::xHomed() const
{
    return m_xHomed;
}

void QKlipperToolHead::setXHomed(bool xHomed)
{
    if (m_xHomed == xHomed)
        return;
    m_xHomed = xHomed;
    emit xHomedChanged();
}

bool QKlipperToolHead::yHomed() const
{
    return m_yHomed;
}

void QKlipperToolHead::setYHomed(bool yHomed)
{
    if (m_yHomed == yHomed)
        return;
    m_yHomed = yHomed;
    emit yHomedChanged();
}

bool QKlipperToolHead::zHomed() const
{
    return m_zHomed;
}

void QKlipperToolHead::setZHomed(bool zHomed)
{
    if (m_zHomed == zHomed)
        return;
    m_zHomed = zHomed;
    emit zHomedChanged();
}

QString QKlipperToolHead::currentExtruderName() const
{
    return m_currentExtruderName;
}

void QKlipperToolHead::setCurrentExtruderName(const QString &currentExtruderName)
{
    if (m_currentExtruderName == currentExtruderName)
        return;
    m_currentExtruderName = currentExtruderName;
    emit currentExtruderNameChanged();
}
