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

#ifndef QKLIPPERMCU_H
#define QKLIPPERMCU_H

#include <QObject>

class QKlipperSystem;
class QKlipperConsole;

class QKlipperMCU : public QObject
{
    Q_OBJECT

    friend QKlipperSystem;
    friend QKlipperConsole;
public:
    explicit QKlipperMCU(QObject *parent = nullptr);

    QKlipperMCU(const QKlipperMCU &value);
    QKlipperMCU(QKlipperMCU &&value);
    QKlipperMCU &operator=(const QKlipperMCU &value);
    QKlipperMCU &operator=(QKlipperMCU &&value);
    bool operator==(const QKlipperMCU &value);
    bool operator==(QKlipperMCU &&value);
    bool operator!=(const QKlipperMCU &value);
    bool operator!=(QKlipperMCU &&value);

    qint64 baudRate() const;

    qint64 sequenceRecieved() const;

    qint64 sequenceSent() const;

    qint64 sequenceRetransmitted() const;

    qint64 bytesInvalid() const;

    qint64 bytesRead() const;

    qint64 bytesRetransmitted() const;

    qint64 bytesWritten() const;

    qint64 bytesUpcoming() const;

    qint64 bytesAvailable() const;

    qint64 frequency() const;

    QString serial() const;

    QString firmwareVersion() const;

    QString hardwareVersion() const;

    qreal awake() const;

    qreal taskAverage() const;

    qreal devAverage() const;

    qreal maxStepperError() const;

public slots:

    void setBaudRate(qint64 baudRate);

    void setSequenceRecieved(qint64 sequenceRecieved);

    void setSequenceSent(qint64 sequenceSent);

    void setSequenceRetransmitted(qint64 sequenceRetransmitted);

    void setBytesInvalid(qint64 bytesInvalid);

    void setBytesRead(qint64 bytesRead);

    void setBytesRetransmitted(qint64 bytesRetransmitted);

    void setBytesWritten(qint64 bytesWritten);

    void setBytesUpcoming(qint64 bytesUpcoming);

    void setBytesAvailable(qint64 bytesAvailable);

    void setFrequency(qint64 frequency);

    void setSerial(const QString &serial);

    void setFirmwareVersion(const QString &firmwareVersion);

    void setHardwareVersion(const QString &hardwareVersion);

    void setAwake(qreal awake);

    void setTaskAverage(qreal taskAverage);

    void setDevAverage(qreal devAverage);

    void setMaxStepperError(qreal maxStepperError);

signals:

    void baudRateChanged();
    void sequenceRecievedChanged();
    void sequenceSentChanged();
    void sequenceRetransmittedChanged();
    void bytesInvalidChanged();
    void bytesReadChanged();
    void bytesRetransmittedChanged();
    void bytesWrittenChanged();
    void bytesUpcomingChanged();
    void bytesAvailableChanged();
    void frequencyChanged();
    void serialChanged();
    void firmwareVersionChanged();
    void hardwareVersionChanged();
    void awakeChanged();
    void taskAverageChanged();
    void devAverageChanged();
    void maxStepperErrorChanged();

private:
    qint64 m_baudRate = 0;
    qint64 m_sequenceRecieved = 0;
    qint64 m_sequenceSent = 0;
    qint64 m_sequenceRetransmitted = 0;
    qint64 m_bytesInvalid = 0;
    qint64 m_bytesRead = 0;
    qint64 m_bytesRetransmitted = 0;
    qint64 m_bytesWritten = 0;
    qint64 m_bytesUpcoming = 0;
    qint64 m_bytesAvailable = 0;
    qint64 m_frequency = 0;

    QString m_serial;
    QString m_firmwareVersion;
    QString m_hardwareVersion;

    qreal m_awake = 0.0;
    qreal m_taskAverage = 0.0;
    qreal m_devAverage = 0.0;
    qreal m_maxStepperError = 0;

    Q_PROPERTY(qint64 baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged FINAL)
    Q_PROPERTY(qint64 sequenceRecieved READ sequenceRecieved WRITE setSequenceRecieved NOTIFY sequenceRecievedChanged FINAL)
    Q_PROPERTY(qint64 sequenceSent READ sequenceSent WRITE setSequenceSent NOTIFY sequenceSentChanged FINAL)
    Q_PROPERTY(qint64 sequenceRetransmitted READ sequenceRetransmitted WRITE setSequenceRetransmitted NOTIFY sequenceRetransmittedChanged FINAL)
    Q_PROPERTY(qint64 bytesInvalid READ bytesInvalid WRITE setBytesInvalid NOTIFY bytesInvalidChanged FINAL)
    Q_PROPERTY(qint64 bytesRead READ bytesRead WRITE setBytesRead NOTIFY bytesReadChanged FINAL)
    Q_PROPERTY(qint64 bytesRetransmitted READ bytesRetransmitted WRITE setBytesRetransmitted NOTIFY bytesRetransmittedChanged FINAL)
    Q_PROPERTY(qint64 bytesWritten READ bytesWritten WRITE setBytesWritten NOTIFY bytesWrittenChanged FINAL)
    Q_PROPERTY(qint64 bytesUpcoming READ bytesUpcoming WRITE setBytesUpcoming NOTIFY bytesUpcomingChanged FINAL)
    Q_PROPERTY(qint64 bytesAvailable READ bytesAvailable WRITE setBytesAvailable NOTIFY bytesAvailableChanged FINAL)
    Q_PROPERTY(qint64 frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged FINAL)
    Q_PROPERTY(QString serial READ serial WRITE setSerial NOTIFY serialChanged FINAL)
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion WRITE setFirmwareVersion NOTIFY firmwareVersionChanged FINAL)
    Q_PROPERTY(QString hardwareVersion READ hardwareVersion WRITE setHardwareVersion NOTIFY hardwareVersionChanged FINAL)
    Q_PROPERTY(qreal awake READ awake WRITE setAwake NOTIFY awakeChanged FINAL)
    Q_PROPERTY(qreal taskAverage READ taskAverage WRITE setTaskAverage NOTIFY taskAverageChanged FINAL)
    Q_PROPERTY(qreal devAverage READ devAverage WRITE setDevAverage NOTIFY devAverageChanged FINAL)
    Q_PROPERTY(qreal maxStepperError READ maxStepperError WRITE setMaxStepperError NOTIFY maxStepperErrorChanged FINAL)
};

#endif // QKLIPPERMCU_H