#include "qklippergcodestorevalue.h"

QKlipperGCodeStoreValue::QKlipperGCodeStoreValue(const QKlipperGCodeStoreValue &value)
{
    m_gcodeType = value.m_gcodeType;
    m_message = value.m_message;
    m_time = value.m_time;
}

QKlipperGCodeStoreValue &QKlipperGCodeStoreValue::operator=(const QKlipperGCodeStoreValue &value)
{
    m_gcodeType = value.m_gcodeType;
    m_message = value.m_message;
    m_time = value.m_time;

    return *this;
}

bool QKlipperGCodeStoreValue::operator==(const QKlipperGCodeStoreValue &value) const
{
    if(m_gcodeType == value.m_gcodeType &&
        m_time == value.m_time &&
        m_message == value.m_message)
        return true;

    return false;
}

bool QKlipperGCodeStoreValue::operator!=(const QKlipperGCodeStoreValue &value) const
{
    return !(*this == value);
}

QString QKlipperGCodeStoreValue::message() const
{
    return m_message;
}

void QKlipperGCodeStoreValue::setMessage(const QString &message)
{
    if (m_message == message)
        return;

    m_message = message;
}

QKlipperGCodeStoreValue::GCodeType QKlipperGCodeStoreValue::gcodeType() const
{
    return m_gcodeType;
}

void QKlipperGCodeStoreValue::setGcodeType(GCodeType gcodeType)
{
    if (m_gcodeType == gcodeType)
        return;

    m_gcodeType = gcodeType;
}

qreal QKlipperGCodeStoreValue::time() const
{
    return m_time;
}

void QKlipperGCodeStoreValue::setTime(qreal time)
{
    if (qFuzzyCompare(m_time, time))
        return;

    m_time = time;
}
