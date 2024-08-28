#include "qklipperv412device.h"

QKlipperV412Device::QKlipperV412Device(QObject *parent)
    : QObject{parent}
{}

QKlipperV412Device::QKlipperV412Device(const QKlipperV412Device &value)
{
    m_name = value.m_name;
    m_path = value.m_path;
    m_cameraName = value.m_cameraName;
    m_driver = value.m_driver;
    m_altName = value.m_altName;
    m_hardwareBus = value.m_hardwareBus;
    m_version = value.m_version;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    m_capabilities = value.m_capabilities;
    m_modes = value.m_modes;
}

QKlipperV412Device::QKlipperV412Device(QKlipperV412Device &&value)
{
    m_name = value.m_name;
    m_path = value.m_path;
    m_cameraName = value.m_cameraName;
    m_driver = value.m_driver;
    m_altName = value.m_altName;
    m_hardwareBus = value.m_hardwareBus;
    m_version = value.m_version;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    m_capabilities = value.m_capabilities;
    m_modes = value.m_modes;
}

QKlipperV412Device &QKlipperV412Device::operator=(const QKlipperV412Device &value)
{
    m_name = value.m_name;
    m_path = value.m_path;
    m_cameraName = value.m_cameraName;
    m_driver = value.m_driver;
    m_altName = value.m_altName;
    m_hardwareBus = value.m_hardwareBus;
    m_version = value.m_version;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    m_capabilities = value.m_capabilities;
    m_modes = value.m_modes;

    return *this;
}

QKlipperV412Device &QKlipperV412Device::operator=(QKlipperV412Device &&value)
{
    m_name = value.m_name;
    m_path = value.m_path;
    m_cameraName = value.m_cameraName;
    m_driver = value.m_driver;
    m_altName = value.m_altName;
    m_hardwareBus = value.m_hardwareBus;
    m_version = value.m_version;
    m_hardwarePath = value.m_hardwarePath;
    m_pathById = value.m_pathById;
    m_usbLocation = value.m_usbLocation;

    m_capabilities = value.m_capabilities;
    m_modes = value.m_modes;

    return *this;
}

bool QKlipperV412Device::operator==(const QKlipperV412Device &value)
{
    if(m_name != value.m_name) return false;
    if(m_path != value.m_path) return false;
    if(m_cameraName != value.m_cameraName) return false;
    if(m_driver != value.m_driver) return false;
    if(m_altName != value.m_altName) return false;
    if(m_hardwareBus != value.m_hardwareBus) return false;
    if(m_version != value.m_version) return false;
    if(m_hardwarePath != value.m_hardwarePath) return false;
    if(m_pathById != value.m_pathById) return false;
    if(m_usbLocation != value.m_usbLocation) return false;

    if(m_capabilities != value.m_capabilities) return false;

    foreach(QKlipperV412Mode mode, value.m_modes)
    {
        if(!m_modes.contains(mode))
            return false;
    }

    return true;
}

bool QKlipperV412Device::operator==(QKlipperV412Device &&value)
{
    if(m_name != value.m_name) return false;
    if(m_path != value.m_path) return false;
    if(m_cameraName != value.m_cameraName) return false;
    if(m_driver != value.m_driver) return false;
    if(m_altName != value.m_altName) return false;
    if(m_hardwareBus != value.m_hardwareBus) return false;
    if(m_version != value.m_version) return false;
    if(m_hardwarePath != value.m_hardwarePath) return false;
    if(m_pathById != value.m_pathById) return false;
    if(m_usbLocation != value.m_usbLocation) return false;

    if(m_capabilities != value.m_capabilities) return false;

    foreach(QKlipperV412Mode mode, value.m_modes)
    {
        if(!m_modes.contains(mode))
            return false;
    }

    return true;
}

bool QKlipperV412Device::operator!=(const QKlipperV412Device &value)
{
    foreach(QKlipperV412Mode mode, value.m_modes)
    {
        if(!m_modes.contains(mode))
            return true;
    }

    if(m_name == value.m_name &&
        m_path == value.m_path &&
        m_cameraName == value.m_cameraName &&
        m_driver == value.m_driver &&
        m_altName == value.m_altName &&
        m_hardwareBus == value.m_hardwareBus &&
        m_version == value.m_version &&
        m_hardwarePath == value.m_hardwarePath &&
        m_pathById == value.m_pathById &&
        m_usbLocation == value.m_usbLocation &&
        m_capabilities == value.m_capabilities)
        return false;

    return true;
}

