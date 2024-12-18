#include "qklipperuser.h"

QString QKlipperUser::username() const
{
    return m_username;
}

void QKlipperUser::setUsername(const QString &username)
{
    if (m_username == username)
        return;
    m_username = username;
}

QString QKlipperUser::source() const
{
    return m_source;
}

void QKlipperUser::setSource(const QString &source)
{
    if (m_source == source)
        return;

    m_source = source;
}

qreal QKlipperUser::createdOn() const
{
    return m_createdOn;
}

void QKlipperUser::setCreatedOn(qreal createdOn)
{
    if (qFuzzyCompare(m_createdOn, createdOn))
        return;

    m_createdOn = createdOn;
}

QKlipperUser::QKlipperUser(const QKlipperUser &user)
{
    m_username = user.m_username;
    m_createdOn = user.m_createdOn;
    m_source = user.m_source;
}

bool QKlipperUser::operator==(const QKlipperUser &user) const
{
    if(m_username != user.m_username)
        return false;
    if(m_createdOn != user.m_createdOn)
        return false;
    if(m_source != user.m_source)
        return false;

    return true;
}

bool QKlipperUser::operator!=(const QKlipperUser &user) const
{
    return !(*this == user);
}

QKlipperUser &QKlipperUser::operator=(const QKlipperUser &user)
{
    m_username = user.m_username;
    m_createdOn = user.m_createdOn;
    m_source = user.m_source;

    return (*this);
}
