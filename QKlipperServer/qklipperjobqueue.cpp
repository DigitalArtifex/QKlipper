#include "qklipperjobqueue.h"

QKlipperJobQueue::QKlipperJobQueue(QObject *parent)
    : QObject{parent}
{}

QList<QKlipperPrintJob*> QKlipperJobQueue::queue() const
{
    return m_queue;
}

void QKlipperJobQueue::setQueue(const QList<QKlipperPrintJob *> &queue)
{
    if (m_queue == queue)
        return;

    foreach(QKlipperPrintJob *job, m_queue)
    {
        m_queue.removeAll(job);
        delete job;
    }

    m_queue = queue;
    emit queueChanged();
}

void QKlipperJobQueue::addJob(QKlipperPrintJob *job)
{
    if(!m_queue.contains(job))
    {
        m_queue.append(job);
        emit queueChanged();
    }
}

void QKlipperJobQueue::removeJob(QKlipperPrintJob *job)
{
    if(m_queue.contains(job))
    {
        m_queue.removeAll(job);
        emit queueChanged();

        delete job;
        job = nullptr;
    }
}

void QKlipperJobQueue::removeJob(const QString &jobId)
{
    foreach(QKlipperPrintJob *job, m_queue)
    {
        if(job->id() == jobId)
        {
            m_queue.removeAll(job);
            emit queueChanged();

            delete job;
            job = nullptr;

            break;
        }
    }
}

void QKlipperJobQueue::clearJobs()
{
    foreach(QKlipperPrintJob *job, m_queue)
    {
        m_queue.removeAll(job);
        delete job;
    }
}

QKlipperJobQueue::State QKlipperJobQueue::state() const
{
    return m_state;
}

QKlipperPrintJob *QKlipperJobQueue::job(QString id)
{
    foreach(QKlipperPrintJob *printJob, m_queue)
    {
        if(printJob->id() == id)
            return printJob;
    }

    return nullptr;
}

void QKlipperJobQueue::setState(State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged();
}
