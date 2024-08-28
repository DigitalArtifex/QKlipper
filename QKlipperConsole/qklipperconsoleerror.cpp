#include "qklipperconsoleerror.h"

QKlipperConsoleError::QKlipperConsoleError(QObject *parent)
    : QObject{parent}
{

}

QKlipperConsoleError::QKlipperConsoleError(const QKlipperConsoleError &value)
{
    m_errorString = value.errorString();
    m_origin = value.origin();
    m_type = value.type();
}

QKlipperConsoleError::QKlipperConsoleError(QKlipperConsoleError &&value)
{
    m_errorString = value.errorString();
    m_origin = value.origin();
    m_type = value.type();
}

QKlipperConsoleError &QKlipperConsoleError::operator=(const QKlipperConsoleError &value)
{
    m_errorString = value.errorString();
    m_origin = value.origin();
    m_type = value.type();

    return *this;
}

QKlipperConsoleError &QKlipperConsoleError::operator=(QKlipperConsoleError &&value)
{
    m_errorString = value.errorString();
    m_origin = value.origin();
    m_type = value.type();

    return *this;
}

QString QKlipperConsoleError::errorString() const
{
    return m_errorString;
}

void QKlipperConsoleError::setErrorString(const QString &errorString)
{
    if (m_errorString == errorString)
        return;

    m_errorString = errorString;
    emit errorStringChanged();
}

QString QKlipperConsoleError::origin() const
{
    return m_origin;
}

void QKlipperConsoleError::setOrigin(const QString &origin)
{
    if (m_origin == origin)
        return;

    m_origin = origin;
    emit originChanged();
}

QKlipperConsoleError::Type QKlipperConsoleError::type() const
{
    return m_type;
}

void QKlipperConsoleError::setType(Type type)
{
    if (m_type == type)
        return;

    m_type = type;
    emit typeChanged();
}
