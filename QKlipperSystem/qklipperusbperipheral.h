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

#ifndef QKLIPPERUSBPERIPHERAL_H
#define QKLIPPERUSBPERIPHERAL_H

#include <QObject>

class QKlipperSystem;
class QKlipperConsole;

/*!
 * \brief Filled by machine.peripherals.usb
 */
class QKlipperUsbPeripheral : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperUsbPeripheral(QObject *parent = nullptr);

    QKlipperUsbPeripheral(const QKlipperUsbPeripheral &value);
    QKlipperUsbPeripheral(QKlipperUsbPeripheral &&value);
    QKlipperUsbPeripheral &operator=(const QKlipperUsbPeripheral &value);
    QKlipperUsbPeripheral &operator=(QKlipperUsbPeripheral &&value);
    bool operator==(const QKlipperUsbPeripheral &value);
    bool operator==(QKlipperUsbPeripheral &&value);
    bool operator!=(const QKlipperUsbPeripheral &value);
    bool operator!=(QKlipperUsbPeripheral &&value);

    qint32 busNumber() const;

    qint32 deviceNumber() const;

    QString location() const;

    QString vendorId() const;

    QString productId() const;

    QString manufacturer() const;

    QString product() const;

    QString className() const;

    QString subclassName() const;

    QString protocol() const;

    QString description() const;

public slots:
    void setBusNumber(qint32 busNumber);

    void setDeviceNumber(qint32 deviceNumber);

    void setLocation(const QString &location);

    void setVendorId(const QString &vendorId);

    void setProductId(const QString &productId);

    void setManufacturer(const QString &manufacturer);

    void setProduct(const QString &product);

    void setClassName(const QString &className);

    void setSubclassName(const QString &subclassName);

    void setProtocol(const QString &protocol);

    void setDescription(const QString &description);

private slots:

signals:

    void busNumberChanged();

    void deviceNumberChanged();

    void locationChanged();

    void vendorIdChanged();

    void productIdChanged();

    void manufacturerChanged();

    void productChanged();

    void classNameChanged();

    void subclassNameChanged();

    void protocolChanged();

    void descriptionChanged();

private:
    qint32 m_busNumber = 0;
    qint32 m_deviceNumber = 0;

    QString m_location;
    QString m_vendorId;
    QString m_productId;
    QString m_manufacturer;
    QString m_product;
    QString m_className;
    QString m_subclassName;
    QString m_protocol;
    QString m_description;

    Q_PROPERTY(qint32 busNumber READ busNumber WRITE setBusNumber NOTIFY busNumberChanged FINAL)
    Q_PROPERTY(qint32 deviceNumber READ deviceNumber WRITE setDeviceNumber NOTIFY deviceNumberChanged FINAL)
    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged FINAL)
    Q_PROPERTY(QString vendorId READ vendorId WRITE setVendorId NOTIFY vendorIdChanged FINAL)
    Q_PROPERTY(QString productId READ productId WRITE setProductId NOTIFY productIdChanged FINAL)
    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged FINAL)
    Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged FINAL)
    Q_PROPERTY(QString className READ className WRITE setClassName NOTIFY classNameChanged FINAL)
    Q_PROPERTY(QString subclassName READ subclassName WRITE setSubclassName NOTIFY subclassNameChanged FINAL)
    Q_PROPERTY(QString protocol READ protocol WRITE setProtocol NOTIFY protocolChanged FINAL)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged FINAL)
};

#endif // QKLIPPERUSBPERIPHERAL_H
