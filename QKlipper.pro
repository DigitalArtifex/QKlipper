QT = core network websockets

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        QKlipperConsole/qklipperconsole.cpp \
        QKlipperConsole/qklipperconsoleerror.cpp \
        QKlipperFile/qklipperfile.cpp \
        QKlipperInstance/qklipperinstance.cpp \
        QKlipperMessage/qklippermessage.cpp \
        QKlipperPrinter/qklipperadjustmentscrew.cpp \
        QKlipperPrinter/qklipperchamber.cpp \
        QKlipperPrinter/qklipperextruder.cpp \
        QKlipperPrinter/qklipperfan.cpp \
        QKlipperPrinter/qklippergcodemove.cpp \
        QKlipperPrinter/qklippermcu.cpp \
        QKlipperPrinter/qklipperposition.cpp \
        QKlipperPrinter/qklipperprintbed.cpp \
        QKlipperPrinter/qklipperprintbedmesh.cpp \
        QKlipperPrinter/qklipperprinter.cpp \
        QKlipperPrinter/qklipperprobedata.cpp \
        QKlipperPrinter/qklippersafezhome.cpp \
        QKlipperPrinter/qklippersteppermotor.cpp \
        QKlipperPrinter/qklippertoolhead.cpp \
        QKlipperServer/qklipperaccessdetails.cpp \
        QKlipperServer/qklipperannouncement.cpp \
        QKlipperServer/qklippergcodestore.cpp \
        QKlipperServer/qklippergcodestorevalue.cpp \
        QKlipperServer/qklipperjobqueue.cpp \
        QKlipperServer/qklipperprintjob.cpp \
        QKlipperServer/qklipperserver.cpp \
        QKlipperServer/qklipperuser.cpp \
        QKlipperSystem/qklippercanbus.cpp \
        QKlipperSystem/qklippercanbusdevice.cpp \
        QKlipperSystem/qklippercpuinfo.cpp \
        QKlipperSystem/qklipperdistributioninfo.cpp \
        QKlipperSystem/qklipperlibcameradevice.cpp \
        QKlipperSystem/qklippermemorystats.cpp \
        QKlipperSystem/qklippermoonrakerstatsentry.cpp \
        QKlipperSystem/qklippernetworkinterface.cpp \
        QKlipperSystem/qklippernetworkstatsentry.cpp \
        QKlipperSystem/qklippersdinfo.cpp \
        QKlipperSystem/qklipperserialperipheral.cpp \
        QKlipperSystem/qklipperservicestate.cpp \
        QKlipperSystem/qklippersystem.cpp \
        QKlipperSystem/qklipperthrottlestate.cpp \
        QKlipperSystem/qklipperupdatecommit.cpp \
        QKlipperSystem/qklipperupdatepackage.cpp \
        QKlipperSystem/qklipperupdatestate.cpp \
        QKlipperSystem/qklipperusbperipheral.cpp \
        QKlipperSystem/qklipperv412device.cpp \
        QKlipperSystem/qklipperv412mode.cpp \
        QKlipperSystem/qklippervirtualizationstate.cpp \
        QKlipperSystem/qklippervirtualsdcard.cpp \
        QKlipperSystem/qklipperwebcam.cpp \
        main.cpp

HEADERS += \
    QKlipperConsole/qklipperconsole.h \
    QKlipperConsole/qklipperconsoleerror.h \
    QKlipperFile/qklipperfile.h \
    QKlipperFile/qklippermetadata.h \
    QKlipperFile/qklipperthumbnail.h \
    QKlipperInstance/qklipperinstance.h \
    QKlipperMessage/qklippermessage.h \
    QKlipperPrinter/qklipperadjustmentscrew.h \
    QKlipperPrinter/qklipperchamber.h \
    QKlipperPrinter/qklipperendstopstatus.h \
    QKlipperPrinter/qklipperextruder.h \
    QKlipperPrinter/qklipperfan.h \
    QKlipperPrinter/qklippergcodemove.h \
    QKlipperPrinter/qklippermcu.h \
    QKlipperPrinter/qklipperposition.h \
    QKlipperPrinter/qklipperprintbed.h \
    QKlipperPrinter/qklipperprintbedmesh.h \
    QKlipperPrinter/qklipperprinter.h \
    QKlipperPrinter/qklipperprobedata.h \
    QKlipperPrinter/qklippersafezhome.h \
    QKlipperPrinter/qklippersteppermotor.h \
    QKlipperPrinter/qklippertemperaturestore.h \
    QKlipperPrinter/qklippertoolhead.h \
    QKlipperServer/qklipperaccessdetails.h \
    QKlipperServer/qklipperannouncement.h \
    QKlipperServer/qklipperclientidentifier.h \
    QKlipperServer/qklippergcodemacro.h \
    QKlipperServer/qklippergcodestore.h \
    QKlipperServer/qklippergcodestorevalue.h \
    QKlipperServer/qklipperjobqueue.h \
    QKlipperServer/qklipperprintjob.h \
    QKlipperServer/qklipperserver.h \
    QKlipperServer/qklipperuser.h \
    QKlipperSystem/qklippercanbus.h \
    QKlipperSystem/qklippercanbusdevice.h \
    QKlipperSystem/qklippercanbusinterface.h \
    QKlipperSystem/qklippercpuinfo.h \
    QKlipperSystem/qklipperdistributioninfo.h \
    QKlipperSystem/qklipperlibcameradevice.h \
    QKlipperSystem/qklipperlibcameramode.h \
    QKlipperSystem/qklippermemorystats.h \
    QKlipperSystem/qklippermoonrakerstatsentry.h \
    QKlipperSystem/qklippernetworkaddress.h \
    QKlipperSystem/qklippernetworkinterface.h \
    QKlipperSystem/qklippernetworkstatsentry.h \
    QKlipperSystem/qklippersdinfo.h \
    QKlipperSystem/qklipperserialperipheral.h \
    QKlipperSystem/qklipperservicestate.h \
    QKlipperSystem/qklippersystem.h \
    QKlipperSystem/qklipperthrottlestate.h \
    QKlipperSystem/qklipperupdatecommit.h \
    QKlipperSystem/qklipperupdatepackage.h \
    QKlipperSystem/qklipperupdatestate.h \
    QKlipperSystem/qklipperusbperipheral.h \
    QKlipperSystem/qklipperv412device.h \
    QKlipperSystem/qklipperv412mode.h \
    QKlipperSystem/qklippervirtualizationstate.h \
    QKlipperSystem/qklippervirtualsdcard.h \
    QKlipperSystem/qklipperwebcam.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

*-g++* {
    QMAKE_CXXFLAGS += -fpermissive
}
