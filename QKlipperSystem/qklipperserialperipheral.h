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

#ifndef QKLIPPERSERIALPERIPHERAL_H
#define QKLIPPERSERIALPERIPHERAL_H

#include <QObject>
class QKlipperSystem;
class QKlipperConsole;
/*!
 * \brief Filled by machine.peripherals.serial
 */
class QKlipperSerialPeripheral : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperSerialPeripheral(QObject *parent = nullptr);

    QKlipperSerialPeripheral(const QKlipperSerialPeripheral &value);
    QKlipperSerialPeripheral(QKlipperSerialPeripheral &&value);
    QKlipperSerialPeripheral &operator=(const QKlipperSerialPeripheral &value);
    QKlipperSerialPeripheral &operator=(QKlipperSerialPeripheral &&value);
    bool operator==(const QKlipperSerialPeripheral &value);
    bool operator==(QKlipperSerialPeripheral &&value);
    bool operator!=(const QKlipperSerialPeripheral &value);
    bool operator!=(QKlipperSerialPeripheral &&value);

    QString type() const;

    QString path() const;

    QString name() const;

    QString driver() const;

    QString hardwarePath() const;

    QString pathById() const;

    QString usbLocation() const;

public slots:
    void setType(const QString &type);

    void setPath(const QString &path);

    void setName(const QString &name);

    void setDriver(const QString &driver);

    void setHardwarePath(const QString &hardwarePath);

    void setPathById(const QString &pathById);

    void setUsbLocation(const QString &usbLocation);

private slots:

signals:

    void typeChanged();
    void pathChanged();
    void nameChanged();
    void driverChanged();
    void hardwarePathChanged();
    void pathByIdChanged();
    void usbLocationChanged();

private:
    QString m_type;
    QString m_path;
    QString m_name;
    QString m_driver;
    QString m_hardwarePath;
    QString m_pathById;
    QString m_usbLocation;

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString driver READ driver WRITE setDriver NOTIFY driverChanged FINAL)
    Q_PROPERTY(QString hardwarePath READ hardwarePath WRITE setHardwarePath NOTIFY hardwarePathChanged FINAL)
    Q_PROPERTY(QString pathById READ pathById WRITE setPathById NOTIFY pathByIdChanged FINAL)
    Q_PROPERTY(QString usbLocation READ usbLocation WRITE setUsbLocation NOTIFY usbLocationChanged FINAL)
};

#endif // QKLIPPERSERIALPERIPHERAL_H
