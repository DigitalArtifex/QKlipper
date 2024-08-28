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

#ifndef QKLIPPERMETADATA_H
#define QKLIPPERMETADATA_H

#include <QObject>
#include <QList>

#include "qklipperthumbnail.h"

//File metadata
class QKlipperMetadata
{
    Q_GADGET
public:
    qreal printStartTime = 0;
    qreal modified = 0;
    qreal layerHeight = 0;
    qreal firstLayerHeight = 0;
    qreal firstLayerBedTemp = 0;
    qreal firstLayerExtruderTemp = 0;
    qreal objectHeight = 0;
    qreal filamentTotal = 0;
    qreal filamentTotalWeight = 0;
    qreal nozzleDiameter = 0;

    qint32 jobId = 0;
    qint64 estimatedTime = 0;
    qint64 gcodeStartByte = 0;
    qint64 gcodeEndByte = 0;
    qint64 size = 0;

    QString filename;
    QString filamentName;
    QString slicer;
    QString slicerVersion;
    QString uuid;

    QList<const QKlipperThumbnail> thumbnails;

    bool operator==(const QKlipperMetadata &value)
    {
        return memcmp(this, &value, sizeof(QKlipperMetadata));
    }
};

Q_DECLARE_METATYPE(QKlipperMetadata)

#endif // QKLIPPERMETADATA_H
