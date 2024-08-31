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

#ifndef QKLIPPERINSTANCE_H
#define QKLIPPERINSTANCE_H

#include <QObject>

#include <QQueue>
#include <QByteArray>
#include <QFunctionPointer>
#include <QAbstractSocket>
#include <QLocalSocket>
#include <QTcpSocket>
#include <QSslSocket>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "QKlipperConsole/qklipperconsole.h"
#include "QKlipperPrinter/qklipperprinter.h"
#include "QKlipperServer/qklipperserver.h"
#include "QKlipperSystem/qklippersystem.h"

class QKlipperInstance : public QObject
{
    Q_OBJECT
public:

    explicit QKlipperInstance(QObject *parent = nullptr);
    ~QKlipperInstance();

    void reset();

    QString name() const;

    QString address() const;

    QString id() const;

    QString apiKey() const;

    bool autoConnect() const;

    qint16 port() const;

    QKlipperConsole *console() const;

    QKlipperServer *server() const;

    QKlipperSystem *system() const;

    QKlipperPrinter *printer() const;

    QString instanceLocation() const;

    QKlipperServer::ConnectionType connectionType() const;

    void connect();

public slots:
    void setName(const QString &name);

    void setAddress(const QString &address);

    void setId(const QString &id);

    void setApiKey(const QString &apiKey);

    void setAutoConnect(bool autoConnect);

    void setPort(qint16 port);

    void setInstanceLocation(const QString &instanceLocation);
    void printerStateChanged();

    void setConnectionType(QKlipperServer::ConnectionType connectionType);

signals:

    void nameChanged();

    void addressChanged();

    void idChanged();

    void apiKeyChanged();

    void autoConnectChanged();

    void portChanged();

    void instanceLocationChanged();

    void connectionTypeChanged();

private:
    QKlipperConsole *m_console = nullptr;
    QKlipperServer *m_server = nullptr;
    QKlipperSystem *m_system = nullptr;
    QKlipperPrinter *m_printer = nullptr;

    QString m_name;
    QString m_address;
    QString m_id;
    QString m_apiKey;
    QString m_instanceLocation;

    bool m_autoConnect = false;

    qint16 m_port;

    QKlipperServer::ConnectionType m_connectionType = QKlipperServer::None;

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged FINAL)
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged FINAL)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged FINAL)
    Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged FINAL)
    Q_PROPERTY(qint16 port READ port WRITE setPort NOTIFY portChanged FINAL)
    Q_PROPERTY(QString instanceLocation READ instanceLocation WRITE setInstanceLocation NOTIFY instanceLocationChanged FINAL)
    Q_PROPERTY(QKlipperServer::ConnectionType connectionType READ connectionType WRITE setConnectionType NOTIFY connectionTypeChanged FINAL)
};

#endif // QKLIPPERINSTANCE_H
