#include "qklipperv412mode.h"

QKlipperV412Mode::QKlipperV412Mode(QObject *parent)
    : QObject{parent}
{}

QKlipperV412Mode::QKlipperV412Mode(const QKlipperV412Mode &value)
{
    m_description = value.m_description;
    m_flags = value.m_flags;
    m_format = value.m_format;
    m_resolutions = value.m_resolutions;
}

QKlipperV412Mode::QKlipperV412Mode(QKlipperV412Mode &&value)
{
    m_description = value.m_description;
    m_flags = value.m_flags;
    m_format = value.m_format;
    m_resolutions = value.m_resolutions;
}

QKlipperV412Mode &QKlipperV412Mode::operator=(const QKlipperV412Mode &value)
{
    m_description = value.m_description;
    m_flags = value.m_flags;
    m_format = value.m_format;
    m_resolutions = value.m_resolutions;

    return *this;
}

QKlipperV412Mode &QKlipperV412Mode::operator=(QKlipperV412Mode &&value)
{
    m_description = value.m_description;
    m_flags = value.m_flags;
    m_format = value.m_format;
    m_resolutions = value.m_resolutions;

    return *this;
}

bool QKlipperV412Mode::operator==(const QKlipperV412Mode &value)
{
    if(m_description != value.m_description) return false;
    if(m_flags != value.m_flags) return false;
    if(m_format != value.m_format) return false;
    if(m_resolutions != value.m_resolutions) return false;

    return true;
}

bool QKlipperV412Mode::operator==(QKlipperV412Mode &&value)
{
    if(m_description != value.m_description) return false;
    if(m_flags != value.m_flags) return false;
    if(m_format != value.m_format) return false;
    if(m_resolutions != value.m_resolutions) return false;

    return true;
}

bool QKlipperV412Mode::operator!=(const QKlipperV412Mode &value)
{
    if(m_description == value.m_description &&
        m_flags == value.m_flags &&
        m_format == value.m_format &&
        m_resolutions == value.m_resolutions)
        return false;

    return true;
}

bool QKlipperV412Mode::operator!=(QKlipperV412Mode &&value)
{
    if(m_description == value.m_description &&
        m_flags == value.m_flags &&
        m_format == value.m_format &&
        m_resolutions == value.m_resolutions)
        return false;

    return true;
}

QString QKlipperV412Mode::description() const
{
    return m_description;
}

void QKlipperV412Mode::setDescription(const QString &description)
{
    if (m_description == description)
        return;
    m_description = description;
    emit descriptionChanged();
}

QString QKlipperV412Mode::format() const
{
    return m_format;
}

void QKlipperV412Mode::setFormat(const QString &format)
{
    if (m_format == format)
        return;
    m_format = format;
    emit formatChanged();
}

QStringList QKlipperV412Mode::flags() const
{
    return m_flags;
}

void QKlipperV412Mode::setFlags(const QStringList &flags)
{
    if (m_flags == flags)
        return;
    m_flags = flags;
    emit flagsChanged();
}

QStringList QKlipperV412Mode::resolutions() const
{
    return m_resolutions;
}

void QKlipperV412Mode::setResolutions(const QStringList &resolutions)
{
    if (m_resolutions == resolutions)
        return;
    m_resolutions = resolutions;
    emit resolutionsChanged();
}

