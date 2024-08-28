#include "qklipperupdatecommit.h"

QKlipperUpdateCommit::QKlipperUpdateCommit(QObject *parent)
    : QObject{parent}
{}

QKlipperUpdateCommit::QKlipperUpdateCommit(const QKlipperUpdateCommit &value)
{
    m_sha = value.m_sha;
    m_author = value.m_author;
    m_date = value.m_date;
    m_subject = value.m_subject;
    m_message = value.m_message;
    m_tag = value.m_tag;
}

QKlipperUpdateCommit::QKlipperUpdateCommit(QKlipperUpdateCommit &&value)
{
    m_sha = value.m_sha;
    m_author = value.m_author;
    m_date = value.m_date;
    m_subject = value.m_subject;
    m_message = value.m_message;
    m_tag = value.m_tag;
}

QKlipperUpdateCommit &QKlipperUpdateCommit::operator=(const QKlipperUpdateCommit &value)
{
    m_sha = value.m_sha;
    m_author = value.m_author;
    m_date = value.m_date;
    m_subject = value.m_subject;
    m_message = value.m_message;
    m_tag = value.m_tag;

    return *this;
}

QKlipperUpdateCommit &QKlipperUpdateCommit::operator=(QKlipperUpdateCommit &&value)
{
    m_sha = value.m_sha;
    m_author = value.m_author;
    m_date = value.m_date;
    m_subject = value.m_subject;
    m_message = value.m_message;
    m_tag = value.m_tag;

    return *this;
}

bool QKlipperUpdateCommit::operator==(const QKlipperUpdateCommit &value)
{
    if(m_sha != value.m_sha) return false;
    if(m_author != value.m_author) return false;
    if(m_date != value.m_date) return false;
    if(m_subject != value.m_subject) return false;
    if(m_message != value.m_message) return false;
    if(m_tag != value.m_tag) return false;

    return true;
}

bool QKlipperUpdateCommit::operator==(QKlipperUpdateCommit &&value)
{
    if(m_sha != value.m_sha) return false;
    if(m_author != value.m_author) return false;
    if(m_date != value.m_date) return false;
    if(m_subject != value.m_subject) return false;
    if(m_message != value.m_message) return false;
    if(m_tag != value.m_tag) return false;

    return true;
}

bool QKlipperUpdateCommit::operator!=(const QKlipperUpdateCommit &value)
{
    if(m_sha == value.m_sha &&
        m_author == value.m_author &&
        m_date == value.m_date &&
        m_subject == value.m_subject &&
        m_message == value.m_message &&
        m_tag == value.m_tag) return false;

    return true;
}

bool QKlipperUpdateCommit::operator!=(QKlipperUpdateCommit &&value)
{
    if(m_sha == value.m_sha &&
    m_author == value.m_author &&
    m_date == value.m_date &&
    m_subject == value.m_subject &&
    m_message == value.m_message &&
    m_tag == value.m_tag) return false;

    return true;
}

QString QKlipperUpdateCommit::sha() const
{
    return m_sha;
}

void QKlipperUpdateCommit::setSha(const QString &sha)
{
    if (m_sha == sha)
        return;
    m_sha = sha;
    emit shaChanged();
}

QString QKlipperUpdateCommit::author() const
{
    return m_author;
}

void QKlipperUpdateCommit::setAuthor(const QString &author)
{
    if (m_author == author)
        return;
    m_author = author;
    emit authorChanged();
}

QString QKlipperUpdateCommit::date() const
{
    return m_date;
}

void QKlipperUpdateCommit::setDate(const QString &date)
{
    if (m_date == date)
        return;
    m_date = date;
    emit dateChanged();
}

QString QKlipperUpdateCommit::subject() const
{
    return m_subject;
}

void QKlipperUpdateCommit::setSubject(const QString &subject)
{
    if (m_subject == subject)
        return;
    m_subject = subject;
    emit subjectChanged();
}

QString QKlipperUpdateCommit::message() const
{
    return m_message;
}

void QKlipperUpdateCommit::setMessage(const QString &message)
{
    if (m_message == message)
        return;
    m_message = message;
    emit messageChanged();
}

QString QKlipperUpdateCommit::tag() const
{
    return m_tag;
}

void QKlipperUpdateCommit::setTag(const QString &tag)
{
    if (m_tag == tag)
        return;
    m_tag = tag;
    emit tagChanged();
}
