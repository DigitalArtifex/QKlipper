#include "qklippervirtualizationstate.h"

QKlipperVirtualizationState::QKlipperVirtualizationState(QObject *parent) : QObject{parent}
{

}

QString QKlipperVirtualizationState::type() const
{
    return m_type;
}

void QKlipperVirtualizationState::setType(const QString &type)
{
    if (m_type == type)
        return;
    m_type = type;
    emit typeChanged();
}

QString QKlipperVirtualizationState::identifier() const
{
    return m_identifier;
}

void QKlipperVirtualizationState::setIdentifier(const QString &identifier)
{
    if (m_identifier == identifier)
        return;
    m_identifier = identifier;
    emit identifierChanged();
}
