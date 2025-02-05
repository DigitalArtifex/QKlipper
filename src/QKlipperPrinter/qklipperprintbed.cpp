#include <QKlipper/QKlipperPrinter/qklipperprintbed.h>
#include <QKlipper/QKlipperConsole/qklipperconsole.h>

QKlipperPrintBed::QKlipperPrintBed(QObject *parent)
    : QKlipperHeater{parent}
{
    m_bedMesh = new QKlipperPrintBedMesh(this);
}

QKlipperPrintBed::~QKlipperPrintBed()
{
    if(m_bedMesh)
        m_bedMesh->deleteLater();

    foreach(QKlipperAdjustmentScrew *screw, m_adjustmentScrews)
        screw->deleteLater();
}

void QKlipperPrintBed::calibrateAdjustmentScrews()
{
    setHasAdjustmentScrewResult(false);
    emit adjustmentScrewsCalibrating();

    QString gcode("SCREWS_TILT_CALCULATE");
    m_console->printerGcodeScript(gcode);

    emit adjustmentScrewsCalibratingFinished();
}

void QKlipperPrintBed::calibrateBedMesh()
{
    setHasBedMeshResult(false);
    m_bedMesh->setReportedProbePoints(0);

    emit bedMeshCalibrating();

    QString gcode("BED_MESH_CALIBRATE");
    m_console->printerGcodeScript(gcode);

    emit bedMeshCalibratingFinished();
}

QDateTime QKlipperPrintBed::startTime() const
{
    return m_startTime;
}

void QKlipperPrintBed::setStartTime(const QDateTime &startTime)
{
    if (m_startTime == startTime)
        return;
    m_startTime = startTime;
    emit startTimeChanged();
}

QKlipperPrintBedMesh *QKlipperPrintBed::bedMesh() const
{
    return m_bedMesh;
}

void QKlipperPrintBed::setBedMesh(QKlipperPrintBedMesh *bedMesh)
{
    if (m_bedMesh == bedMesh)
        return;
    m_bedMesh = bedMesh;
    emit bedMeshChanged();
}

QMap<QString, QKlipperAdjustmentScrew *> QKlipperPrintBed::adjustmentScrews() const
{
    return m_adjustmentScrews;
}

QKlipperAdjustmentScrew *QKlipperPrintBed::adjustmentScrew(const QString &name)
{
    if(m_adjustmentScrews.contains(name))
        return m_adjustmentScrews[name];

    return nullptr;
}

void QKlipperPrintBed::setAdjustmentScrews(const QMap<QString, QKlipperAdjustmentScrew *> &adjustmentScrews)
{
    if (m_adjustmentScrews == adjustmentScrews)
        return;

    m_adjustmentScrews = adjustmentScrews;
    emit adjustmentScrewsChanged();
}

void QKlipperPrintBed::setAdjustmentScrew(QString name, QKlipperAdjustmentScrew *adjustmentScrew)
{
    if(m_adjustmentScrews.contains(name))
    {
        if(m_adjustmentScrews[name] == adjustmentScrew)
            return;

        QKlipperAdjustmentScrew *screw = m_adjustmentScrews[name];
        m_adjustmentScrews.remove(name);

        screw->deleteLater();
        screw = nullptr;
    }

    m_adjustmentScrews[name] = adjustmentScrew;
    emit adjustmentScrewsChanged();
}

bool QKlipperPrintBed::adjustmentScrewsError() const
{
    return m_adjustmentScrewsError;
}

void QKlipperPrintBed::setAdjustmentScrewsError(bool adjustmentScrewsError)
{
    if (m_adjustmentScrewsError == adjustmentScrewsError)
        return;
    m_adjustmentScrewsError = adjustmentScrewsError;
    emit adjustmentScrewsErrorChanged();
}

bool QKlipperPrintBed::hasAdjustmentScrewResult() const
{
    return m_hasAdjustmentScrewResult;
}

void QKlipperPrintBed::setHasAdjustmentScrewResult(bool hasAdjustmentScrewResult)
{
    if (m_hasAdjustmentScrewResult == hasAdjustmentScrewResult)
        return;
    m_hasAdjustmentScrewResult = hasAdjustmentScrewResult;
    emit hasAdjustmentScrewResultChanged();
}

bool QKlipperPrintBed::hasBedMeshResult() const
{
    return m_hasBedMeshResult;
}

void QKlipperPrintBed::setHasBedMeshResult(bool hasBedMeshResult)
{
    if (m_hasBedMeshResult == hasBedMeshResult)
        return;
    m_hasBedMeshResult = hasBedMeshResult;
    emit hasBedMeshResultChanged();
}

bool QKlipperPrintBed::isAdjustmentScrewsCalibrating() const
{
    return m_adjustmentScrewsCalibrating;
}

void QKlipperPrintBed::setAdjustmentScrewsCalibrating(bool adjustmentScrewsCalibrating)
{
    if (m_adjustmentScrewsCalibrating == adjustmentScrewsCalibrating)
        return;
    m_adjustmentScrewsCalibrating = adjustmentScrewsCalibrating;
    emit adjustmentScrewsCalibratingChanged();
}

bool QKlipperPrintBed::isBedMeshCalibrating() const
{
    return m_bedMeshCalibrating;
}

void QKlipperPrintBed::setBedMeshCalibrating(bool bedMeshCalibrating)
{
    if (m_bedMeshCalibrating == bedMeshCalibrating)
        return;

    m_bedMeshCalibrating = bedMeshCalibrating;
    emit bedMeshCalibratingChanged();
}

qreal QKlipperPrintBed::adjustmentScrewsMaxDeviation() const
{
    return m_adjustmentScrewsMaxDeviation;
}

void QKlipperPrintBed::setAdjustmentScrewsMaxDeviation(qreal adjustmentScrewsMaxDeviation)
{
    if (qFuzzyCompare(m_adjustmentScrewsMaxDeviation, adjustmentScrewsMaxDeviation))
        return;
    m_adjustmentScrewsMaxDeviation = adjustmentScrewsMaxDeviation;
    emit adjustmentScrewsMaxDeviationChanged();
}

bool QKlipperPrintBed::tiltAdjustError() const
{
    return m_tiltAdjustError;
}

void QKlipperPrintBed::setTargetTemp(qreal targetTemp)
{
    if(!m_console)
    {
        qWarning() << "Parental structure incorrect. No console for QKlipperHeater " + name();
        return;
    }

    //M140 sets bed temp without waiting
    QString gcode = QString("M140 S") + QString::number(targetTemp);

    //send the G-Code
    m_console->printerGcodeScript(gcode);
}

void QKlipperPrintBed::setTiltAdjustError(bool tiltAdjustError)
{
    if (m_tiltAdjustError == tiltAdjustError)
        return;
    m_tiltAdjustError = tiltAdjustError;
    emit tiltAdjustErrorChanged();
}

QKlipperPrintBed::Type QKlipperPrintBed::type() const
{
    return m_type;
}

void QKlipperPrintBed::setType(Type type)
{
    if (m_type == type)
        return;
    m_type = type;
    emit typeChanged();
}
