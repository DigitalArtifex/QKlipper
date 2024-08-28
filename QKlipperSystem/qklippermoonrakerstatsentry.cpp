#include "qklippermoonrakerstatsentry.h"

QKlipperMoonrakerStatsEntry::QKlipperMoonrakerStatsEntry(QObject *parent)
    : QObject{parent}
{}

QKlipperMoonrakerStatsEntry::QKlipperMoonrakerStatsEntry(const QKlipperMoonrakerStatsEntry &value)
{
    m_time = value.m_time;
    m_cpuUsage = value.m_cpuUsage;
    m_memory = value.m_memory;
}

QKlipperMoonrakerStatsEntry::QKlipperMoonrakerStatsEntry(QKlipperMoonrakerStatsEntry &&value)
{
    m_time = value.m_time;
    m_cpuUsage = value.m_cpuUsage;
    m_memory = value.m_memory;
}

QKlipperMoonrakerStatsEntry &QKlipperMoonrakerStatsEntry::operator=(const QKlipperMoonrakerStatsEntry &value)
{
    m_time = value.m_time;
    m_cpuUsage = value.m_cpuUsage;
    m_memory = value.m_memory;

    return *this;
}

QKlipperMoonrakerStatsEntry &QKlipperMoonrakerStatsEntry::operator=(QKlipperMoonrakerStatsEntry &&value)
{
    m_time = value.m_time;
    m_cpuUsage = value.m_cpuUsage;
    m_memory = value.m_memory;

    return *this;
}

bool QKlipperMoonrakerStatsEntry::operator==(const QKlipperMoonrakerStatsEntry &value)
{
    if(m_time != value.m_time) return false;
    if(m_cpuUsage != value.m_cpuUsage) return false;
    if(m_memory != value.m_memory) return false;

    return true;
}

bool QKlipperMoonrakerStatsEntry::operator==(QKlipperMoonrakerStatsEntry &&value)
{
    if(m_time != value.m_time) return false;
    if(m_cpuUsage != value.m_cpuUsage) return false;
    if(m_memory != value.m_memory) return false;

    return true;
}

bool QKlipperMoonrakerStatsEntry::operator!=(const QKlipperMoonrakerStatsEntry &value)
{
    if(m_time == value.m_time &&
        m_cpuUsage == value.m_cpuUsage &&
        m_memory == value.m_memory)
        return false;

    return true;
}

bool QKlipperMoonrakerStatsEntry::operator!=(QKlipperMoonrakerStatsEntry &&value)
{
    if(m_time == value.m_time &&
        m_cpuUsage == value.m_cpuUsage &&
        m_memory == value.m_memory)
        return false;

    return true;
}

qreal QKlipperMoonrakerStatsEntry::time() const
{
    return m_time;
}

void QKlipperMoonrakerStatsEntry::setTime(qreal time)
{
    if (qFuzzyCompare(m_time, time))
        return;

    m_time = time;
    emit timeChanged();
}

qreal QKlipperMoonrakerStatsEntry::cpuUsage() const
{
    return m_cpuUsage;
}

void QKlipperMoonrakerStatsEntry::setCpuUsage(qreal cpuUsage)
{
    if (qFuzzyCompare(m_cpuUsage, cpuUsage))
        return;
    m_cpuUsage = cpuUsage;
    emit cpuUsageChanged();
}

qreal QKlipperMoonrakerStatsEntry::memory() const
{
    return m_memory;
}

void QKlipperMoonrakerStatsEntry::setMemory(qreal memory)
{
    if (qFuzzyCompare(m_memory, memory))
        return;
    m_memory = memory;
    emit memoryChanged();
}

QString QKlipperMoonrakerStatsEntry::memoryUnits() const
{
    return m_memoryUnits;
}

void QKlipperMoonrakerStatsEntry::setMemoryUnits(const QString &memoryUnits)
{
    if (m_memoryUnits == memoryUnits)
        return;
    m_memoryUnits = memoryUnits;
    emit memoryUnitsChanged();
}