bool QKlipperV412Device::operator!=(QKlipperV412Device &&value)
{
    foreach(QKlipperV412Mode mode, value.m_modes)
    {
        if(!m_modes.contains(mode))
            return true;
    }

    if(m_name == value.m_name &&
        m_path == value.m_path &&
        m_cameraName == value.m_cameraName &&
        m_driver == value.m_driver &&
        m_altName == value.m_altName &&
        m_hardwareBus == value.m_hardwareBus &&
        m_version == value.m_version &&
        m_hardwarePath == value.m_hardwarePath &&
        m_pathById == value.m_pathById &&
        m_usbLocation == value.m_usbLocation &&
        m_capabilities == value.m_capabilities)
        return false;

    return true;
}

QString QKlipperV412Device::name() const
{
    return m_name;
}

void QKlipperV412Device::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}

QString QKlipperV412Device::path() const
{
    return m_path;
}

void QKlipperV412Device::setPath(const QString &path)
{
    if (m_path == path)
        return;

    m_path = path;
    emit pathChanged();
}

QString QKlipperV412Device::cameraName() const
{
    return m_cameraName;
}

void QKlipperV412Device::setCameraName(const QString &cameraName)
{
    if (m_cameraName == cameraName)
        return;

    m_cameraName = cameraName;
    emit cameraNameChanged();
}

QString QKlipperV412Device::driver() const
{
    return m_driver;
}

void QKlipperV412Device::setDriver(const QString &driver)
{
    if (m_driver == driver)
        return;

    m_driver = driver;
    emit driverChanged();
}

QString QKlipperV412Device::altName() const
{
    return m_altName;
}

void QKlipperV412Device::setAltName(const QString &altName)
{
    if (m_altName == altName)
        return;

    m_altName = altName;
    emit altNameChanged();
}

QString QKlipperV412Device::hardwareBus() const
{
    return m_hardwareBus;
}

void QKlipperV412Device::setHardwareBus(const QString &hardwareBus)
{
    if (m_hardwareBus == hardwareBus)
        return;

    m_hardwareBus = hardwareBus;
    emit hardwareBusChanged();
}

QString QKlipperV412Device::version() const
{
    return m_version;
}

void QKlipperV412Device::setVersion(const QString &version)
{
    if (m_version == version)
        return;

    m_version = version;
    emit versionChanged();
}

QString QKlipperV412Device::hardwarePath() const
{
    return m_hardwarePath;
}

void QKlipperV412Device::setHardwarePath(const QString &hardwarePath)
{
    if (m_hardwarePath == hardwarePath)
        return;

    m_hardwarePath = hardwarePath;
    emit hardwarePathChanged();
}

QString QKlipperV412Device::pathById() const
{
    return m_pathById;
}

void QKlipperV412Device::setPathById(const QString &pathById)
{
    if (m_pathById == pathById)
        return;

    m_pathById = pathById;
    emit pathByIdChanged();
}

QString QKlipperV412Device::usbLocation() const
{
    return m_usbLocation;
}

void QKlipperV412Device::setUsbLocation(const QString &usbLocation)
{
    if (m_usbLocation == usbLocation)
        return;

    m_usbLocation = usbLocation;
    emit usbLocationChanged();
}

QStringList QKlipperV412Device::capabilities() const
{
    return m_capabilities;
}

void QKlipperV412Device::setCapabilities(const QStringList &capabilities)
{
    if (m_capabilities == capabilities)
        return;

    m_capabilities = capabilities;
    emit capabilitiesChanged();
}

QList<QKlipperV412Mode> QKlipperV412Device::modes() const
{
    return m_modes;
}

void QKlipperV412Device::setModes(const QList<QKlipperV412Mode> &modes)
{
    bool changed = false;

    foreach(QKlipperV412Mode mode, modes)
    {
        if(!m_modes.contains(mode))
        {
            changed = true;
            break;
        }
    }

    if(!changed)
        return;

    m_modes = modes;
    emit modesChanged();
}
