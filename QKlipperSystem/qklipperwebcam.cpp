#include "qklipperwebcam.h"

QKlipperWebcam::QKlipperWebcam(QObject *parent)
    : QObject{parent}
{}

QKlipperWebcam::QKlipperWebcam(const QKlipperWebcam &value)
{
    m_name = value.m_name;
    m_location = value.m_location;
    m_service = value.m_service;
    m_iconString = value.m_iconString;
    m_streamUrl = value.m_streamUrl;
    m_snapshotUrl = value.m_snapshotUrl;
    m_aspectRatio = value.m_aspectRatio;
    m_source = value.m_source;
    m_uid = value.m_uid;

    m_targetFps = value.m_targetFps;
    m_targetFpsIdle = value.m_targetFpsIdle;
    m_rotation = value.m_rotation;

    m_enabled = value.m_enabled;
    m_flipVertical = value.m_flipVertical;
    m_flipHorizontal = value.m_flipHorizontal;
}

QKlipperWebcam::QKlipperWebcam(QKlipperWebcam &&value)
{
    m_name = value.m_name;
    m_location = value.m_location;
    m_service = value.m_service;
    m_iconString = value.m_iconString;
    m_streamUrl = value.m_streamUrl;
    m_snapshotUrl = value.m_snapshotUrl;
    m_aspectRatio = value.m_aspectRatio;
    m_source = value.m_source;
    m_uid = value.m_uid;

    m_targetFps = value.m_targetFps;
    m_targetFpsIdle = value.m_targetFpsIdle;
    m_rotation = value.m_rotation;

    m_enabled = value.m_enabled;
    m_flipVertical = value.m_flipVertical;
    m_flipHorizontal = value.m_flipHorizontal;
}

QKlipperWebcam &QKlipperWebcam::operator=(const QKlipperWebcam &value)
{
    m_name = value.m_name;
    m_location = value.m_location;
    m_service = value.m_service;
    m_iconString = value.m_iconString;
    m_streamUrl = value.m_streamUrl;
    m_snapshotUrl = value.m_snapshotUrl;
    m_aspectRatio = value.m_aspectRatio;
    m_source = value.m_source;
    m_uid = value.m_uid;

    m_targetFps = value.m_targetFps;
    m_targetFpsIdle = value.m_targetFpsIdle;
    m_rotation = value.m_rotation;

    m_enabled = value.m_enabled;
    m_flipVertical = value.m_flipVertical;
    m_flipHorizontal = value.m_flipHorizontal;

    return *this;
}

QKlipperWebcam &QKlipperWebcam::operator=(QKlipperWebcam &&value)
{
    m_name = value.m_name;
    m_location = value.m_location;
    m_service = value.m_service;
    m_iconString = value.m_iconString;
    m_streamUrl = value.m_streamUrl;
    m_snapshotUrl = value.m_snapshotUrl;
    m_aspectRatio = value.m_aspectRatio;
    m_source = value.m_source;
    m_uid = value.m_uid;

    m_targetFps = value.m_targetFps;
    m_targetFpsIdle = value.m_targetFpsIdle;
    m_rotation = value.m_rotation;

    m_enabled = value.m_enabled;
    m_flipVertical = value.m_flipVertical;
    m_flipHorizontal = value.m_flipHorizontal;

    return *this;
}

bool QKlipperWebcam::operator==(const QKlipperWebcam &value)
{
    if(m_name != value.m_name) return false;
    if(m_location != value.m_location) return false;
    if(m_service != value.m_service) return false;
    if(m_iconString != value.m_iconString) return false;
    if(m_streamUrl != value.m_streamUrl) return false;
    if(m_snapshotUrl != value.m_snapshotUrl) return false;
    if(m_aspectRatio != value.m_aspectRatio) return false;
    if(m_source != value.m_source) return false;
    if(m_uid != value.m_uid) return false;

    if(m_targetFps != value.m_targetFps) return false;
    if(m_targetFpsIdle != value.m_targetFpsIdle) return false;
    if(m_rotation != value.m_rotation) return false;

    if(m_enabled != value.m_enabled) return false;
    if(m_flipVertical != value.m_flipVertical) return false;
    if(m_flipHorizontal != value.m_flipHorizontal) return false;

    return true;
}

bool QKlipperWebcam::operator==(QKlipperWebcam &&value)
{
    if(m_name != value.m_name) return false;
    if(m_location != value.m_location) return false;
    if(m_service != value.m_service) return false;
    if(m_iconString != value.m_iconString) return false;
    if(m_streamUrl != value.m_streamUrl) return false;
    if(m_snapshotUrl != value.m_snapshotUrl) return false;
    if(m_aspectRatio != value.m_aspectRatio) return false;
    if(m_source != value.m_source) return false;
    if(m_uid != value.m_uid) return false;

    if(m_targetFps != value.m_targetFps) return false;
    if(m_targetFpsIdle != value.m_targetFpsIdle) return false;
    if(m_rotation != value.m_rotation) return false;

    if(m_enabled != value.m_enabled) return false;
    if(m_flipVertical != value.m_flipVertical) return false;
    if(m_flipHorizontal != value.m_flipHorizontal) return false;

    return true;
}

