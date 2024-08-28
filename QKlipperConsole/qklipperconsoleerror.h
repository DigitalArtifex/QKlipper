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

#ifndef QKLIPPERCONSOLEERROR_H
#define QKLIPPERCONSOLEERROR_H

#include <QObject>

class QKlipperConsoleError : public QObject
{
    Q_OBJECT
public:

    enum Type
    {
        Unspecified = 0,
        Command = 1,
        Socket = 2,
        File = 3,
        NoError = 4
    };

    explicit QKlipperConsoleError(QObject *parent = nullptr);

    QKlipperConsoleError(const QKlipperConsoleError &value);
    QKlipperConsoleError(QKlipperConsoleError &&value);
    QKlipperConsoleError &operator=(const QKlipperConsoleError &value);
    QKlipperConsoleError &operator=(QKlipperConsoleError &&value);

    QString errorString() const;

    QString origin() const;

    Type type() const;

public slots:
    void setErrorString(const QString &errorString);

    void setOrigin(const QString &origin);

    void setType(Type type);

signals:

    void errorStringChanged();

    void originChanged();

    void typeChanged();

private:
    QString m_errorString;
    QString m_origin;
    Type m_type = Unspecified;

    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(QString origin READ origin WRITE setOrigin NOTIFY originChanged FINAL)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)
};

#endif // QKLIPPERCONSOLEERROR_H
