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

#ifndef QKLIPPERSERVICE_H
#define QKLIPPERSERVICE_H

#include <QObject>

class QKlipperSystem;
class QKlipperConsole;

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperService : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperService(QObject *parent = nullptr);
    ~QKlipperService();

    bool operator==(const QKlipperService *value);
    bool operator!=(const QKlipperService *value);

    QString activeState() const;

    QString subState() const;

    QString name() const;

public slots:
    void setActiveState(const QString &activeState);

    void setSubState(const QString &subState);

    void setName(const QString &name);

private slots:

signals:

    void activeStateChanged();
    void subStateChanged();
    void nameChanged();

private:
    QString m_activeState;
    QString m_subState;
    QString m_name;

    Q_PROPERTY(QString activeState READ activeState WRITE setActiveState NOTIFY activeStateChanged FINAL)
    Q_PROPERTY(QString subState READ subState WRITE setSubState NOTIFY subStateChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
};

#endif // QKLIPPERSERVICE_H
