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

#ifndef QKLIPPERMOONRAKERSTATSENTRY_H
#define QKLIPPERMOONRAKERSTATSENTRY_H

#include <QObject>
class QKlipperSystem;
class QKlipperConsole;
/*!
 * \brief Filled by machine.proc_stats
 */
class QKlipperMoonrakerStatsEntry : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperMoonrakerStatsEntry(QObject *parent = nullptr);

    QKlipperMoonrakerStatsEntry(const QKlipperMoonrakerStatsEntry &value);
    QKlipperMoonrakerStatsEntry(QKlipperMoonrakerStatsEntry &&value);
    QKlipperMoonrakerStatsEntry &operator=(const QKlipperMoonrakerStatsEntry &value);
    QKlipperMoonrakerStatsEntry &operator=(QKlipperMoonrakerStatsEntry &&value);
    bool operator==(const QKlipperMoonrakerStatsEntry &value);
    bool operator==(QKlipperMoonrakerStatsEntry &&value);
    bool operator!=(const QKlipperMoonrakerStatsEntry &value);
    bool operator!=(QKlipperMoonrakerStatsEntry &&value);

    qreal time() const;

    qreal cpuUsage() const;

    qreal memory() const;

    QString memoryUnits() const;

public slots:
    void setTime(qreal time);

    void setCpuUsage(qreal cpuUsage);

    void setMemory(qreal memory);

    void setMemoryUnits(const QString &memoryUnits);

private slots:

signals:
    void timeChanged();
    void cpuUsageChanged();
    void memoryChanged();
    void memoryUnitsChanged();

private:
    qreal m_time = 0;
    qreal m_cpuUsage = 0;
    qreal m_memory = 0;

    QString m_memoryUnits;

    Q_PROPERTY(qreal time READ time WRITE setTime NOTIFY timeChanged FINAL)
    Q_PROPERTY(qreal cpuUsage READ cpuUsage WRITE setCpuUsage NOTIFY cpuUsageChanged FINAL)
    Q_PROPERTY(qreal memory READ memory WRITE setMemory NOTIFY memoryChanged FINAL)
    Q_PROPERTY(QString memoryUnits READ memoryUnits WRITE setMemoryUnits NOTIFY memoryUnitsChanged FINAL)
};

#endif // QKLIPPERMOONRAKERSTATSENTRY_H
