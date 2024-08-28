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

#ifndef QKLIPPERPRINTBEDMESH_H
#define QKLIPPERPRINTBEDMESH_H

#include <QObject>

class QKlipperPrintBedMesh : public QObject
{
    Q_OBJECT
public:
    struct Limit
    {
        qreal x;
        qreal y;
    };

    explicit QKlipperPrintBedMesh(QObject *parent = nullptr);

    qreal fadeEnd() const;

    qreal fadeStart() const;

    qreal fadeTarget() const;

    qreal horizontalMoveZ() const;

    qreal adaptiveMargin() const;

    qreal speed() const;

    qreal tension() const;

    qreal splitDeltaZ() const;

    qreal moveCheckDistance() const;

    QString profileName() const;

    QStringList profiles() const;

    Limit minimum() const;

    Limit maximum() const;

    Limit probeCount() const;

    QList<QList<qreal> > matrix() const;

    QList<QList<qreal> > probed() const;

    QString algorithm() const;

public slots:
    void setFadeEnd(qreal fadeEnd);

    void setFadeStart(qreal fadeStart);

    void setFadeTarget(qreal fadeTarget);

    void setHorizontalMoveZ(qreal horizontalMoveZ);

    void setAdaptiveMargin(qreal adaptiveMargin);

    void setSpeed(qreal speed);

    void setTension(qreal tension);

    void setSplitDeltaZ(qreal splitDeltaZ);

    void setMoveCheckDistance(qreal moveCheckDistance);

    void setProfileName(const QString &profileName);

    void setProfiles(const QStringList &profiles);

    void setMinimum(const Limit &minimum);

    void setMaximum(const Limit &maximum);

    void setProbeCount(const Limit &probeCount);

    void setMatrix(const QList<QList<qreal> > &matrix);

    void setProbed(const QList<QList<qreal> > &probed);

    void setAlgorithm(const QString &algorithm);

signals:

    void fadeEndChanged();

    void fadeStartChanged();

    void fadeTargetChanged();

    void horizontalMoveZChanged();

    void adaptiveMarginChanged();

    void speedChanged();

    void tensionChanged();

    void splitDeltaZChanged();

    void moveCheckDistanceChanged();

    void profileNameChanged();

    void profilesChanged();

    void minimumChanged();

    void maximumChanged();

    void probeCountChanged();

    void matrixChanged();

    void probedChanged();

    void algorithmChanged();

private:

    qreal m_fadeEnd = 0;
    qreal m_fadeStart = 0;
    qreal m_fadeTarget = 0;
    qreal m_horizontalMoveZ = 0;
    qreal m_adaptiveMargin = 0;
    qreal m_speed = 0;
    qreal m_tension = 0;
    qreal m_splitDeltaZ = 0;
    qreal m_moveCheckDistance = 0;

    QString m_profileName;
    QStringList m_profiles;

    Limit m_minimum;
    Limit m_maximum;
    Limit m_probeCount;

    QList<QList<qreal>> m_matrix;
    QList<QList<qreal>> m_probed;

    QString m_algorithm;

    Q_PROPERTY(qreal fadeEnd READ fadeEnd WRITE setFadeEnd NOTIFY fadeEndChanged FINAL)
    Q_PROPERTY(qreal fadeStart READ fadeStart WRITE setFadeStart NOTIFY fadeStartChanged FINAL)
    Q_PROPERTY(qreal fadeTarget READ fadeTarget WRITE setFadeTarget NOTIFY fadeTargetChanged FINAL)
    Q_PROPERTY(qreal horizontalMoveZ READ horizontalMoveZ WRITE setHorizontalMoveZ NOTIFY horizontalMoveZChanged FINAL)
    Q_PROPERTY(qreal adaptiveMargin READ adaptiveMargin WRITE setAdaptiveMargin NOTIFY adaptiveMarginChanged FINAL)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged FINAL)
    Q_PROPERTY(qreal tension READ tension WRITE setTension NOTIFY tensionChanged FINAL)
    Q_PROPERTY(qreal splitDeltaZ READ splitDeltaZ WRITE setSplitDeltaZ NOTIFY splitDeltaZChanged FINAL)
    Q_PROPERTY(qreal moveCheckDistance READ moveCheckDistance WRITE setMoveCheckDistance NOTIFY moveCheckDistanceChanged FINAL)
    Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileNameChanged FINAL)
    Q_PROPERTY(QStringList profiles READ profiles WRITE setProfiles NOTIFY profilesChanged FINAL)
    Q_PROPERTY(Limit minimum READ minimum WRITE setMinimum NOTIFY minimumChanged FINAL)
    Q_PROPERTY(Limit maximum READ maximum WRITE setMaximum NOTIFY maximumChanged FINAL)
    Q_PROPERTY(Limit probeCount READ probeCount WRITE setProbeCount NOTIFY probeCountChanged FINAL)
    Q_PROPERTY(QList<QList<qreal> > matrix READ matrix WRITE setMatrix NOTIFY matrixChanged FINAL)
    Q_PROPERTY(QList<QList<qreal> > probed READ probed WRITE setProbed NOTIFY probedChanged FINAL)
    Q_PROPERTY(QString algorithm READ algorithm WRITE setAlgorithm NOTIFY algorithmChanged FINAL)
};

#endif // QKLIPPERPRINTBEDMESH_H