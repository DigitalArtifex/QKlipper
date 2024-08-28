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

#ifndef QKLIPPERUPDATECOMMIT_H
#define QKLIPPERUPDATECOMMIT_H

#include <QObject>

class QKlipperUpdateCommit : public QObject
{
    Q_OBJECT
public:
    explicit QKlipperUpdateCommit(QObject *parent = nullptr);

    QKlipperUpdateCommit(const QKlipperUpdateCommit &value);
    QKlipperUpdateCommit(QKlipperUpdateCommit &&value);
    QKlipperUpdateCommit &operator=(const QKlipperUpdateCommit &value);
    QKlipperUpdateCommit &operator=(QKlipperUpdateCommit &&value);
    bool operator==(const QKlipperUpdateCommit &value);
    bool operator==(QKlipperUpdateCommit &&value);
    bool operator!=(const QKlipperUpdateCommit &value);
    bool operator!=(QKlipperUpdateCommit &&value);

    QString sha() const;

    QString author() const;

    QString date() const;

    QString subject() const;

    QString message() const;

    QString tag() const;

public slots:
    void setSha(const QString &sha);

    void setAuthor(const QString &author);

    void setDate(const QString &date);

    void setSubject(const QString &subject);

    void setMessage(const QString &message);

    void setTag(const QString &tag);

signals:

    void shaChanged();

    void authorChanged();

    void dateChanged();

    void subjectChanged();

    void messageChanged();

    void tagChanged();

private:
    QString m_sha;
    QString m_author;
    QString m_date;
    QString m_subject;
    QString m_message;
    QString m_tag;

    Q_PROPERTY(QString sha READ sha WRITE setSha NOTIFY shaChanged FINAL)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged FINAL)
    Q_PROPERTY(QString date READ date WRITE setDate NOTIFY dateChanged FINAL)
    Q_PROPERTY(QString subject READ subject WRITE setSubject NOTIFY subjectChanged FINAL)
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged FINAL)
    Q_PROPERTY(QString tag READ tag WRITE setTag NOTIFY tagChanged FINAL)
};

#endif // QKLIPPERUPDATECOMMIT_H