bool QKlipperWebcam::operator!=(const QKlipperWebcam &value)
{
    if(m_name == value.m_name &&
        m_location == value.m_location &&
        m_service == value.m_service &&
        m_iconString == value.m_iconString &&
        m_streamUrl == value.m_streamUrl &&
        m_snapshotUrl == value.m_snapshotUrl &&
        m_aspectRatio == value.m_aspectRatio &&
        m_source == value.m_source &&
        m_uid == value.m_uid &&
        m_targetFps == value.m_targetFps &&
        m_targetFpsIdle == value.m_targetFpsIdle &&
        m_rotation == value.m_rotation &&
        m_enabled == value.m_enabled &&
        m_flipVertical == value.m_flipVertical &&
        m_flipHorizontal == value.m_flipHorizontal)
        return false;

    return true;
}

bool QKlipperWebcam::operator!=(QKlipperWebcam &&value)
{
    if(m_name == value.m_name &&
        m_location == value.m_location &&
        m_service == value.m_service &&
        m_iconString == value.m_iconString &&
        m_streamUrl == value.m_streamUrl &&
        m_snapshotUrl == value.m_snapshotUrl &&
        m_aspectRatio == value.m_aspectRatio &&
        m_source == value.m_source &&
        m_uid == value.m_uid &&
        m_targetFps == value.m_targetFps &&
        m_targetFpsIdle == value.m_targetFpsIdle &&
        m_rotation == value.m_rotation &&
        m_enabled == value.m_enabled &&
        m_flipVertical == value.m_flipVertical &&
        m_flipHorizontal == value.m_flipHorizontal)
        return false;

    return true;
}

QString QKlipperWebcam::name() const
{
    return m_name;
}

void QKlipperWebcam::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged();
}

QString QKlipperWebcam::location() const
{
    return m_location;
}

void QKlipperWebcam::setLocation(const QString &location)
{
    if (m_location == location)
        return;
    m_location = location;
    emit locationChanged();
}

QString QKlipperWebcam::service() const
{
    return m_service;
}

void QKlipperWebcam::setService(const QString &service)
{
    if (m_service == service)
        return;
    m_service = service;
    emit serviceChanged();
}

QString QKlipperWebcam::iconString() const
{
    return m_iconString;
}

void QKlipperWebcam::setIconString(const QString &iconString)
{
    if (m_iconString == iconString)
        return;
    m_iconString = iconString;
    emit iconStringChanged();
}

QString QKlipperWebcam::streamUrl() const
{
    return m_streamUrl;
}

void QKlipperWebcam::setStreamUrl(const QString &streamUrl)
{
    if (m_streamUrl == streamUrl)
        return;
    m_streamUrl = streamUrl;
    emit streamUrlChanged();
}

QString QKlipperWebcam::snapshotUrl() const
{
    return m_snapshotUrl;
}

void QKlipperWebcam::setSnapshotUrl(const QString &snapshotUrl)
{
    if (m_snapshotUrl == snapshotUrl)
        return;
    m_snapshotUrl = snapshotUrl;
    emit snapshotUrlChanged();
}

QString QKlipperWebcam::aspectRatio() const
{
    return m_aspectRatio;
}

void QKlipperWebcam::setAspectRatio(const QString &aspectRatio)
{
    if (m_aspectRatio == aspectRatio)
        return;
    m_aspectRatio = aspectRatio;
    emit aspectRatioChanged();
}

QString QKlipperWebcam::source() const
{
    return m_source;
}

void QKlipperWebcam::setSource(const QString &source)
{
    if (m_source == source)
        return;
    m_source = source;
    emit sourceChanged();
}

QString QKlipperWebcam::uid() const
{
    return m_uid;
}

void QKlipperWebcam::setUid(const QString &uid)
{
    if (m_uid == uid)
        return;
    m_uid = uid;
    emit uidChanged();
}

qint16 QKlipperWebcam::targetFps() const
{
    return m_targetFps;
}

void QKlipperWebcam::setTargetFps(qint16 targetFps)
{
    if (m_targetFps == targetFps)
        return;
    m_targetFps = targetFps;
    emit targetFpsChanged();
}

qint16 QKlipperWebcam::rotation() const
{
    return m_rotation;
}

void QKlipperWebcam::setRotation(qint16 rotation)
{
    if (m_rotation == rotation)
        return;
    m_rotation = rotation;
    emit rotationChanged();
}

bool QKlipperWebcam::enabled() const
{
    return m_enabled;
}

void QKlipperWebcam::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit enabledChanged();
}

bool QKlipperWebcam::flipVertical() const
{
    return m_flipVertical;
}

void QKlipperWebcam::setFlipVertical(bool flipVertical)
{
    if (m_flipVertical == flipVertical)
        return;
    m_flipVertical = flipVertical;
    emit flipVerticalChanged();
}

bool QKlipperWebcam::flipHorizontal() const
{
    return m_flipHorizontal;
}

void QKlipperWebcam::setFlipHorizontal(bool flipHorizontal)
{
    if (m_flipHorizontal == flipHorizontal)
        return;
    m_flipHorizontal = flipHorizontal;
    emit flipHorizontalChanged();
}

qint16 QKlipperWebcam::targetFpsIdle() const
{
    return m_targetFpsIdle;
}

void QKlipperWebcam::setTargetFpsIdle(qint16 targetFpsIdle)
{
    if (m_targetFpsIdle == targetFpsIdle)
        return;
    m_targetFpsIdle = targetFpsIdle;
    emit targetFpsIdleChanged();
}
