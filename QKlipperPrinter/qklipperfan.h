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

#ifndef QKLIPPERFAN_H
#define QKLIPPERFAN_H

#include <QObject>

class QKlipperPrinter;
class QKlipperConsole;

class QKlipperFan : public QObject
{
    Q_OBJECT

    friend QKlipperConsole;
public:
    enum State
    {
        Off = 0,
        On = 1
    };

    explicit QKlipperFan(QObject *parent = nullptr);

    QString name() const;

    qreal speed() const;

    qreal rpm() const;

    bool isControllable() const;

    QKlipperPrinter *printer() const;

public slots:
    void setSpeed(qreal speed);

private slots:
    void setNameData(const QString &name);
    void resetName();

    void setSpeedData(qreal speed);
    void resetSpeed();

    void setRpmData(qreal rpm);
    void resetRpm();

    void setPrinter(QKlipperPrinter *printer);

    void setIsControllable(bool isControllable);
    void resetIsControllable();

signals:

    void nameChanged();

    void speedChanged();

    void rpmChanged();

    void isControllableChanged();

private:
    QString m_name;
    qreal m_speed = 0;
    qreal m_rpm = 0;
    bool m_isControllable = 0;

    QKlipperPrinter *m_printer = nullptr;
    QKlipperConsole *m_console = nullptr;

    Q_PROPERTY(QString name READ name RESET resetName NOTIFY nameChanged FINAL)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed RESET resetSpeed NOTIFY speedChanged FINAL)
    Q_PROPERTY(qreal rpm READ rpm RESET resetRpm NOTIFY rpmChanged FINAL)
    Q_PROPERTY(bool isControllable READ isControllable RESET resetIsControllable NOTIFY isControllableChanged FINAL)
};

#endif // QKLIPPERFAN_H
