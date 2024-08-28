/*
 * QKlipper - A Qt library for the Klipper/Moonraker API
 * Copyright (C) 2024 James Dudeck
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QKLIPPERWEBCAM_H
#define QKLIPPERWEBCAM_H

#include <QObject>

class QKlipperSystem;
class QKlipperConsole;

class QKlipperWebcam : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperWebcam(QObject *parent = nullptr);

    QKlipperWebcam(const QKlipperWebcam &value);
    QKlipperWebcam(QKlipperWebcam &&value);
    QKlipperWebcam &operator=(const QKlipperWebcam &value);
    QKlipperWebcam &operator=(QKlipperWebcam &&value);
    bool operator==(const QKlipperWebcam &value);
    bool operator==(QKlipperWebcam &&value);
    bool operator!=(const QKlipperWebcam &value);
    bool operator!=(QKlipperWebcam &&value);

    QString name() const;

    QString location() const;

    QString service() const;

    QString iconString() const;

    QString streamUrl() const;

    QString snapshotUrl() const;

    QString aspectRatio() const;

    QString source() const;

    QString uid() const;

    qint16 targetFps() const;

    qint16 rotation() const;

    bool enabled() const;

    bool flipVertical() const;

    bool flipHorizontal() const;

    qint16 targetFpsIdle() const;

public slots:
    void setName(const QString &name);

    void setLocation(const QString &location);

    void setService(const QString &service);

    void setIconString(const QString &iconString);

    void setStreamUrl(const QString &streamUrl);

    void setSnapshotUrl(const QString &snapshotUrl);

    void setAspectRatio(const QString &aspectRatio);

    void setSource(const QString &source);

    void setUid(const QString &uid);

    void setTargetFps(qint16 targetFps);

    void setRotation(qint16 rotation);

    void setEnabled(bool enabled);

    void setFlipVertical(bool flipVertical);

    void setFlipHorizontal(bool flipHorizontal);

    void setTargetFpsIdle(qint16 targetFpsIdle);

private slots:

signals:

    void nameChanged();

    void locationChanged();

    void serviceChanged();

    void iconStringChanged();

    void streamUrlChanged();

    void snapshotUrlChanged();

    void aspectRatioChanged();

    void sourceChanged();

    void uidChanged();

    void targetFpsChanged();

    void rotationChanged();

    void enabledChanged();

    void flipVerticalChanged();

    void flipHorizontalChanged();

    void targetFpsIdleChanged();

private:
    QString m_name;
    QString m_location;
    QString m_service;
    QString m_iconString;
    QString m_streamUrl;
    QString m_snapshotUrl;
    QString m_aspectRatio;
    QString m_source;
    QString m_uid;

    qint16 m_targetFps = 0;
    qint16 m_targetFpsIdle = 0;
    qint16 m_rotation = 0;

    bool m_enabled = false;
    bool m_flipVertical = false;
    bool m_flipHorizontal = false;

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged FINAL)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged FINAL)
    Q_PROPERTY(QString iconString READ iconString WRITE setIconString NOTIFY iconStringChanged FINAL)
    Q_PROPERTY(QString streamUrl READ streamUrl WRITE setStreamUrl NOTIFY streamUrlChanged FINAL)
    Q_PROPERTY(QString snapshotUrl READ snapshotUrl WRITE setSnapshotUrl NOTIFY snapshotUrlChanged FINAL)
    Q_PROPERTY(QString aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged FINAL)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString uid READ uid WRITE setUid NOTIFY uidChanged FINAL)
    Q_PROPERTY(qint16 targetFps READ targetFps WRITE setTargetFps NOTIFY targetFpsChanged FINAL)
    Q_PROPERTY(qint16 rotation READ rotation WRITE setRotation NOTIFY rotationChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool flipVertical READ flipVertical WRITE setFlipVertical NOTIFY flipVerticalChanged FINAL)
    Q_PROPERTY(bool flipHorizontal READ flipHorizontal WRITE setFlipHorizontal NOTIFY flipHorizontalChanged FINAL)
    Q_PROPERTY(qint16 targetFpsIdle READ targetFpsIdle WRITE setTargetFpsIdle NOTIFY targetFpsIdleChanged FINAL)
};

typedef QList<QKlipperWebcam> QKlipperWebcamList;

#endif // QKLIPPERWEBCAM_H