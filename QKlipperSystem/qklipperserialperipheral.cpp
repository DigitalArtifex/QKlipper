#include "qklipperserialperipheral.h"

QKlipperSerialPeripheral::QKlipperSerialPeripheral(QObject *parent)
    : QObject{parent}
{}

QKlipperSerialPeripheral::QKlipperSerialPeripheral(const QKlipperSerialPeripheral &value)
{
    m_type = value.m_type;
    m_path = value.m_path;
    m_name = value.m_name;
    m_driver = value.m_driver;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;
}

QKlipperSerialPeripheral::QKlipperSerialPeripheral(QKlipperSerialPeripheral &&value)
{
    m_type = value.m_type;
    m_path = value.m_path;
    m_name = value.m_name;
    m_driver = value.m_driver;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;
}

QKlipperSerialPeripheral &QKlipperSerialPeripheral::operator=(const QKlipperSerialPeripheral &value)
{
    m_type = value.m_type;
    m_path = value.m_path;
    m_name = value.m_name;
    m_driver = value.m_driver;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    return *this;
}

QKlipperSerialPeripheral &QKlipperSerialPeripheral::operator=(QKlipperSerialPeripheral &&value)
{
    m_type = value.m_type;
    m_path = value.m_path;
    m_name = value.m_name;
    m_driver = value.m_driver;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    return *this;
}

bool QKlipperSerialPeripheral::operator==(const QKlipperSerialPeripheral &value)
{
    if(m_type != value.m_type) return false;
    if(m_path != value.m_path) return false;
    if(m_name != value.m_name) return false;
    if(m_driver != value.m_driver) return false;
    if(m_hardwarePath != value.m_hardwarePath) return false;
    if(m_pathById != value.m_pathById) return false;
    if(m_usbLocation != value.m_usbLocation) return false;

    return true;
}

bool QKlipperSerialPeripheral::operator==(QKlipperSerialPeripheral &&value)
{
    if(m_type != value.m_type) return false;
    if(m_path != value.m_path) return false;
    if(m_name != value.m_name) return false;
    if(m_driver != value.m_driver) return false;
    if(m_hardwarePath != value.m_hardwarePath) return false;
    if(m_pathById != value.m_pathById) return false;
    if(m_usbLocation != value.m_usbLocation) return false;

    return true;
}

bool QKlipperSerialPeripheral::operator!=(const QKlipperSerialPeripheral &value)
{
    if(m_type == value.m_type &&
        m_path == value.m_path &&
        m_name == value.m_name &&
        m_driver == value.m_driver &&
        m_hardwarePath == value.m_hardwarePath &&
        m_pathById == value.m_pathById &&
        m_usbLocation == value.m_usbLocation)
        return false;

    return true;
}

bool QKlipperSerialPeripheral::operator!=(QKlipperSerialPeripheral &&value)
{
    if(m_type == value.m_type &&
        m_path == value.m_path &&
        m_name == value.m_name &&
        m_driver == value.m_driver &&
        m_hardwarePath == value.m_hardwarePath &&
        m_pathById == value.m_pathById &&
        m_usbLocation == value.m_usbLocation)
        return false;

    return true;
}

QString QKlipperSerialPeripheral::type() const
{
    return m_type;
}

void QKlipperSerialPeripheral::setType(const QString &type)
{
    if (m_type == type)
        return;
    m_type = type;
    emit typeChanged();
}

QString QKlipperSerialPeripheral::path() const
{
    return m_path;
}

void QKlipperSerialPeripheral::setPath(const QString &path)
{
    if (m_path == path)
        return;
    m_path = path;
    emit pathChanged();
}

QString QKlipperSerialPeripheral::name() const
{
    return m_name;
}

void QKlipperSerialPeripheral::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged();
}

QString QKlipperSerialPeripheral::driver() const
{
    return m_driver;
}

void QKlipperSerialPeripheral::setDriver(const QString &driver)
{
    if (m_driver == driver)
        return;
    m_driver = driver;
    emit driverChanged();
}

QString QKlipperSerialPeripheral::hardwarePath() const
{
    return m_hardwarePath;
}

void QKlipperSerialPeripheral::setHardwarePath(const QString &hardwarePath)
{
    if (m_hardwarePath == hardwarePath)
        return;
    m_hardwarePath = hardwarePath;
    emit hardwarePathChanged();
}

QString QKlipperSerialPeripheral::pathById() const
{
    return m_pathById;
}

void QKlipperSerialPeripheral::setPathById(const QString &pathById)
{
    if (m_pathById == pathById)
        return;
    m_pathById = pathById;
    emit pathByIdChanged();
}

QString QKlipperSerialPeripheral::usbLocation() const
{
    return m_usbLocation;
}

void QKlipperSerialPeripheral::setUsbLocation(const QString &usbLocation)
{
    if (m_usbLocation == usbLocation)
        return;
    m_usbLocation = usbLocation;
    emit usbLocationChanged();
}
