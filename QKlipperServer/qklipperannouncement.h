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

#ifndef QKLIPPERANNOUNCEMENT_H
#define QKLIPPERANNOUNCEMENT_H

#include <QObject>

class QKlipperSystem;
class QKlipperConsole;

class QKlipperAnnouncement : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperAnnouncement(QObject *parent = nullptr);

    QKlipperAnnouncement(const QKlipperAnnouncement &value);
    QKlipperAnnouncement(QKlipperAnnouncement &&value);
    QKlipperAnnouncement &operator=(const QKlipperAnnouncement &value);
    QKlipperAnnouncement &operator=(QKlipperAnnouncement &&value);

    bool operator==(const QKlipperAnnouncement &value)
    {
        if(m_isDismissed != value.m_isDismissed ||
           m_dateDismissed != value.m_dateDismissed ||
            m_dateDismissedWake != value.m_dateDismissedWake ||
            m_date != value.m_date ||
            m_description != value.m_description ||
            m_entryId != value.m_entryId ||
            m_feed != value.m_feed ||
            m_priority != value.m_priority ||
            m_source != value.m_source ||
            m_title != value.m_title ||
            m_url != value.m_url )
            return false;

        return true;
    }
    bool operator==(QKlipperAnnouncement &&value)
    {
        if(m_isDismissed != value.m_isDismissed ||
            m_dateDismissed != value.m_dateDismissed ||
            m_dateDismissedWake != value.m_dateDismissedWake ||
            m_date != value.m_date ||
            m_description != value.m_description ||
            m_entryId != value.m_entryId ||
            m_feed != value.m_feed ||
            m_priority != value.m_priority ||
            m_source != value.m_source ||
            m_title != value.m_title ||
            m_url != value.m_url )
            return false;

        return true;
    }
    bool operator!=(const QKlipperAnnouncement &value)
    {
        if(m_isDismissed == value.m_isDismissed &&
            m_dateDismissed == value.m_dateDismissed &&
            m_dateDismissedWake == value.m_dateDismissedWake &&
            m_date == value.m_date &&
            m_description == value.m_description &&
            m_entryId == value.m_entryId &&
            m_feed == value.m_feed &&
            m_priority == value.m_priority &&
            m_source == value.m_source &&
            m_title == value.m_title &&
            m_url == value.m_url )
            return false;

        return true;
    }
    bool operator!=(QKlipperAnnouncement &&value)
    {
        if(m_isDismissed == value.m_isDismissed &&
            m_dateDismissed == value.m_dateDismissed &&
            m_dateDismissedWake == value.m_dateDismissedWake &&
            m_date == value.m_date &&
            m_description == value.m_description &&
            m_entryId == value.m_entryId &&
            m_feed == value.m_feed &&
            m_priority == value.m_priority &&
            m_source == value.m_source &&
            m_title == value.m_title &&
            m_url == value.m_url )
            return false;

        return true;
    }

    QString entryId() const;

    QString url() const;

    QString title() const;

    QString description() const;

    QString priority() const;

    QString feed() const;

    QString source() const;

    qreal date() const;

    qreal dateDismissed() const;

    qreal dateDismissedWake() const;

    bool isDismissed() const;

private slots:
    void setEntryId(const QString &entryId);

    void setUrl(const QString &url);

    void setTitle(const QString &title);

    void setDescription(const QString &description);

    void setPriority(const QString &priority);

    void setFeed(const QString &feed);

    void setSource(const QString &source);

    void setDate(qreal date);

    void setDateDismissed(qreal dateDismissed);

    void setDateDismissedWake(qreal dateDismissedWake);

    void setIsDismissed(bool isDismissed);

signals:

    void entryIdChanged();

    void urlChanged();

    void titleChanged();

    void descriptionChanged();

    void priorityChanged();

    void feedChanged();

    void sourceChanged();

    void dateChanged();

    void dateDismissedChanged();

    void dateDismissedWakeChanged();

    void isDismissedChanged();

private:
    QString m_entryId;
    QString m_url;
    QString m_title;
    QString m_description;
    QString m_priority;
    QString m_feed;
    QString m_source;

    qreal m_date = 0;
    qreal m_dateDismissed = 0;
    qreal m_dateDismissedWake = 0;

    bool m_isDismissed = false;

    Q_PROPERTY(QString entryId READ entryId NOTIFY entryIdChanged FINAL)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged FINAL)
    Q_PROPERTY(QString priority READ priority NOTIFY priorityChanged FINAL)
    Q_PROPERTY(QString feed READ feed NOTIFY feedChanged FINAL)
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged FINAL)
    Q_PROPERTY(qreal date READ date NOTIFY dateChanged FINAL)
    Q_PROPERTY(qreal dateDismissed READ dateDismissed NOTIFY dateDismissedChanged FINAL)
    Q_PROPERTY(qreal dateDismissedWake READ dateDismissedWake NOTIFY dateDismissedWakeChanged FINAL)
    Q_PROPERTY(bool isDismissed READ isDismissed NOTIFY isDismissedChanged FINAL)
};

typedef QList<QKlipperAnnouncement> QKlipperAnnouncementList;

#endif // QKLIPPERANNOUNCEMENT_H
