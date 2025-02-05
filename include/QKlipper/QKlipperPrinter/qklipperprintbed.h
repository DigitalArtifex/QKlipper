/*!
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

#ifndef QKLIPPERPRINTBED_H
#define QKLIPPERPRINTBED_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "qklipperprintbedmesh.h"
#include "qklipperadjustmentscrew.h"
#include "qklipperheater.h"

class QKlipperPrinter;
class QKlipperConsole;

#include "QKlipper/dalib_global.h"


//!  QKlipperPrintBed class
/*!
  This class is responsible for the actual communication to the klipper instance via moonraker.
  It provides methods for the moonraker API for controlling and querying aspects of the klipper
  instance.

  Prior to connecting, it is required to pass instance references to QKlipperPrinter, QKlipperSystem
  and QKlipperServer that have been setup and configured.
*/
class DA_EXPORT QKlipperPrintBed : public QKlipperHeater
{
    Q_OBJECT
    friend QKlipperConsole;
    friend QKlipperPrinter;

public:
    enum Type
    {
        Heated,
        NonHeated
    };

    /*!
     * Constructor
     *
     * \param parent The parent object
     */
    explicit QKlipperPrintBed(QObject *parent = nullptr);

    /*!
     * Destructor
     */
    ~QKlipperPrintBed();

    QDateTime startTime() const;

    QKlipperPrintBedMesh *bedMesh() const;

    QMap<QString, QKlipperAdjustmentScrew *> adjustmentScrews() const;
    QKlipperAdjustmentScrew *adjustmentScrew(const QString &name);

    bool adjustmentScrewsError() const;

    bool hasAdjustmentScrewResult() const;

    bool hasBedMeshResult() const;

    bool isAdjustmentScrewsCalibrating() const;

    bool isBedMeshCalibrating() const;

    qreal adjustmentScrewsMaxDeviation() const;

    bool tiltAdjustError() const;

    Type type() const;

public slots:
    /*!
     * Sends a gcode script to set the requested temperature
     *
     * \param targetTemp The value (in Celsius) to set (0-maxTemp)
     */
    virtual void setTargetTemp(qreal targetTemp) override;

    /*!
     * Sends a gcode script to probe the bed corners to determine adjustment values
     */
    void calibrateAdjustmentScrews();

    /*!
     * Sends a gcode script to probe the entire bed to generate the bed mesh
     */
    void calibrateBedMesh();

private slots:
    void setStartTime(const QDateTime &startTime);

    void setBedMesh(QKlipperPrintBedMesh *bedMesh);

    void setAdjustmentScrews(const QMap<QString, QKlipperAdjustmentScrew *> &adjustmentScrews);
    void setAdjustmentScrew(QString name, QKlipperAdjustmentScrew *adjustmentScrew);

    void setAdjustmentScrewsError(bool adjustmentScrewsError);

    void setHasAdjustmentScrewResult(bool hasAdjustmentScrewResult);

    void setHasBedMeshResult(bool hasBedMeshResult);

    void setAdjustmentScrewsCalibrating(bool adjustmentScrewsCalibrating);

    void setBedMeshCalibrating(bool bedMeshCalibrating);

    void setAdjustmentScrewsMaxDeviation(qreal adjustmentScrewsMaxDeviation);

    void setTiltAdjustError(bool tiltAdjustError);

    void setType(Type type);

signals:
    void startTimeChanged();
    void bedMeshChanged();
    void adjustmentScrewsChanged();
    void adjustmentScrewsErrorChanged();
    void hasAdjustmentScrewResultChanged();
    void hasBedMeshResultChanged();
    void adjustmentScrewsCalibratingChanged();
    void bedMeshCalibratingChanged();
    void adjustmentScrewsMaxDeviationChanged();
    void typeChanged();
    void adjustmentScrewsCalibrating();
    void adjustmentScrewsCalibratingFinished();
    void bedMeshCalibrating();
    void bedMeshCalibratingFinished();
    void tiltAdjustErrorChanged();

private:
    QDateTime m_startTime;

    QKlipperPrintBedMesh *m_bedMesh;

    QMap<QString, QKlipperAdjustmentScrew*>  m_adjustmentScrews;
    bool                                     m_adjustmentScrewsError = false;
    bool                                     m_hasAdjustmentScrewResult = false;
    bool                                     m_hasBedMeshResult = false;
    bool                                     m_adjustmentScrewsCalibrating = false;
    bool                                     m_bedMeshCalibrating = false;
    qreal                                    m_adjustmentScrewsMaxDeviation = 0;

    bool                                     m_tiltAdjustError = false;

    Type                                     m_type = NonHeated;
};

Q_DECLARE_METATYPE(QKlipperPrintBed::Type)
#endif // QKLIPPERPRINTBED_H
