#include "qklipperannouncement.h"

QKlipperAnnouncement::QKlipperAnnouncement(QObject *parent)
    : QObject{parent}
{

}

QKlipperAnnouncement::QKlipperAnnouncement(const QKlipperAnnouncement &value)
{
    m_entryId = value.entryId();
    m_date = value.date();
    m_dateDismissed = value.dateDismissed();
    m_isDismissed = value.isDismissed();
    m_description = value.description();
    m_feed = value.feed();
    m_priority = value.priority();
    m_source = value.source();
    m_title = value.title();
    m_url = value.url();
}

QKlipperAnnouncement::QKlipperAnnouncement(QKlipperAnnouncement &&value)
{
    m_entryId = value.entryId();
    m_date = value.date();
    m_dateDismissed = value.dateDismissed();
    m_isDismissed = value.isDismissed();
    m_description = value.description();
    m_feed = value.feed();
    m_priority = value.priority();
    m_source = value.source();
    m_title = value.title();
    m_url = value.url();
}

QKlipperAnnouncement &QKlipperAnnouncement::operator=(const QKlipperAnnouncement &value)
{
    m_entryId = value.entryId();
    m_date = value.date();
    m_dateDismissed = value.dateDismissed();
    m_isDismissed = value.isDismissed();
    m_description = value.description();
    m_feed = value.feed();
    m_priority = value.priority();
    m_source = value.source();
    m_title = value.title();
    m_url = value.url();

    return *this;
}

QKlipperAnnouncement &QKlipperAnnouncement::operator=(QKlipperAnnouncement &&value)
{
    m_entryId = value.entryId();
    m_date = value.date();
    m_dateDismissed = value.dateDismissed();
    m_isDismissed = value.isDismissed();
    m_description = value.description();
    m_feed = value.feed();
    m_priority = value.priority();
    m_source = value.source();
    m_title = value.title();
    m_url = value.url();

    return *this;
}

QString QKlipperAnnouncement::entryId() const
{
    return m_entryId;
}

void QKlipperAnnouncement::setEntryId(const QString &entryId)
{
    if (m_entryId == entryId)
        return;

    m_entryId = entryId;
    emit entryIdChanged();
}

QString QKlipperAnnouncement::url() const
{
    return m_url;
}

void QKlipperAnnouncement::setUrl(const QString &url)
{
    if (m_url == url)
        return;
    m_url = url;
    emit urlChanged();
}

QString QKlipperAnnouncement::title() const
{
    return m_title;
}

void QKlipperAnnouncement::setTitle(const QString &title)
{
    if (m_title == title)
        return;
    m_title = title;
    emit titleChanged();
}

QString QKlipperAnnouncement::description() const
{
    return m_description;
}

void QKlipperAnnouncement::setDescription(const QString &description)
{
    if (m_description == description)
        return;
    m_description = description;
    emit descriptionChanged();
}

QString QKlipperAnnouncement::priority() const
{
    return m_priority;
}

void QKlipperAnnouncement::setPriority(const QString &priority)
{
    if (m_priority == priority)
        return;
    m_priority = priority;
    emit priorityChanged();
}

QString QKlipperAnnouncement::feed() const
{
    return m_feed;
}

void QKlipperAnnouncement::setFeed(const QString &feed)
{
    if (m_feed == feed)
        return;
    m_feed = feed;
    emit feedChanged();
}

QString QKlipperAnnouncement::source() const
{
    return m_source;
}

void QKlipperAnnouncement::setSource(const QString &source)
{
    if (m_source == source)
        return;
    m_source = source;
    emit sourceChanged();
}

qreal QKlipperAnnouncement::date() const
{
    return m_date;
}

void QKlipperAnnouncement::setDate(qreal date)
{
    if (qFuzzyCompare(m_date, date))
        return;
    m_date = date;
    emit dateChanged();
}

qreal QKlipperAnnouncement::dateDismissed() const
{
    return m_dateDismissed;
}

void QKlipperAnnouncement::setDateDismissed(qreal dateDismissed)
{
    if (qFuzzyCompare(m_dateDismissed, dateDismissed))
        return;
    m_dateDismissed = dateDismissed;
    emit dateDismissedChanged();
}

qreal QKlipperAnnouncement::dateDismissedWake() const
{
    return m_dateDismissedWake;
}

void QKlipperAnnouncement::setDateDismissedWake(qreal dateDismissedWake)
{
    if (qFuzzyCompare(m_dateDismissedWake, dateDismissedWake))
        return;
    m_dateDismissedWake = dateDismissedWake;
    emit dateDismissedWakeChanged();
}

bool QKlipperAnnouncement::isDismissed() const
{
    return m_isDismissed;
}

void QKlipperAnnouncement::setIsDismissed(bool isDismissed)
{
    if (m_isDismissed == isDismissed)
        return;

    m_isDismissed = isDismissed;
    emit isDismissedChanged();
}
