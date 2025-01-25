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

#ifndef QKLIPPERTHUMBNAIL_H
#define QKLIPPERTHUMBNAIL_H

#include <QObject>
#include <QVariant>

#include "QKlipper/dalib_global.h"

class DA_EXPORT QKlipperThumbnail
{
public:
    qint32 width = 0;
    qint32 height = 0;
    qint64 size = 0;

    QString relativePath;

    QKlipperThumbnail() = default;
    ~QKlipperThumbnail() = default;

    QKlipperThumbnail(const QKlipperThumbnail &value)
    {
        width = value.width;
        height = value.height;
        size = value.size;
        relativePath = value.relativePath;
    }

    QKlipperThumbnail &operator=(const QKlipperThumbnail &value)
    {
        width = value.width;
        height = value.height;
        size = value.size;
        relativePath = value.relativePath;

        return *this;
    }

    bool operator==(const QKlipperThumbnail &value) const
    {
        if(width != value.width)
            return false;
        if(height != value.height)
            return false;
        if(size != value.size)
            return false;
        if(relativePath != value.relativePath)
            return false;

        return true;
    }

    bool operator!=(const QKlipperThumbnail &value) const { return !(*this == value); }
};

Q_DECLARE_METATYPE(QKlipperThumbnail)
#endif // QKLIPPERTHUMBNAIL_H
