QT += core network websockets quick

NAME = QKlipper
CONFIG += c++17 metatypes shared create_prl
TARGET      = $$qtLibraryTarget(qklipper)
TEMPLATE = lib
DEFINES += DALIB
VERSION = 1.0.1

unix {
    LIBEXTENSION = .so
    STATICLIBEXTENSION = .a
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    QKlipper/QKlipperConsole/qklippercommand.cpp \
    QKlipper/QKlipperConsole/qklipperconsole.cpp \
    QKlipper/QKlipperFile/qklipperfile.cpp \
    QKlipper/QKlipperInstance/qklipperinstance.cpp \
    QKlipper/QKlipperMessage/qklippermessage.cpp \
    QKlipper/QKlipperPrinter/qklipperadjustmentscrew.cpp \
    QKlipper/QKlipperPrinter/qklipperchamber.cpp \
    QKlipper/QKlipperPrinter/qklipperextruder.cpp \
    QKlipper/QKlipperPrinter/qklipperfan.cpp \
    QKlipper/QKlipperPrinter/qklippergcodemove.cpp \
    QKlipper/QKlipperPrinter/qklippermcu.cpp \
    QKlipper/QKlipperPrinter/qklipperprintbed.cpp \
    QKlipper/QKlipperPrinter/qklipperprintbedmesh.cpp \
    QKlipper/QKlipperPrinter/qklipperprinter.cpp \
    QKlipper/QKlipperPrinter/qklippersteppermotor.cpp \
    QKlipper/QKlipperPrinter/qklippertoolhead.cpp \
    QKlipper/QKlipperServer/qklippergcodestore.cpp \
    QKlipper/QKlipperServer/qklipperjobqueue.cpp \
    QKlipper/QKlipperServer/qklipperprintjob.cpp \
    QKlipper/QKlipperServer/qklipperserver.cpp \
    QKlipper/QKlipperSystem/qklippercanbus.cpp \
    QKlipper/QKlipperSystem/qklipperledstrip.cpp \
    QKlipper/QKlipperSystem/qklipperlibcameradevice.cpp \
    QKlipper/QKlipperSystem/qklippermemorystats.cpp \
    QKlipper/QKlipperSystem/qklipperpowerdevice.cpp \
    QKlipper/QKlipperSystem/qklippersensor.cpp \
    QKlipper/QKlipperSystem/qklippersensordata.cpp \
    QKlipper/QKlipperSystem/qklipperserialperipheral.cpp \
    QKlipper/QKlipperSystem/qklipperservice.cpp \
    QKlipper/QKlipperSystem/qklippersystem.cpp \
    QKlipper/QKlipperSystem/qklipperthrottlestate.cpp \
    QKlipper/QKlipperSystem/qklipperupdatemanager.cpp \
    QKlipper/QKlipperSystem/qklippervirtualizationstate.cpp \
    QKlipper/QKlipperSystem/qklippervirtualsdcard.cpp \
    main.cpp

HEADERS += \
    QKlipper/QKlipperConsole/qklippercommand.h \
    QKlipper/QKlipperConsole/qklipperconsole.h \
    QKlipper/QKlipperFile/qklipperfile.h \
    QKlipper/QKlipperFile/qklippermetadata.h \
    QKlipper/QKlipperFile/qklipperthumbnail.h \
    QKlipper/QKlipperInstance/qklipperinstance.h \
    QKlipper/QKlipperMessage/qklippermessage.h \
    QKlipper/QKlipperPrinter/qklipperadjustmentscrew.h \
    QKlipper/QKlipperPrinter/qklipperchamber.h \
    QKlipper/QKlipperPrinter/qklipperendstopstatus.h \
    QKlipper/QKlipperPrinter/qklipperextruder.h \
    QKlipper/QKlipperPrinter/qklipperfan.h \
    QKlipper/QKlipperPrinter/qklippergcodemove.h \
    QKlipper/QKlipperPrinter/qklippermcu.h \
    QKlipper/QKlipperPrinter/qklipperposition.h \
    QKlipper/QKlipperPrinter/qklipperprintbed.h \
    QKlipper/QKlipperPrinter/qklipperprintbedmesh.h \
    QKlipper/QKlipperPrinter/qklipperprinter.h \
    QKlipper/QKlipperPrinter/qklipperprobedata.h \
    QKlipper/QKlipperPrinter/qklippersafezhome.h \
    QKlipper/QKlipperPrinter/qklippersteppermotor.h \
    QKlipper/QKlipperPrinter/qklippertemperaturestore.h \
    QKlipper/QKlipperPrinter/qklippertoolhead.h \
    QKlipper/QKlipperServer/qklipperaccessdetails.h \
    QKlipper/QKlipperServer/qklipperannouncement.h \
    QKlipper/QKlipperServer/qklipperclientidentifier.h \
    QKlipper/QKlipperServer/qklippergcodemacro.h \
    QKlipper/QKlipperServer/qklippergcodestore.h \
    QKlipper/QKlipperServer/qklippergcodestorevalue.h \
    QKlipper/QKlipperServer/qklipperjobqueue.h \
    QKlipper/QKlipperServer/qklipperprintjob.h \
    QKlipper/QKlipperServer/qklipperserver.h \
    QKlipper/QKlipperServer/qklipperuser.h \
    QKlipper/QKlipperSystem/qklippercanbus.h \
    QKlipper/QKlipperSystem/qklippercanbusdevice.h \
    QKlipper/QKlipperSystem/qklippercanbusinterface.h \
    QKlipper/QKlipperSystem/qklippercpuinfo.h \
    QKlipper/QKlipperSystem/qklipperdistributioninfo.h \
    QKlipper/QKlipperSystem/qklipperledstrip.h \
    QKlipper/QKlipperSystem/qklipperlibcameradevice.h \
    QKlipper/QKlipperSystem/qklipperlibcameramode.h \
    QKlipper/QKlipperSystem/qklippermemorystats.h \
    QKlipper/QKlipperSystem/qklippermoonrakerstatsentry.h \
    QKlipper/QKlipperSystem/qklippernetworkaddress.h \
    QKlipper/QKlipperSystem/qklippernetworkinterface.h \
    QKlipper/QKlipperSystem/qklippernetworkstatsentry.h \
    QKlipper/QKlipperSystem/qklipperpowerdevice.h \
    QKlipper/QKlipperSystem/qklippersdinfo.h \
    QKlipper/QKlipperSystem/qklippersensor.h \
    QKlipper/QKlipperSystem/qklippersensordata.h \
    QKlipper/QKlipperSystem/qklipperserialperipheral.h \
    QKlipper/QKlipperSystem/qklipperservice.h \
    QKlipper/QKlipperSystem/qklippersystem.h \
    QKlipper/QKlipperSystem/qklipperthrottlestate.h \
    QKlipper/QKlipperSystem/qklipperupdatecommit.h \
    QKlipper/QKlipperSystem/qklipperupdatemanager.h \
    QKlipper/QKlipperSystem/qklipperupdatepackage.h \
    QKlipper/QKlipperSystem/qklipperusbperipheral.h \
    QKlipper/QKlipperSystem/qklipperv412device.h \
    QKlipper/QKlipperSystem/qklipperv412mode.h \
    QKlipper/QKlipperSystem/qklippervirtualizationstate.h \
    QKlipper/QKlipperSystem/qklippervirtualsdcard.h \
    QKlipper/QKlipperSystem/qklipperwebcam.h \
    QKlipper/qklipper.h \
    QKlipper/qklippererror.h \
    dalib_global.h

GCC_VERSION = $$QMAKE_CXX.QMAKE_GCC_MAJOR_VERSION$$PERIOD$$QMAKE_CXX.QMAKE_GCC_MINOR_VERSION$$PERIOD$$QMAKE_CXX.QMAKE_GCC_PATCH_VERSION

#create the module file information
modulefile = '{ $$escape_expand(\\n\\t)\
"name": "QKlipper", $$escape_expand(\\n\\t)\
"repository": "qtbase", $$escape_expand(\\n\\t)\
"version": "$$QT_VERSION", $$escape_expand(\\n\\t)\
"plugin_types": ["generic"], $$escape_expand(\\n\\t)\
"built_with": { $$escape_expand(\\n\\t\\t)\
"compiler_id": "GNU", $$escape_expand(\\n\\t\\t)\
"compiler_target": "", $$escape_expand(\\n\\t\\t)\
"compiler_version": "$$GCC_VERSION", $$escape_expand(\\n\\t\\t)\
"cross_compiled": false, $$escape_expand(\\n\\t\\t)\
"target_system": "$$QMAKE_TARGET.platform", $$escape_expand(\\n\\t\\t)\
"architecture": "$$QMAKE_TARGET.arch" $$escape_expand(\\n\\t)\
}$$escape_expand(\\n)}'

write_file($$OUT_PWD/QKlipper.json, modulefile)

headerfiles.files = $$HEADERS
headerfiles.path = $$[QT_INSTALL_HEADERS]/QKlipper/
INSTALLS    += headerfiles

consoleheaders.files = \
    QKlipper/QKlipperConsole/qklippercommand.h \
    QKlipper/QKlipperConsole/qklipperconsole.h

consoleheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperConsole
INSTALLS    += consoleheaders

fileheaders.files =  \
    QKlipper/QKlipperFile/qklipperfile.h \
    QKlipper/QKlipperFile/qklippermetadata.h \
    QKlipper/QKlipperFile/qklipperthumbnail.h

fileheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperFile
INSTALLS    += fileheaders

instanceheaders.files =  \
    QKlipper/QKlipperInstance/qklipperinstance.h

instanceheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperInstance
INSTALLS    += instanceheaders

messageheaders.files =  \
    QKlipper/QKlipperMessage/qklippermessage.h
messageheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperMessage
INSTALLS    += messageheaders

printerheaders.files =  \
    QKlipper/QKlipperPrinter/qklipperadjustmentscrew.h \
    QKlipper/QKlipperPrinter/qklipperchamber.h \
    QKlipper/QKlipperPrinter/qklipperendstopstatus.h \
    QKlipper/QKlipperPrinter/qklipperextruder.h \
    QKlipper/QKlipperPrinter/qklipperfan.h \
    QKlipper/QKlipperPrinter/qklippergcodemove.h \
    QKlipper/QKlipperPrinter/qklippermcu.h \
    QKlipper/QKlipperPrinter/qklipperposition.h \
    QKlipper/QKlipperPrinter/qklipperprintbed.h \
    QKlipper/QKlipperPrinter/qklipperprintbedmesh.h \
    QKlipper/QKlipperPrinter/qklipperprinter.h \
    QKlipper/QKlipperPrinter/qklipperprobedata.h \
    QKlipper/QKlipperPrinter/qklippersafezhome.h \
    QKlipper/QKlipperPrinter/qklippersteppermotor.h \
    QKlipper/QKlipperPrinter/qklippertemperaturestore.h \
    QKlipper/QKlipperPrinter/qklippertoolhead.h
printerheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperPrinter
INSTALLS    += printerheaders

systemheaders.files = \
    QKlipper/QKlipperSystem/qklippercanbus.h \
    QKlipper/QKlipperSystem/qklippercanbusdevice.h \
    QKlipper/QKlipperSystem/qklippercanbusinterface.h \
    QKlipper/QKlipperSystem/qklippercpuinfo.h \
    QKlipper/QKlipperSystem/qklipperdistributioninfo.h \
    QKlipper/QKlipperSystem/qklipperledstrip.h \
    QKlipper/QKlipperSystem/qklipperlibcameradevice.h \
    QKlipper/QKlipperSystem/qklipperlibcameramode.h \
    QKlipper/QKlipperSystem/qklippermemorystats.h \
    QKlipper/QKlipperSystem/qklippermoonrakerstatsentry.h \
    QKlipper/QKlipperSystem/qklippernetworkaddress.h \
    QKlipper/QKlipperSystem/qklippernetworkinterface.h \
    QKlipper/QKlipperSystem/qklippernetworkstatsentry.h \
    QKlipper/QKlipperSystem/qklipperpowerdevice.h \
    QKlipper/QKlipperSystem/qklippersdinfo.h \
    QKlipper/QKlipperSystem/qklippersensor.h \
    QKlipper/QKlipperSystem/qklippersensordata.h \
    QKlipper/QKlipperSystem/qklipperserialperipheral.h \
    QKlipper/QKlipperSystem/qklipperservice.h \
    QKlipper/QKlipperSystem/qklippersystem.h \
    QKlipper/QKlipperSystem/qklipperthrottlestate.h \
    QKlipper/QKlipperSystem/qklipperupdatecommit.h \
    QKlipper/QKlipperSystem/qklipperupdatemanager.h \
    QKlipper/QKlipperSystem/qklipperupdatepackage.h \
    QKlipper/QKlipperSystem/qklipperusbperipheral.h \
    QKlipper/QKlipperSystem/qklipperv412device.h \
    QKlipper/QKlipperSystem/qklipperv412mode.h \
    QKlipper/QKlipperSystem/qklippervirtualizationstate.h \
    QKlipper/QKlipperSystem/qklippervirtualsdcard.h \
    QKlipper/QKlipperSystem/qklipperwebcam.h

systemheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperSystem
INSTALLS    += systemheaders

serverheaders.files =  \
    QKlipper/QKlipperServer/qklipperaccessdetails.h \
    QKlipper/QKlipperServer/qklipperannouncement.h \
    QKlipper/QKlipperServer/qklipperclientidentifier.h \
    QKlipper/QKlipperServer/qklippergcodemacro.h \
    QKlipper/QKlipperServer/qklippergcodestore.h \
    QKlipper/QKlipperServer/qklippergcodestorevalue.h \
    QKlipper/QKlipperServer/qklipperjobqueue.h \
    QKlipper/QKlipperServer/qklipperprintjob.h \
    QKlipper/QKlipperServer/qklipperserver.h \
    QKlipper/QKlipperServer/qklipperuser.h

serverheaders.path = $$[QT_INSTALL_HEADERS]/QKlipper/QKlipperServer
INSTALLS    += serverheaders

headerfiles.files =  \
QKlipper/qklipper.h \
QKlipper/qklippererror.h \
dalib_global.h
headerfiles.path = $$[QT_INSTALL_HEADERS]/QKlipper/
INSTALLS    += headerfiles

module.files = $$OUT_PWD/QKlipper.json
module.path = $$[QT_HOST_PREFIX]/modules
INSTALLS += module

prl.files = $$OUT_PWD/lib$$TARGET$$PRLEXTENSION
prl.path = $$[QT_HOST_LIBS]/
INSTALLS += prl

staticlib.files = $$OUT_PWD/lib$$TARGET$$STATICLIBEXTENSION
staticlib.path = $$[QT_HOST_LIBS]/
INSTALLS += staticlib

target.path = $$[QT_HOST_LIBS]/
INSTALLS += target

#this will be removed in the final release once I fix some of the operators
# *-g++* {
#     QMAKE_CXXFLAGS += -fpermissive
# }

RESOURCES += \
    data.qrc
