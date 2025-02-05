#include <QKlipper/QKlipperConsole/qklipperconsole.h>
#include "qeventloop.h"
#include "qthread.h"

QKlipperConsole::QKlipperConsole(QObject *parent)
    : QObject{parent}
{
    generateParserMap();
    resetStartupSequence();
}

QKlipperConsole::~QKlipperConsole()
{
    for(auto iterator = m_messageMap.begin(); iterator != m_messageMap.end();)
        iterator = m_messageMap.erase(iterator);

    if(m_rpcUpdateSocket)
    {
        m_rpcUpdateSocket->close();

        delete m_rpcUpdateSocket;
    }

    m_startupSequence.clear();
    m_parserMap.clear();
}

bool QKlipperConsole::connect()
{
    if(m_rpcUpdateSocket && m_rpcUpdateSocket->isOpen())
        return true;

    setupNetworkAccessManager();

    addConnectionState(Connecting | Startup);

    m_printer->setStatus(QKlipperPrinter::Connecting);

    if(m_server->connectionType() == QKlipperServer::Local)
    {
        if(!m_rpcUpdateSocket)
            m_rpcUpdateSocket = (QAbstractSocket*)(new QLocalSocket(this));

        ((QLocalSocket*)m_rpcUpdateSocket)->setServerName(m_server->websocketAddress());
        ((QLocalSocket*)m_rpcUpdateSocket)->connectToServer();

        if(!((QLocalSocket*)m_rpcUpdateSocket)->waitForConnected(5000))
        {
            qDebug() << QString("Failed to connect to moonraker");

            QKlipperError error;
            error.setErrorString(QString("Failed to connect to moonraker"));
            error.setType(QKlipperError::Socket);
            error.setErrorTitle("Socket Error");
            error.setOrigin("Console Connect");

            emit errorOccured(error);

            //sendError("Could not connect to local socket");
            return false;
        }

        addConnectionState(MoonrakerConnected);
        startConnectionTimer();

        QObject::connect(m_rpcUpdateSocket, SIGNAL(readyRead()), this, SLOT(rpcUpdateSocketDataReady()));
    }
    else
    {
        if(!m_rpcUpdateSocket)
            m_rpcUpdateSocket = (QAbstractSocket*)(new QWebSocket());

        m_server->setConnectionType(QKlipperServer::Remote);

        QWebSocket *socket = qobject_cast<QWebSocket*>(m_rpcUpdateSocket);

        QEventLoop loop;
        bool errorOccurred = false;
        QAbstractSocket::SocketError socketError;

        QObject::connect(m_rpcUpdateSocket, SIGNAL(connected()), &loop, SLOT(quit()));
        QObject::connect
        (
            m_rpcUpdateSocket,
            &QAbstractSocket::disconnected,
            this,
            [&errorOccurred, &loop]() {
                errorOccurred = true;
                loop.quit();
            }
        );

        QObject::connect
        (
            m_rpcUpdateSocket,
            &QAbstractSocket::errorOccurred,
            this,
            [&socketError, &errorOccurred, &loop](QAbstractSocket::SocketError error) {
                errorOccurred = true;
                socketError = error;
                loop.quit();
            }
        );

        QObject::connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(rpcUpdateSocketDataReceived(QString)));

        //Timeout for websocket -_-
        QTimer *timeout = new QTimer(this);
        timeout->setInterval(10000);
        timeout->setSingleShot(true);

        QObject::connect
        (
            timeout,
            &QTimer::timeout,
            this,
            [&socketError, &errorOccurred, &loop]() {
                errorOccurred = true;
                socketError = QAbstractSocket::SocketTimeoutError;
                loop.quit();
            }
        );

        qDebug() << "Connecting to " << m_server->websocketAddress();
        socket->open(m_server->websocketAddress());
        timeout->start();
        loop.exec();

        timeout->deleteLater();

        if(socket->state() != QAbstractSocket::ConnectedState)
            errorOccurred = true;

        if(errorOccurred)
        {
            QKlipperError error;
            error.setErrorString(QString("Failed to connect to moonraker"));
            error.setType(QKlipperError::Socket);
            error.setErrorTitle("Socket Error " + QString::number(socketError));
            error.setOrigin("Console Connect");

            setConnectionState(Idle);

            emit errorOccured(error);
            return false;
        }

        addConnectionState(MoonrakerConnected);
        startConnectionTimer();
    }

    if(m_startupSequence.count())
    {
        qDebug() << QString("Beginning Startup Sequence");
        StartupFunction function = m_startupSequence.dequeue();
        (this->*function)();
    }
    else
        setConnectionState(Syncronized);

    return true;
}

void QKlipperConsole::disconnect()
{
    if(m_rpcUpdateSocket)
    {
        if(m_server->connectionType() == QKlipperServer::Local)
        {
            QLocalSocket *socket = qobject_cast<QLocalSocket*>(m_rpcUpdateSocket);
            socket->close();
        }
        else if(m_server->connectionType() == QKlipperServer::Remote)
        {
            QWebSocket *socket = qobject_cast<QWebSocket*>(m_rpcUpdateSocket);
            socket->close();
        }

        delete m_rpcUpdateSocket;
        m_rpcUpdateSocket = nullptr;
    }

    if(m_networkManager)
    {
        delete m_networkManager;
        m_networkManager = nullptr;
    }

    setConnectionState(Idle);
}

void QKlipperConsole::machineShutdown(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.shutdown");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);
    }
}

void QKlipperConsole::machineReboot(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.reboot");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);
    }
}

void QKlipperConsole::machineSystemInfo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.system_info");

    sendWebSocketMessageAsync(message);
}

bool QKlipperConsole::machineServiceRestart(QString service, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.restart");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineServiceStop(QString service, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.stop");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineServiceStart(QString service, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.start");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

void QKlipperConsole::machinePeripheralsUSB()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.usb");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machinePeripheralsSerial()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.serial");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machinePeripheralsVideo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.video");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machinePeripheralsCanbus(qint32 canBus)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("interface", canBus);
    message->setMethod("machine.peripherals.canbus");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineProcStats()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Machine Stats..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.proc_stats");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineUpdateStatus()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Update Status..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.status");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineUpdateRefresh()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.refresh");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

bool QKlipperConsole::machineUpdateFull(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.full");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateMoonraker(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.moonraker");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateKlipper(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.klipper");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateClient(QString client, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", client);
    message->setMethod("machine.update.client");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateSystem(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.system");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateRecover(QString name, bool hardRecover, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", name);
    message->setParam("hard", hardRecover);
    message->setMethod("machine.update.recover");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::machineUpdateRollback(QString name, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", name);
    message->setMethod("machine.update.rollback");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

void QKlipperConsole::printerInfo()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Printer Info..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.info");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::printerMCUInfo()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting MCU Info..");

    QString object = "mcu";
    printerObjectsQuery(object);
}

void QKlipperConsole::printerObjectsList()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Available Objects..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.objects.list");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::printerObjectsQuery(QString &object)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam(object, QVariant());
    message->setMethod("printer.objects.query");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::printerObjectsQuery(QStringList &objects)
{
    QKlipperMessage *message = new QKlipperMessage();

    foreach(QString object, objects)
        message->setParam(object,QVariant());

    message->setMethod("printer.objects.query");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::printerObjectsQuery()
{
    QKlipperMessage *message = new QKlipperMessage();

    foreach(QString object, m_server->availableObjects())
        message->setParam(object,QVariant());

    message->setMethod("printer.objects.query");

    m_messageMap.insert(message->id(), message);
    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::printerSubscribe()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Subscribing To Printer Objects..");

    QKlipperMessage *message = new QKlipperMessage();
    QJsonObject objects;

    foreach(QString object, m_server->availableObjects())
        objects[object] = QJsonValue();

    message->setParam("objects", objects);
    message->setMethod("printer.objects.subscribe");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

bool QKlipperConsole::printerEmergencyStop(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.emergency_stop");
    message->setProtocol(QKlipperMessage::HttpDeleteProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

void QKlipperConsole::printerQueryEndstops()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.query_endstops.status");

    sendWebSocketMessageAsync(message);
}

bool QKlipperConsole::printerPrintStart(QString file, QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filename", file);
    message->setMethod("printer.print.start");

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::printerPrintStart(QKlipperFile *file, QKlipperError *error)
{
    QString filename = QString("%1/%2").arg(file->path(), file->filename());
    return printerPrintStart(filename, error);
}

bool QKlipperConsole::printerPrintPause(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.print.pause");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::printerPrintResume(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.print.resume");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::printerPrintCancel(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.printer.cancel");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not cancel print");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::printerGcodeScript(QString gcode, QKlipperError *error, QKlipperMessage::Origin origin)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setOrigin(origin);
    message->setParam("script", gcode);
    message->setMethod("printer.gcode.script");
    message->setIsGcode(true);
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not send gcode script");
        processError(error);

        return false;
    }

    return true;
}

bool QKlipperConsole::restartKlipper(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.restart");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    bool errorCleanup = false;

    if(!error)
    {
        errorCleanup = true;
        error = new QKlipperError();
    }

    sendWebSocketMessage(message, error);

    if(error->type() != QKlipperError::NoError || message->response().toString() != "ok")
    {
        error->setErrorTitle("Could not restart klipper");
        processError(error);

        if(errorCleanup)
            delete error;

        return false;
    }

    if(errorCleanup)
        delete error;

    disconnect();
    setConnectionState(Restarting);
    resetStartupSequence();
    startConnectionTimer();

    return true;
}

bool QKlipperConsole::restartFirmware(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.firmware_restart");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    bool errorCleanup = false;

    if(!error)
    {
        errorCleanup = true;
        error = new QKlipperError();
    }

    sendWebSocketMessage(message, error);

    if(error->type() != QKlipperError::NoError || message->response().toString() != "ok")
    {
        error->setErrorTitle("Could not restart Firmware");
        processError(error);

        if(errorCleanup)
            delete error;

        return false;
    }

    if(errorCleanup)
        delete error;

    setConnectionState(Restarting);
    disconnect();
    resetStartupSequence();
    // connect();

    return true;
}

bool QKlipperConsole::serverRestart(QKlipperError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.restart");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessage(message, error);

    if(error && (error->type() != QKlipperError::NoError || message->response().toString() != "ok"))
    {
        error->setErrorTitle("Could not restart server");
        processError(error);

        return false;
    }

    return true;
}

void QKlipperConsole::serverInfo()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Server Info..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.info");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverConfig()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Server Configuration..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.config");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFileRoots()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting File Roots..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.files.roots");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFilesMetadata(QString fileName)
{
    if(fileName.startsWith("gcodes/", Qt::CaseInsensitive))
        fileName = fileName.remove(0, 7);

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filename", fileName);
    message->setParam("extended", true);
    message->setMethod("server.files.metadata");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFilesMetadata(QKlipperFile *file)
{
    //path is relative to gcodes
    QString path;

    if(file->path().length() > 0)
        path = file->path() + file->filename();
    else
        path = file->filename();

    if(path.startsWith("gcodes/", Qt::CaseInsensitive))
        path = path.remove(0, 7);

    qDebug() << path;

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filename", path);
    message->setParam("extended", true);
    message->setMethod("server.files.metadata");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFilesList(QString directory)
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting File Lists..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setParam("extended", true);
    message->setMethod("server.files.get_directory");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFileDelete(QString file)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", file);
    message->setParam("extended", true);
    message->setMethod("server.files.delete_file");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFileDelete(QKlipperFile *file)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", file->uri());
    message->setParam("extended", true);
    message->setMethod("server.files.delete_file");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFileMove(QString source, QString destination)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("source", source);
    message->setParam("dest", destination);
    message->setMethod("server.files.move");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverFileCopy(QString source, QString destination)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("source", source);
    message->setParam("dest", destination);
    message->setMethod("server.files.copy");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

QByteArray QKlipperConsole::serverFileDownload(QKlipperFile *file, QKlipperError *error)
{
    QByteArray fileData;

    if(m_server->connectionType() == QKlipperServer::Local)
    {
        QString rootLocation;

        if(file->fileType() == QKlipperFile::GCode)
            rootLocation = m_server->gcodesLocation();
        else if(file->fileType() == QKlipperFile::Config)
            rootLocation = m_server->configLocation();

        if(file->path().length() > 0)
            rootLocation += file->path();

        rootLocation = rootLocation + file->filename();

        QFile localFile(rootLocation);


        if(localFile.open(QFile::ReadOnly))
        {
            fileData = localFile.readAll();
            localFile.close();
        }
        else if(error)
        {
            error->setErrorString(localFile.errorString());
            error->setOrigin("Console - Server.Files.Download");
            error->setType(QKlipperError::File);
        }
    }

    else if(m_server->connectionType() == QKlipperServer::Remote)
    {
        QEventLoop loop;

        QString address = QString("%1/server/files/%2").arg(m_server->address(), file->uri());
        QNetworkRequest request(address);
        QNetworkReply *reply = m_networkManager->get(request);

        QObject::connect
        (
            reply,
            &QNetworkReply::finished,
            this,
            [&loop]()
            {
                loop.quit();
            }
        );

        loop.exec();

        if (reply->error())
        {
            qDebug() << "Error: " + reply->errorString() << reply->url() ;

            if(!error)
                error = new QKlipperError();

            error->setErrorString(reply->errorString());
            error->setType(QKlipperError::Socket);
            error->setOrigin("Server Files Download");
            error->setErrorTitle("Error Sending Websocket Command");

            processError(error);
        }
        else
        {
            if(error)
            {
                error->setErrorString("");
                error->setOrigin("");
                error->setType(QKlipperError::NoError);
            }

            fileData = reply->readAll();
        }

        reply->deleteLater();
    }

    return fileData;
}

QByteArray QKlipperConsole::serverFileDownload(QString file, QKlipperError *error)
{

    QByteArray fileData;

    if(m_server->connectionType() == QKlipperServer::Local)
    {
        // QString rootLocation;

        // if(file->fileType() == QKlipperFile::GCode)
        //     rootLocation = m_server->gcodesLocation();
        // else if(file->fileType() == QKlipperFile::Config)
        //     rootLocation = m_server->configLocation();

        // if(file->path().length() > 0)
        //     rootLocation += file->path();

        // rootLocation = rootLocation + file->filename();

        QFile localFile(file);


        if(localFile.open(QFile::ReadOnly))
        {
            fileData = localFile.readAll();
            localFile.close();
        }
        else if(error)
        {
            error->setErrorString(localFile.errorString());
            error->setOrigin("Console - Server.Files.Download");
            error->setType(QKlipperError::File);
        }
    }

    else if(m_server->connectionType() == QKlipperServer::Remote)
    {
        QEventLoop loop;

        QString address = QString("%1/server/files/%2").arg(m_server->address(), file);
        QNetworkRequest request(address);
        QNetworkReply *reply = m_networkManager->get(request);

        QObject::connect
        (
            reply,
            &QNetworkReply::finished,
            this,
            [&loop]()
            {
                loop.quit();
            }
        );

        loop.exec();

        if (reply->error())
        {
            qDebug() << "Error: " + reply->errorString() << reply->url() ;

            if(!error)
                error = new QKlipperError();

            error->setErrorString(reply->errorString());
            error->setType(QKlipperError::Socket);
            error->setOrigin("Server Files Download");
            error->setErrorTitle("Error Sending Websocket Command");

            processError(error);
        }
        else
        {
            if(error)
            {
                error->setErrorString("");
                error->setOrigin("");
                error->setType(QKlipperError::NoError);
            }

            fileData = reply->readAll();
        }

        reply->deleteLater();
    }

    return fileData;
}

bool QKlipperConsole::serverFileUpload(QString root, QString directory, QString name, QByteArray data, QKlipperError *error)
{
    if(m_server->connectionType() == QKlipperServer::Local)
    {
        QString rootLocation;
        QString localFileLocation;

        if(directory.endsWith(QDir::separator()))
            directory.removeLast();

        if(root == QString("gcodes"))
            rootLocation = m_server->gcodesLocation();
        else if(root == QString("config"))
            rootLocation = m_server->configLocation();

        if(directory.isEmpty())
            localFileLocation = rootLocation + QDir::separator() + name;
        else
            localFileLocation = rootLocation + QDir::separator() + directory + QDir::separator() + name;

        if(QFile::exists(localFileLocation))
            QFile::moveToTrash(localFileLocation);

        QFile localFile(localFileLocation);
        QString localFileData;

        if(localFile.open(QFile::WriteOnly))
        {
            localFile.write(data);
            localFile.close();

            if(error)
            {
                error->setErrorString("");
                error->setOrigin("");
                error->setType(QKlipperError::NoError);
            }

            return true;
        }
        else
        {
            if(error)
            {
                error->setErrorString(localFile.errorString());
                error->setOrigin("Console - Server.Files.Upload");
                error->setType(QKlipperError::File);
            }

            return false;
        }
    }
    else if(m_server->connectionType() == QKlipperServer::Remote)
    {
        QString address = QString("%1/server/files/upload").arg(m_server->address());

        QNetworkRequest request(address);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=FormBoundaryemap3PkuvKX0B3HH");

        QString contentDisposition;

        if(directory.length() > 0)
            contentDisposition = QString("form-data; root=\"%1\"; name=\"file\"; path=\"%2\"; filename=\"%3\"").arg(root, directory, name);
        else
            contentDisposition = QString("form-data; root=\"%1\"; name=\"file\"; filename=\"%2\"").arg(root, name);

        //create multipart form for file upload
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        multiPart->setBoundary("FormBoundaryemap3PkuvKX0B3HH"); //taken directly from Klipper docs

        // Add parts to the multipart message
        QHttpPart fileData;
        fileData.setHeader(QNetworkRequest::ContentDispositionHeader, contentDisposition);
        fileData.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
        fileData.setBody(data);

        //add file data part to multipart
        multiPart->append(fileData);

        QEventLoop loop;

        QNetworkReply *reply = m_networkManager->post(request, multiPart);
        multiPart->setParent(reply);

        QObject::connect
        (
            reply,
            &QNetworkReply::finished,
            this,
            [&loop]()
            {
                loop.quit();
            }
        );

        loop.exec();

        if (reply->error())
        {
            qDebug() << "Error: " + reply->errorString() << reply->url() ;

            if(!error)
                error = new QKlipperError();

            error->setErrorString(reply->errorString());
            error->setType(QKlipperError::Socket);
            error->setOrigin("Server Files Upload");
            error->setErrorTitle("Error Sending Websocket Command");

            processError(error);

            return false;
        }
        else
        {
            if(error)
            {
                error->setErrorString("");
                error->setOrigin("");
                error->setType(QKlipperError::NoError);
            }

            return true;
        }

        reply->deleteLater();
    }

    if(error)
    {
        error->setErrorString("Unknown Error");
        error->setOrigin("Console - Server.Files.Upload");
        error->setType(QKlipperError::Unspecified);
    }

    return false;
}

void QKlipperConsole::serverDirectoryPost(QString directory)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setMethod("server.files.directory");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverDirectoryDelete(QString directory, bool forced = false)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setParam("forced", forced);
    message->setMethod("server.files.directory");
    message->setProtocol(QKlipperMessage::HttpDeleteProtocol);

    m_messageMap.insert(message->id(), message);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverTemperatureStore()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("include_monitors", false);
    message->setMethod("server.temperature_store");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverGcodeStore()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("count", 100);
    message->setMethod("server.gcode_store");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverLogsRollover()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.logs.rollover");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverLogsRollover(QString &application)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("application", application);
    message->setMethod("server.logs.rollover");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverWebsocketId()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.websocket.id");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverWebcamList()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Webcams..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.webcams.list");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverWebcamCreate(QKlipperWebcam *webcam)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", webcam->name());
    message->setParam("location", webcam->location());
    message->setParam("service", webcam->service());
    message->setParam("enabled", webcam->enabled());
    message->setParam("icon", webcam->iconString());
    message->setParam("target_fps", webcam->targetFps());
    message->setParam("target_fps_idle", webcam->targetFpsIdle());
    message->setParam("stream_url", webcam->streamUrl());
    message->setParam("snapshot_url", webcam->snapshotUrl());
    message->setParam("flip_horizontal", webcam->flipHorizontal());
    message->setParam("flip_vertical", webcam->flipVertical());
    message->setParam("rotation", webcam->rotation());
    message->setParam("aspect_ratio", webcam->aspectRatio());
    message->setParam("source", webcam->source());
    message->setMethod("server.webcams.item");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverWebcamUpdate(QKlipperWebcam *webcam)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", webcam->name());
    message->setParam("location", webcam->location());
    message->setParam("service", webcam->service());
    message->setParam("enabled", webcam->enabled());
    message->setParam("icon", webcam->iconString());
    message->setParam("target_fps", webcam->targetFps());
    message->setParam("target_fps_idle", webcam->targetFpsIdle());
    message->setParam("stream_url", webcam->streamUrl());
    message->setParam("snapshot_url", webcam->snapshotUrl());
    message->setParam("flip_horizontal", webcam->flipHorizontal());
    message->setParam("flip_vertical", webcam->flipVertical());
    message->setParam("rotation", webcam->rotation());
    message->setParam("aspect_ratio", webcam->aspectRatio());
    message->setParam("source", webcam->source());
    message->setParam("uid", webcam->uid());
    message->setMethod("server.webcams.item");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverWebcamDelete(QKlipperWebcam *webcam)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("uid", webcam->uid());
    message->setMethod("server.webcams.item");
    message->setProtocol(QKlipperMessage::HttpDeleteProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverAnnouncementsList(bool includeDismissed)
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Announcements..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("include_dismissed", includeDismissed);
    message->setMethod("server.announcements.list");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverAnnouncementsUpdate()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.announcements.update");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverAnnouncementDismiss(QString entryId, qint64 waketime)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("entry_id", entryId);
    message->setParam("waketime", waketime);
    message->setMethod("server.announcements.dismiss");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueueStatus()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Job Queue..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.status");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueueStart()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.start");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueuePause()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.pause");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueueJump(QString id)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("job_id", id);
    message->setMethod("server.job_queue.jump");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueueAdd(QStringList filenames)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filenames", filenames.join(','));
    message->setMethod("server.job_queue.job");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::serverJobQueueDelete(QStringList ids)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("job_ids", ids.join(','));
    message->setMethod("server.job_queue.job");
    message->setProtocol(QKlipperMessage::HttpDeleteProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::clientIdentifier()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Sending Identifier..");

    QKlipperMessage *message = new QKlipperMessage();
    QKlipperClientIdentifier identifier = m_server->clientIdentifier();

    message->setParam("client_name", identifier.name);
    message->setParam("version", identifier.version);
    message->setParam("type", identifier.type);
    message->setParam("url", identifier.url);
    message->setMethod("server.connection.identify");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::databaseCreate(const QString &key, const QVariant &value)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("namespace", QCoreApplication::applicationName());
    message->setParam("key", key);
    message->setParam("value", value);
    message->setMethod("server.database.item");
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::databaseDelete(const QString &key)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("namespace", QCoreApplication::applicationName());
    message->setParam("key", key);
    message->setMethod("server.database.item");
    message->setProtocol(QKlipperMessage::HttpDeleteProtocol);

    sendWebSocketMessageAsync(message);
}

QVariant QKlipperConsole::databaseGetItem(const QString &key)
{
    QVariant value;

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("namespace", QCoreApplication::applicationName());
    message->setParam("key", key);
    message->setMethod("server.database.item");

    sendWebSocketMessage(message);

    if(message->response().isObject())
    {
        QJsonObject response = message->response().toObject();

        if(response.contains("value"))
            value = response["value"].toVariant();
    }

    return value;
}

void QKlipperConsole::machinePowerDeviceList()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Power Devices..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.device_power.devices");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machinePowerDeviceStatus(QStringList names)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.device_power.device");

    for(QString name : names)
        message->setParam(name, QVariant());

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machinePowerDeviceSetState(const QString &name, const QString &action)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.device_power.device");
    message->setParam("device", name);
    message->setParam("action", action);
    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineLedStripList()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting LED Strips..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.wled.strips");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineLedStrip(const QStringList &names)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.wled.status");

    for(const QString &name : names)
        message->setParam(name, QVariant());

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineLedStripOn(const QStringList &names)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.wled.on");

    for(const QString &name : names)
        message->setParam(name, QVariant());

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineLedStripOff(const QStringList &names)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.wled.off");

    for(const QString &name : names)
        message->setParam(name, QVariant());

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineSetLedStrip(QKlipperLedStrip *stripData)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.wled.strip");
    message->setParam("strip", stripData->m_name);
    message->setParam("preset", stripData->m_preset);
    message->setParam("brightness", stripData->m_brightness);
    message->setParam("intensity", stripData->m_intensity);
    message->setParam("speed", stripData->m_speed);

    if(stripData->isOn())
        message->setParam("action", "on");
    else
        message->setParam("action", "off");

    message->setProtocol(QKlipperMessage::HttpPostProtocol);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineSensorsList()
{
    if(hasConnectionState(Startup))
        setStartupSequenceText("Getting Machine Sensors..");

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.sensors.list");
    message->setParam("extended", true);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineSensorInfo(const QString &name)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.sensors.info");
    message->setParam("extended", true);
    message->setParam("sensor", name);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineSensorMeasurement(const QString &name)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.sensors.measurements");
    message->setParam("sensor", name);

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::machineSensorMeasurements()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.sensors.measurements");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::accessLogin(QString username, QString password)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("username", username);
    message->setParam("password", password);
    message->setMethod("access.login");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::accessLogout()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("access.logout");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::accessGetUser()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("access.get_user");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::accessCreateUser(QString username, QString password)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("username", username);
    message->setParam("password", password);
    message->setMethod("access.post_user");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::accessDeleteUser(QString username)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("username", username);
    message->setMethod("access.delete_user");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::accessUsersList()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("access.users.list");

    sendWebSocketMessageAsync(message);
}

void QKlipperConsole::accessUserPasswordReset(QString password, QString newPassword)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("new_password", newPassword);
    message->setParam("password", password);
    message->setMethod("access.user.password");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

void QKlipperConsole::rpcUpdateSocketDataReady()
{
    resetConnectionTimer();

    if(m_rpcUpdateSocket->isOpen())
    {
        QByteArray incoming = m_rpcUpdateSocket->readAll();
        m_rpcDataBuffer += incoming;
    }

    scanRpcUpdateBuffer();
}

void QKlipperConsole::rpcUpdateSocketDataReceived(QString data)
{
    resetConnectionTimer();

    //EOF is either not being sent or being filtered out by QWebSocket
    //We need to do the same thing without scanning for EOF
    QJsonParseError responseDocumentError;
    QJsonDocument responseDocument(QJsonDocument::fromJson(data.toUtf8(), &responseDocumentError));

    if(responseDocumentError.error != QJsonParseError::NoError)
    {
        qDebug() << "Invalid response document";
        //sendError("Invalid response from RPC socket");
        return;
    }

    //construct message object
    QKlipperMessage *message;

    QJsonObject responseObject = responseDocument.object();

    if(responseObject.contains("error"))
        qDebug() << "Error" << responseObject;

    int id = -1;

    if(responseObject.contains("id"))
        id = responseObject["id"].toInt();

    if(m_messageMap.contains(id))
    {
        message = m_messageMap[id];

        if(!responseObject.contains("method"))
            responseObject["method"] = message->method();
    }
    else
    {
        message = new QKlipperMessage();
        message->setId(id);
        message->setMethod(responseObject["method"].toString());
    }

    if(responseObject.contains("error"))
    {
        QKlipperError error;

        QString errorString(responseObject["error"].toString());
        QString errorTitle = errorString.right(errorString.lastIndexOf('\n'));
        errorString = errorString.left(errorString.lastIndexOf('\n'));

        error.setErrorString(errorString);
        error.setErrorTitle(errorTitle);

        message->setError(error);
    }

    if(responseObject["method"].toString().startsWith("notify")
        || responseObject["method"].toString().startsWith("sensors:"))
    {

        if(responseObject.contains("result"))
            responseObject = responseObject["result"].toObject();

        message->setResponse(responseObject);
        parseNotification(message);
    }
    else
    {
        if(responseObject.contains("result"))
            responseObject = responseObject["result"].toObject();

        message->setResponse(responseObject);
        parseResponse(message);
    }
}

void QKlipperConsole::scanRpcUpdateBuffer()
{
    if(m_rpcDataBuffer.contains((char)0x03))
    {
        int index = m_rpcDataBuffer.indexOf((char)0x03);
        QByteArray responseData = m_rpcDataBuffer.mid(0, index + 1);
        m_rpcDataBuffer.remove(0, index + 1);

        if(responseData.endsWith((char)0x03))
            responseData.remove(responseData.length() - 1, 1);

        QJsonParseError responseDocumentError;
        QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

        if(responseDocumentError.error != QJsonParseError::NoError)
        {
            //sendError("Invalid response from RPC socket");
            return;
        }

        //construct message object
        QKlipperMessage *message;

        QJsonObject responseObject = responseDocument.object();

        int id = -1;

        if(responseObject.contains("id"))
            id = responseObject["id"].toInt();

        if(m_messageMap.contains(id))
        {
            message = m_messageMap[id];

            if(!responseObject.contains("method"))
                responseObject["method"] = message->method();
        }
        else
        {
            message = new QKlipperMessage();
            message->setId(id);
            message->setMethod(responseObject["method"].toString());
        }

        if(responseObject.contains("error"))
        {
            QKlipperError error;

            QString errorString(responseObject["error"].toString());
            QString errorTitle = errorString.right(errorString.lastIndexOf('\n'));
            errorString = errorString.left(errorString.lastIndexOf('\n'));

            error.setErrorString(errorString);
            error.setErrorTitle(errorTitle);

            message->setError(error);
        }

        if(responseObject["method"].toString().startsWith("notify"))
        {
            if(responseObject.contains("params")
                && responseObject["params"].isArray()
                && responseObject["params"].toArray().count() > 0)
            {
                QJsonArray params = responseObject["params"].toArray();
                QString method = responseObject["method"].toString();

                //set the first object as the response object
                for(int i = 0; i < params.count(); i++)
                {
                    if(params[i].isObject())
                    {
                        responseObject = params[i].toObject();
                        responseObject["method"] = method;
                        break;
                    }
                }
            }

            if(responseObject.contains("result"))
                responseObject = responseObject["result"].toObject();

            message->setResponse(responseObject);
            parseNotification(message);
        }
        else
        {
            if(responseObject.contains("result"))
                responseObject = responseObject["result"].toObject();

            message->setResponse(responseObject);
            parseResponse(message);
        }
    }
}

void QKlipperConsole::deleteMessage(QKlipperMessage *message)
{
    if(message)
    {
        m_messageMap.remove(message->id());
        message->deleteLater();
    }
}

QKlipperConsole::ConnectionState QKlipperConsole::connectionState() const
{
    return m_connectionState;
}

bool QKlipperConsole::hasConnectionState(ConnectionState state)
{
    return ((m_connectionState & state) == state);
}

void QKlipperConsole::addConnectionState(ConnectionState state)
{
    setConnectionState(m_connectionState | state);

    if(hasConnectionState((ConnectionState)(KlipperConnected | MoonrakerConnected)) && !hasConnectionState(Startup) && !hasConnectionState(Syncronized))
    {
        addConnectionState(Syncronized);
    }
    if(hasConnectionState((ConnectionState)(Connecting | Syncronized)))
    {
        removeConnectionState(Connecting);
        addConnectionState(Connected);
    }
}

void QKlipperConsole::removeConnectionState(ConnectionState state)
{
    if((state & KlipperConnected) == KlipperConnected || (state & MoonrakerConnected) == MoonrakerConnected)
    {
        if(hasConnectionState(Syncronized))
            m_connectionState = m_connectionState & ~Syncronized;
    }

    setConnectionState((ConnectionState)(m_connectionState & ~state));
}

bool QKlipperConsole::isConnecting()
{
    return hasConnectionState(Connecting);
}

bool QKlipperConsole::isConnected()
{
    return hasConnectionState(Connected);
}

void QKlipperConsole::setConnectionState(ConnectionState connectionState)
{
    if (m_connectionState == connectionState)
        return;

    m_connectionState = connectionState;
    emit connectionStateChanged();

    if(m_connectionState == Idle || m_connectionState == Restarting)
    {
        m_printer->setStatus(QKlipperPrinter::Offline);
        resetConnectionState();
    }

    qDebug() << "Connection State: " << m_connectionState;
}

void QKlipperConsole::resetConnectionState()
{
    setConnectionState(Idle);
}

void QKlipperConsole::resetStartupSequence()
{
    m_startupSequence.clear();

    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::clientIdentifier);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::accessUsersList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineSystemInfo);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machinePowerDeviceList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineLedStripList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineSensorsList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineProcStats);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machinePeripheralsUSB);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machinePeripheralsSerial);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineUpdateStatus);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::serverConfig);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::serverFileRoots);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::serverWebcamList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::serverAnnouncementsList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerObjectsList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerInfo);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::serverInfo);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerMCUInfo);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerObjectsQuery);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerSubscribe);

    m_startupSequenceCount = m_startupSequence.count();
    setStartupSequenceProgress(0);
}

QString QKlipperConsole::startupSequenceText() const
{
    return m_startupSequenceText;
}

void QKlipperConsole::setStartupSequenceText(const QString &startupSequenceText)
{
    if (m_startupSequenceText == startupSequenceText)
        return;

    m_startupSequenceText = startupSequenceText;
    emit startupSequenceTextChanged();
}

void QKlipperConsole::resetStartupSequenceText()
{
    setStartupSequenceText({}); // TODO: Adapt to use your actual default value
}

void QKlipperConsole::generateParserMap()
{
    m_parserMap.clear();

    m_parserMap[QString("printer.info")] = (ParserFunction)&QKlipperConsole::printerInfoParser;
    m_parserMap[QString("printer.objects.list")] = (ParserFunction)&QKlipperConsole::printerObjectsListParser;
    m_parserMap[QString("printer.objects.subscribe")] = (ParserFunction)&QKlipperConsole::printerSubscribeParser;
    m_parserMap[QString("printer.objects.query")] = (ParserFunction)&QKlipperConsole::printerSubscribeParser;
    m_parserMap[QString("notify_status_update")] = (ParserFunction)&QKlipperConsole::printerSubscribeParser;
    m_parserMap[QString("printer.query_endstops.status")] = (ParserFunction)&QKlipperConsole::printerQueryEndstopsParser;

    m_parserMap[QString("server.connection.identify")] = (ParserFunction)&QKlipperConsole::clientIdentifierParser;
    m_parserMap[QString("server.info")] = (ParserFunction)&QKlipperConsole::serverInfoParser;
    m_parserMap[QString("server.files.post_directory")] = (ParserFunction)&QKlipperConsole::serverDirectoryPostParser;
    m_parserMap[QString("server.files.delete_directory")] = (ParserFunction)&QKlipperConsole::serverDirectoryDeleteParser;
    m_parserMap[QString("server.files.roots")] = (ParserFunction)&QKlipperConsole::serverFileRootsParser;
    m_parserMap[QString("server.files.get_directory")] = (ParserFunction)&QKlipperConsole::serverFilesListParser;
    m_parserMap[QString("server.temperature_store")] = (ParserFunction)&QKlipperConsole::serverTemperatureStoreParser;
    m_parserMap[QString("server.websocket.id")] = (ParserFunction)&QKlipperConsole::serverWebsocketIdParser;
    m_parserMap[QString("server.logs.rollover")] = (ParserFunction)&QKlipperConsole::serverLogsRolloverParser;
    m_parserMap[QString("server.files.list")] = (ParserFunction)&QKlipperConsole::serverFilesListParser;
    m_parserMap[QString("server.files.metadata")] = (ParserFunction)&QKlipperConsole::serverFilesMetadataParser;
    m_parserMap[QString("server.webcams.list")] = (ParserFunction)&QKlipperConsole::serverWebcamListParser;
    m_parserMap[QString("server.webcams.post_item")] = (ParserFunction)&QKlipperConsole::serverWebcamCreateParser;
    m_parserMap[QString("server.webcams.delete_item")] = (ParserFunction)&QKlipperConsole::serverWebcamDeleteParser;
    m_parserMap[QString("server.announcements.list")] = (ParserFunction)&QKlipperConsole::serverAnnouncementsListParser;
    m_parserMap[QString("server.announcements.update")] = (ParserFunction)&QKlipperConsole::serverAnnouncementsUpdateParser;

    //Job queue management
    m_parserMap[QString("server.job_queue.status")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;
    m_parserMap[QString("server.job_queue.post_job")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;
    m_parserMap[QString("server.job_queue.pause")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;
    m_parserMap[QString("server.job_queue.delete_job")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;
    m_parserMap[QString("server.job_queue.start")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;
    m_parserMap[QString("server.job_queue.jump")] = (ParserFunction)&QKlipperConsole::serverJobQueueStatusParser;

    m_parserMap[QString("machine.system_info")] = (ParserFunction)&QKlipperConsole::machineSystemInfoParser;
    m_parserMap[QString("machine.proc_stats")] = (ParserFunction)&QKlipperConsole::machineProcStatsParser;
    m_parserMap[QString("notify_proc_stat_update")] = (ParserFunction)&QKlipperConsole::machineProcStatsParser;

    m_parserMap[QString("machine.peripherals.usb")] = (ParserFunction)&QKlipperConsole::machinePeripheralsUSBParser;
    m_parserMap[QString("machine.peripherals.serial")] = (ParserFunction)&QKlipperConsole::machinePeripheralsSerialParser;
    m_parserMap[QString("machine.peripherals.video")] = (ParserFunction)&QKlipperConsole::machinePeripheralsVideoParser;
    m_parserMap[QString("machine.peripherals.canbus")] = (ParserFunction)&QKlipperConsole::machinePeripheralsCanbusParser;
    m_parserMap[QString("machine.device_power.devices")] = (ParserFunction)&QKlipperConsole::machinePowerDeviceListParser;
    m_parserMap[QString("machine.device_power.status")] = (ParserFunction)&QKlipperConsole::machinePowerDeviceListParser;
    m_parserMap[QString("machine.wled.strips")] = (ParserFunction)&QKlipperConsole::machineLedStripListParser;
    m_parserMap[QString("machine.wled.status")] = (ParserFunction)&QKlipperConsole::machineLedStripListParser;
    m_parserMap[QString("machine.wled.on")] = (ParserFunction)&QKlipperConsole::machineLedStripListParser;
    m_parserMap[QString("machine.wled.off")] = (ParserFunction)&QKlipperConsole::machineLedStripListParser;
    m_parserMap[QString("machine.wled.strip")] = (ParserFunction)&QKlipperConsole::machineLedStripListParser;
    m_parserMap[QString("server.sensors.list")] = (ParserFunction)&QKlipperConsole::machineSensorListParser;
    m_parserMap[QString("server.sensors.measurements")] = (ParserFunction)&QKlipperConsole::machineSensorMeasurementParser;
    m_parserMap[QString("server.sensors.info")] = (ParserFunction)&QKlipperConsole::machineSensorParser;

    m_parserMap[QString("machine.update.status")] = (ParserFunction)&QKlipperConsole::machineUpdateStatusParser;
    m_parserMap[QString("machine.update.refresh")] = (ParserFunction)&QKlipperConsole::machineUpdateStatusParser;

    m_parserMap[QString("access.login")] = (ParserFunction)&QKlipperConsole::accessLoginParser;
    m_parserMap[QString("access.logout")] = (ParserFunction)&QKlipperConsole::accessLogoutParser;
    m_parserMap[QString("access.get_user")] = (ParserFunction)&QKlipperConsole::accessGetUserParser;
    m_parserMap[QString("access.post_user")] = (ParserFunction)&QKlipperConsole::accessCreateUserParser;
    m_parserMap[QString("access.delete_user")] = (ParserFunction)&QKlipperConsole::accessDeleteUserParser;
    m_parserMap[QString("access.users.list")] = (ParserFunction)&QKlipperConsole::accessUsersListParser;
    m_parserMap[QString("access.user.password")] = (ParserFunction)&QKlipperConsole::accessUserPasswordResetParser;
}

void QKlipperConsole::setupNetworkAccessManager()
{
    if(!m_networkManager)
        m_networkManager = new QNetworkAccessManager(this);
}

void QKlipperConsole::resetConnectionTimer()
{
    stopConnectionTimer();
    startConnectionTimer();
}

void QKlipperConsole::startConnectionTimer()
{
    if(!m_rpcConnectionTimer)
    {
        m_rpcConnectionTimer = new QTimer(this);
        QObject::connect(m_rpcConnectionTimer, SIGNAL(timeout()), this, SLOT(onRpcConnectionTimeout()));
    }

    if(!m_rpcConnectionTimer->isActive())
    {
        m_rpcConnectionTimer->setInterval(m_rpcConnectionTimeoutValue);
        m_rpcConnectionTimer->setSingleShot(true);
        m_rpcConnectionTimer->start();
    }
}

void QKlipperConsole::stopConnectionTimer()
{
    if(m_rpcConnectionTimer && m_rpcConnectionTimer->isActive())
    {
        m_rpcConnectionTimer->stop();
    }
}

void QKlipperConsole::onRpcConnectionTimeout()
{
    disconnect();
    resetStartupSequence();
    setStartupSequenceText("Datastream Interrupted..");
    connect();
}

void QKlipperConsole::processError(QKlipperError *error)
{
    if(hasConnectionState(Startup))
    {
        disconnect();
        resetStartupSequence();
    }

    emit errorOccured(*error);
}

QKlipperServer *QKlipperConsole::server() const
{
    return m_server;
}

void QKlipperConsole::setServer(QKlipperServer *server)
{
    if(m_server == server)
        return;

    m_server = server;
}

QKlipperSystem *QKlipperConsole::system() const
{
    return m_system;
}

void QKlipperConsole::setSystem(QKlipperSystem *system)
{
    if(m_system == system)
        return;

    m_system = system;
}

QKlipperPrinter *QKlipperConsole::printer() const
{
    return m_printer;
}

void QKlipperConsole::setPrinter(QKlipperPrinter *printer)
{
    if(printer == m_printer)
        return;

    m_printer = printer;
    m_printer->setConsole(this);
}

qreal QKlipperConsole::startupSequenceProgress() const
{
    return m_startupSequenceProgress;
}

void QKlipperConsole::setStartupSequenceProgress(qreal startupSequenceProgress)
{
    m_startupSequenceProgress = startupSequenceProgress;
    emit startupSequenceProgressChanged();
}

void QKlipperConsole::resetStartupSequenceProgress()
{
    setStartupSequenceProgress(0);
}

void QKlipperConsole::sendRpcMessage(QKlipperMessage *message)
{
    if(m_rpcUpdateSocket)
    {
        qDebug() << "Sending RPC method" << message->method();
        QByteArray data = message->toRpc();

        if(m_server->connectionType() == QKlipperServer::Local)
            m_rpcUpdateSocket->write(data);
        else if(m_server->connectionType() == QKlipperServer::Remote)
        {
            QWebSocket *socket = qobject_cast<QWebSocket*>(m_rpcUpdateSocket);

            if(socket)
            {
                data = data.removeLast();

                qint64 sent = socket->sendTextMessage(data);

                if(sent != data.length())
                {
                    qDebug() << QString("Sent %1 of %2").arg(QString::number(sent), QString::number(data.length()));
                }
            }
        }
    }
}

void QKlipperConsole::sendWebSocketMessageAsync(QKlipperMessage *message)
{
    if(!m_messageMap.contains(message->id()))
        m_messageMap.insert(message->id(), message);

    QNetworkReply *reply;

    QNetworkRequest request(m_server->address() + message->toUri());

    if(!m_server->apiKey().isEmpty())
        request.setRawHeader("X-Api-Key", m_server->apiKey().toUtf8());

    if(message->protocol() == QKlipperMessage::HttpPostProtocol)
    {
        QByteArray data;

        if(!message->bodyData().isNull())
        {
            QJsonDocument document;

            if(message->bodyData().isObject())
                document.setObject(message->bodyData().toObject());
            else if(message->bodyData().isArray())
                document.setArray(message->bodyData().toArray());

            data = document.toJson();
        }

        reply = m_networkManager->post(request, data);
    }
    else if(message->protocol() == QKlipperMessage::HttpDeleteProtocol)
        reply = m_networkManager->deleteResource(request);
    else
        reply = m_networkManager->get(request);

    QObject::connect
    (
        reply,
        &QNetworkReply::finished,
        this, [reply, message, this]() {

            if (reply->error())
            {
                QKlipperError error;

                qDebug() << "Error: " + reply->errorString() << message->method() << reply->url() ;

                error.setErrorString(reply->errorString());
                error.setType(QKlipperError::Socket);
                error.setOrigin(message->method());
                error.setErrorTitle("Error Sending Websocket Command");

                message->setError(error);
                bool emitError = true;

                if(hasConnectionState(Startup) && m_ignoreErrorOnStartup.contains(message->method()))
                    emitError = false;

                if(emitError)
                {
                    emit errorOccured(error);
                    return;
                }
            }

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
            {
                QKlipperError error;

                qDebug() << "Error: " + reply->errorString() << message->method() << reply->url() ;

                error.setErrorString(reply->errorString());
                error.setType(QKlipperError::Socket);
                error.setOrigin(message->method());
                error.setErrorTitle("Invalid Response From Server");

                message->setError(error);
                bool emitError = true;

                if(hasConnectionState(Startup) && m_ignoreErrorOnStartup.contains(message->method()))
                    emitError = false;

                if(emitError)
                {
                    emit errorOccured(error);
                    return;
                }
            }

            message->setResponse(responseDocument["result"]);
            parseResponse(message);
        }
    );

    emit messageSent(message);
}

bool QKlipperConsole::sendWebSocketMessage(QKlipperMessage *message, QKlipperError *error)
{
    qDebug() << "Sending WS method" << message->method();

    emit messageSent(message);

    QEventLoop loop;
    QNetworkReply *reply;
    QNetworkRequest request(m_server->address() + message->toUri());

    if(message->protocol() == QKlipperMessage::HttpPostProtocol)
    {
        QByteArray data;

        if(!message->bodyData().isNull())
        {
            QJsonDocument document;

            if(message->bodyData().isObject())
                document.setObject(message->bodyData().toObject());
            else if(message->bodyData().isArray())
                document.setArray(message->bodyData().toArray());

            data = document.toJson();
        }

        reply = m_networkManager->post(request, data);
    }
    else if(message->protocol() == QKlipperMessage::HttpDeleteProtocol)
        reply = m_networkManager->deleteResource(request);
    else
        reply = m_networkManager->get(request);

    QObject::connect
    (
        reply,
        &QNetworkReply::finished,
        this, [&loop]()
        {
            loop.quit();
        }
    );

    loop.exec();

    if (reply->error())
    {
        qDebug() << reply->errorString() << reply->url() ;

        if(!error)
            error = new QKlipperError();

        error->setErrorString(reply->errorString());
        error->setType(QKlipperError::Socket);
        error->setOrigin("Server GCode Script");
        error->setErrorTitle("Error Sending Websocket Command");
        message->setError(*error);

        reply->deleteLater();

        bool emitError = true;

        if(hasConnectionState(Startup) && m_ignoreErrorOnStartup.contains(message->method()))
            emitError = false;

        if(emitError)
            processError(error);

        return false;
    }

    if(error)
        error->setType(QKlipperError::NoError);

    message->setResponse(reply->readAll());

    return true;
}

/*
 * Notification parsing
 */

void QKlipperConsole::parseNotification(QKlipperMessage *message)
{
    if(m_parserMap.contains(message->method()))
    {
        QString key = message->method();
        ParserFunction parser = m_parserMap[key];
        (this->*parser)(message);
    }

    else if(message->method() == QString("notify_klippy_disconnected"))
    {
        removeConnectionState(KlipperConnected);
    }
    else if(message->method() == QString("notify_klippy_ready"))
    {
        if(m_connectionState == Restarting)
        {
            setConnectionState(Syncronized);
        }
        else
            addConnectionState(KlipperConnected);
    }
    else if(message->method() == QString("notify_klippy_shutdown"))
    {
        removeConnectionState(KlipperConnected);
        disconnect();
    }
    else if(message->method() == QString("notify_filelist_changed"))
    {
        if(message->response().toObject().contains(QString("action")))
        {
            if(message->response()["action"] == QString("delete_file")
                || message->response()["action"] == QString("create_file")
                || message->response()["action"] == QString("move_file"))
            {
                QJsonObject item = message->response()["item"].toObject();

                QString path = item["path"].toString();
                QString root = item["root"].toString();

                if(path.contains(QDir::separator()))
                    path = path.mid(0,path.lastIndexOf(QDir::separator()));
                else
                    path.clear();

                path = root + QDir::separator() + path;
                serverFilesList(path);
            }

            else if(message->response()["action"] == QString("delete_dir")
                || message->response()["action"] == QString("create_dir")
                || message->response()["action"] == QString("move_dir"))
            {
                QJsonObject item = message->response()["item"].toObject();

                QString path = item["path"].toString();
                QString root = item["root"].toString();

                path = root + QDir::separator() + path;
                serverFilesList(path);
            }
            else if(message->response()["action"] == QString("root_changed"))
            {
                serverConfig();
            }
        }
    }
    else if(message->method() == QString("notify_gcode_response"))
    {
        if(message->response().toObject().contains("params"))
        {
            QJsonArray paramsArray = message->response()["params"].toArray();

            for(const QJsonValue &paramValue : paramsArray)
            {
                //string message
                QString response = paramValue.toString();

                if(response.startsWith("// probe at"))
                {
                    if(m_printer->bed()->isBedMeshCalibrating())
                    {
                        quint32 points = m_printer->bed()->bedMesh()->reportedProbePoints();
                        m_printer->bed()->bedMesh()->setReportedProbePoints(points + 1);
                    }
                }

                if(response.startsWith("// Mesh Bed Leveling Complete"))
                {
                    m_printer->bed()->setHasBedMeshResult(true);
                }

                emit gcodeResponse(response);
            }
        }

    }

    else if(message->method() == QString("notify_update_response"))
    {
        if(message->response().isObject())
        {
            QJsonObject object = message->response().toObject();
            QString application = object["application"].toString();
            QString message = object["message"].toString();
            bool complete = object["complete"].toBool();

            if(complete)
            {
                m_system->updateManager()->setCurrentStateMessage(QString("Updating %1 Complete. %2").arg(application, message));
                m_system->setState(QKlipperSystem::Idle);
            }
            else
            {
                m_system->setState(QKlipperSystem::Updating);
                m_system->updateManager()->setCurrentStateMessage(QString("Updating %1: %2").arg(application, message));
            }
        }
    }

    //emitted when a package has completed auto-scan for update
    else if(message->method() == QString("notify_update_refreshed"))
    {
        machineUpdateStatusParser(message);
    }
    else if(message->method() == QString("notify_cpu_throttled"))
    {
        QJsonObject response = message->response().toObject();
        QJsonArray flagArray = response["flags"].toArray();
        QStringList flagList;

        for(const QJsonValue &flag : flagArray)
            flagList += flag.toString();

        m_system->throttleState()->setFlags(flagList);
        m_system->throttleState()->setBits(response["bits"].toInt());
    }
    else if(message->method() == QString("notify_history_changed"))
    {
        QJsonObject response = message->response().toObject();
        QJsonObject printJobObject = response["job"].toObject();
        bool newJob = false;
        QKlipperPrintJob *printJob = m_server->jobQueue()->job(printJobObject["id"].toString());

        if(!printJob)
        {
            printJob = new QKlipperPrintJob(m_server->jobQueue());
            newJob = true;
        }

        if(newJob)
            m_server->jobQueue()->addJob(printJob);
    }
    else if(message->method() == QString("notify_user_created"))
    {
        accessGetUser();
    }
    else if(message->method() == QString("notify_user_deleted"))
    {
        QJsonObject response = message->response().toObject();

        for(QKlipperUser &user : m_server->userList())
        {
            if(user.username() == response["username"].toString())
            {
                m_server->deleteUser(user);
                break;
            }
        }
    }
    else if(message->method() == QString("notify_user_logged_out"))
    {
        accessGetUser();
    }
    else if(message->method() == QString("notify_service_state_changed"))
    {
        QJsonObject response = message->response().toObject();
        QStringList stateKeys = response.keys();

        for(const QString &key : stateKeys)
        {
            if(m_system->services().contains(key))
            {
                QJsonObject stateObject = response[key].toObject();
                m_system->services()[key]->setActiveState(stateObject["active_state"].toString());
                m_system->services()[key]->setSubState(stateObject["sub_state"].toString());
            }
        }
    }
    else if(message->method() == QString("notify_job_queue_changed"))
    {
        QJsonObject response = message->response().toObject();

        if(response["action"] == QString("state_changed")) {
            m_server->jobQueue()->setState(response["queue_state"].toString());
        }
        else {
            serverJobQueueStatus();
        }

    }
    else if(message->method() == QString("notify_button_event"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_button_event",
            "params": [
                {
                    "name": "my_button",
                    "type": "gpio",
                    "event": {
                        "elapsed_time": 0.09323832602240145,
                        "received_time": 698614.214597004,
                        "render_time": 698614.214728513,
                        "pressed": false
                    },
                    "aux": null
                }
            ]
        }
        The params array will always contain a single object with the following fields:

        name: The name of the configured button

        type: The button type, currently this will always be gpio

        event: An object with details about the button event, containing the following fields:

        elapsed_time: The time elapsed (in seconds) since the last detected button event

        received_time: The time the event was detected according to asyncio's monotonic clock. Note that this is not in "unix time".

        render_time: The time the template was rendered (began execution) according to asyncio's monotonic clock.
        It is possible execution of an event may be delayed well beyond the received_time.

        pressed: A boolean value to indicate if the button is currently pressed.

        aux: This is an optional field where the button may specify any json encodable value. Clients may suggest a specific button
        configuration that includes details about the event. If no aux parameter is specified in the configuration this will be a null value.
        */
    }
    else if(message->method() == QString("notify_announcement_update"))
    {
        serverAnnouncementsListParser(message);
    }
    else if(message->method() == QString("notify_announcement_dismissed"))
    {
        serverAnnouncementsUpdate();
    }
    else if(message->method() == QString("notify_announcement_wake"))
    {
        serverAnnouncementsUpdate();
    }
    else if(message->method() == QString("notify_sudo_alert"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_sudo_alert",
            "params": [
                {
                    "sudo_requested": true,
                    "sudo_messages": [
                        "Sudo password required to update Moonraker's systemd service."
                    ]
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_webcams_changed"))
    {
        serverWebcamList();
    }
    else if(message->method() == QString("notify_active_spool_set"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_active_spool_set",
            "params": [
                {
                    "spool_id": 1
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_spoolman_status_changed"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_spoolman_status_changed",
            "params": [
                {
                    "spoolman_connected": false
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_agent_event"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_agent_event",
            "params": [
                {
                    "agent": "moonagent",
                    "event": "connected",
                    "data": {
                        "name": "moonagent",
                        "version": "0.0.1",
                        "type": "agent",
                        "url": "https://github.com/arksine/moontest"
                    }
                }
            ]
        }
        */
    }
    else if(message->method() == QString("sensors:sensor_update"))
    {
        QJsonObject response = message->response().toObject();
        QStringList keys = response.keys();

        for(const QString &key : keys)
        {
            if(m_system->m_sensors.contains(key))
            {
                QJsonObject sensorObject = response[key].toObject();
                QStringList valueKeys = sensorObject.keys();

                for(const QString &valueKey : valueKeys)
                    m_system->m_sensors[key]->addValue(valueKey, sensorObject[valueKey].toVariant());
            }
        }
    }
}

/*
 * Message parsing
 */

void QKlipperConsole::parseResponse(QKlipperMessage *message)
{
    qDebug() << QString("Parsing Response") << message->method();

    if(m_parserMap.contains(message->method()))
    {
        QString key = message->method();
        ParserFunction parser = m_parserMap[key];
        (this->*parser)(message);
    }

    //delete message unless it came from the user
    if(message->origin() != QKlipperMessage::User)
        deleteMessage(message);

    processStartupSequence();
}

void QKlipperConsole::processStartupSequence()
{
    if(hasConnectionState(Startup))
    {
        if(m_startupSequence.isEmpty())
        {
            qDebug() << QString("Completed Startup Sequence");
            setStartupSequenceProgress(100);
            setStartupSequenceText("Connected");
            removeConnectionState(Startup);
            addConnectionState(Initialized);
        }
        else
        {
            qreal progress = 1.0 - ((qreal)m_startupSequence.count() / m_startupSequenceCount);
            setStartupSequenceProgress(progress);

            qDebug() << progress;

            StartupFunction function = m_startupSequence.dequeue();
            (this->*function)();
        }
    }
}

void QKlipperConsole::accessLoginParser(QKlipperMessage *message)
{
    QKlipperAccessDetails accessDetails;

    accessDetails.setIsLoggedIn(true);
    accessDetails.setToken(message->response()["token"].toString());
    accessDetails.setRefreshToken(message->response()["refresh_token"].toString());

    foreach(QKlipperUser user, m_server->userList())
    {
        if(user.username() == message->response()["username"].toString())
        {
            accessDetails.setUser(user);
            break;
        }
    }

    m_server->setAccessDetails(accessDetails);
}

void QKlipperConsole::accessLogoutParser(QKlipperMessage *message)
{
    QKlipperUser user = m_server->m_accessDetails.user();

    if(user.username() == message->response()["username"].toString())
    {
        QKlipperAccessDetails accessDetails;

        accessDetails.setIsLoggedIn(false);
        accessDetails.setToken("");
        accessDetails.setRefreshToken("");
        accessDetails.setUser(QKlipperUser());

        m_server->setAccessDetails(accessDetails);
    }
}

void QKlipperConsole::accessGetUserParser(QKlipperMessage *message)
{
    QKlipperAccessDetails accessDetails = m_server->accessDetails();

    foreach(QKlipperUser user, m_server->userList())
    {
        if(user.username() == message->response()["username"].toString())
        {
            accessDetails.setUser(user);
            break;
        }
    }

    m_server->setAccessDetails(accessDetails);
}

void QKlipperConsole::accessCreateUserParser(QKlipperMessage *message)
{
    QKlipperUser user;
    QKlipperUserList users = m_server->userList();
    QKlipperAccessDetails accessDetails;

    accessDetails.setIsLoggedIn(true);
    accessDetails.setToken(message->response()["token"].toString());
    accessDetails.setRefreshToken(message->response()["refresh_token"].toString());

    user.setUsername(message->response()["username"].toString());
    user.setSource(message->response()["source"].toString());
    user.setCreatedOn(QDateTime::currentDateTime().toSecsSinceEpoch());

    users.append(user);
    m_server->setUserList(users);

    accessDetails.setUser(user);
    m_server->setAccessDetails(accessDetails);
}

void QKlipperConsole::accessDeleteUserParser(QKlipperMessage *message)
{
    QKlipperAccessDetails accessDetails = m_server->accessDetails();
    QKlipperUserList users = m_server->userList();
    QKlipperUser user;

    for(int i = 0; i < users.count(); i++)
    {
        if(users[i].username() == message->response()["username"].toString())
        {
            user = users[i];
            users.removeAt(i);

            m_server->setUserList(users);
            break;
        }
    }

    if(user == accessDetails.user())
    {
        accessDetails.setIsLoggedIn(false);
        accessDetails.setToken("");
        accessDetails.setRefreshToken("");
        accessDetails.setUser(QKlipperUser());

        m_server->setAccessDetails(accessDetails);
    }
}

void QKlipperConsole::accessUsersListParser(QKlipperMessage *message)
{
    QJsonArray userArray = message->response()["users"].toArray();

    QKlipperUserList users;

    for(int i = 0; i < userArray.count(); i++)
    {
        QJsonObject userObject = userArray[i].toObject();

        QKlipperUser user;
        user.setUsername(userObject["username"].toString());
        user.setSource(userObject["source"].toString());
        user.setCreatedOn(userObject["created_on"].toDouble());

        users.append(user);
    }

    m_server->setUserList(users);
}

void QKlipperConsole::accessUserPasswordResetParser(QKlipperMessage *message)
{
    QKlipperAccessDetails accessDetails = m_server->accessDetails();

    foreach(QKlipperUser user, m_server->userList())
    {
        if(user.username() == message->response()["username"].toString())
        {
            accessDetails.setUser(user);
            break;
        }
    }

    m_server->setAccessDetails(accessDetails);
}

void QKlipperConsole::clientIdentifierParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("connection_id"))
        m_server->setConnectionId(message->response()["connection_id"].toString());
}

void QKlipperConsole::machineSystemInfoParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("system_info"))
    {
        QJsonObject systemInfo = message->response()["system_info"].toObject();

        if(systemInfo.contains("cpu_info") && systemInfo["cpu_info"].isObject())
        {
            QJsonObject cpuInfoObject = systemInfo["cpu_info"].toObject();
            QKlipperCpuInfo cpuInfo = m_system->cpuInfo();

            if(cpuInfoObject.contains("cpu_count"))
                cpuInfo.setCpuCount(cpuInfoObject["cpu_count"].toInt());

            if(cpuInfoObject.contains("bits"))
            {
                QString bitString = cpuInfoObject["bits"].toString();
                bitString.remove("bit");

                cpuInfo.setBitCount(bitString.toInt());
            }

            if(cpuInfoObject.contains("processor"))
                cpuInfo.setProcessor(cpuInfoObject["processor"].toString());

            if(cpuInfoObject.contains("cpu_desc"))
                cpuInfo.setDescription(cpuInfoObject["cpu_desc"].toString());

            if(cpuInfoObject.contains("serial_number"))
                cpuInfo.setSerialNumber(cpuInfoObject["serial_number"].toString());

            if(cpuInfoObject.contains("hardware_desc"))
                cpuInfo.setHardwareDescription(cpuInfoObject["hardware_desc"].toString());

            if(cpuInfoObject.contains("model"))
                cpuInfo.setModel(cpuInfoObject["model"].toString());

            if(cpuInfoObject.contains("total_memory"))
                cpuInfo.setTotalMemory(cpuInfoObject["total_memory"].toInt());

            if(cpuInfoObject.contains("memory_units"))
                cpuInfo.setMemoryUnits(cpuInfoObject["memory_units"].toString());

            m_system->setCpuInfo(cpuInfo);
        }

        if(systemInfo.contains("sd_info"))
        {
            QJsonObject sdInfoObject = systemInfo["sd_info"].toObject();
            QKlipperSdInfo sdInfo = m_system->sdInfo();

            if(sdInfoObject.contains("manufacturer_id"))
                sdInfo.setManufacturerId(sdInfoObject["manufacturer_id"].toString());

            if(sdInfoObject.contains("manufacturer"))
                sdInfo.setManufacturer(sdInfoObject["manufacturer"].toString());

            if(sdInfoObject.contains("oem_id"))
                sdInfo.setOemId(sdInfoObject["oem_id"].toString());

            if(sdInfoObject.contains("product_name"))
                sdInfo.setProductName(sdInfoObject["product_name"].toString());

            if(sdInfoObject.contains("product_revision"))
                sdInfo.setProductRevision(sdInfoObject["product_revision"].toString());

            if(sdInfoObject.contains("serial_number"))
                sdInfo.setSerialNumber(sdInfoObject["serial_number"].toString());

            if(sdInfoObject.contains("manufacturer_date"))
                sdInfo.setManufacturerDate(sdInfoObject["manufacturer_date"].toString());

            if(sdInfoObject.contains("capacity"))
                sdInfo.setCapacityString(sdInfoObject["capacity"].toString());

            if(sdInfoObject.contains("total_bytes"))
                sdInfo.setTotalBytes(sdInfoObject["total_memory"].toInt());

            m_system->setSdInfo(sdInfo);
        }

        if(systemInfo.contains("distribution") && systemInfo["distribution"].isObject())
        {
            QJsonObject distributionObject = systemInfo["distribution"].toObject();
            QKlipperDistributionInfo distribution = m_system->distributionInfo();

            if(distributionObject.contains("name"))
                distribution.setName(distributionObject["name"].toString());

            if(distributionObject.contains("id"))
                distribution.setId(distributionObject["id"].toString());

            if(distributionObject.contains("version"))
                distribution.setVersion(distributionObject["version"].toString());

            if(distributionObject.contains("version_parts"))
            {
                QJsonObject versionParts = distributionObject["version_parts"].toObject();

                if(versionParts.contains("major"))
                    distribution.setVersionMajor(versionParts["major"].toString());
                if(versionParts.contains("minor"))
                    distribution.setVersionMinor(versionParts["minor"].toString());
                if(versionParts.contains("build_number"))
                    distribution.setVersionBuildNumber(versionParts["build_number"].toString());

            }

            if(distributionObject.contains("like"))
                distribution.setStyle(distributionObject["like"].toString());

            if(distributionObject.contains("codename"))
                distribution.setCodename(distributionObject["codename"].toString());

            m_system->setDistributionInfo(distribution);
        }

        if(systemInfo.contains("available_services") && systemInfo["available_services"].isArray())
        {
            QJsonArray servicesArray = systemInfo["available_services"].toArray();
            QStringList availableServices;

            for(int i = 0; i < servicesArray.count(); i++)
                availableServices.append(servicesArray[i].toString());

            m_system->setAvailableServices(availableServices);
        }

        if(systemInfo.contains("service_state") && systemInfo["service_state"].isObject())
        {
            QJsonObject serviceStateObject = systemInfo["service_state"].toObject();
            QStringList keys = serviceStateObject.keys();

            QMap<QString, QKlipperService*> serviceStates = m_system->services();

            for(const QString &key : keys)
            {
                if(serviceStateObject[key].isObject())
                {
                    QJsonObject stateObject = serviceStateObject[key].toObject();
                    QKlipperService *state = m_system->service(key);

                    if(!state)
                        state = new QKlipperService(m_system);

                    if(stateObject.contains("active_state"))
                        state->setActiveState(stateObject["active_state"].toString());

                    if(stateObject.contains("sub_state"))
                        state->setSubState(stateObject["sub_state"].toString());

                    state->setName(key);

                    if(!m_system->services().contains(key))
                        m_system->addService(state);
                }
            }
        }

        if(systemInfo.contains("network") && systemInfo["network"].isObject())
        {
            QJsonObject networkObject = systemInfo["network"].toObject();
            QStringList keys = networkObject.keys();

            QMap<QString, QKlipperNetworkInterface> interfaces;

            foreach(QString key, keys)
            {
                QJsonObject interfaceObject = networkObject[key].toObject();
                QKlipperNetworkInterface interface;

                if(interfaceObject.contains("mac_address"))
                    interface.setMacAddress(interfaceObject["mac_address"].toString());

                if(interfaceObject.contains("ip_addresses"))
                {
                    QJsonArray ipArray = interfaceObject["ip_addresses"].toArray();
                    QList<QKlipperNetworkAddress> addressList;

                    for(int i = 0; i < ipArray.count(); i++)
                    {
                        QJsonObject ipObject = ipArray[i].toObject();
                        QKlipperNetworkAddress ipAddress;

                        if(ipObject.contains("family"))
                            ipAddress.family = ipObject["family"].toString();

                        if(ipObject.contains("address"))
                            ipAddress.address = ipObject["address"].toString();

                        if(ipObject.contains("is_link_local"))
                            ipAddress.isLinkLocal = ipObject["is_link_local"].toBool();

                        addressList.append(ipAddress);
                    }

                    interface.setIpAddresses(addressList);
                }

                interfaces.insert(key, interface);
            }

            m_system->setNetworkInterfaces(interfaces);
        }

        if(systemInfo.contains("canbus") && systemInfo["canbus"].isObject())
        {
            QJsonObject canbusObject = systemInfo["canbus"].toObject();
            QStringList keys = canbusObject.keys();

            QKlipperCanBus *bus = m_system->canBus();
            QKlipperCanBusDeviceMap devices;

            foreach(QString key, keys)
            {
                if(canbusObject[key].isObject())
                {
                    QJsonObject canDeviceObject = canbusObject[key].toObject();
                    QKlipperCanBusDevice canDevice;

                    if(canDeviceObject.contains("tx_queue_len"))
                        canDevice.setQueueLength(canDeviceObject["tx_queue_len"].toInt());

                    if(canDeviceObject.contains("bitrate"))
                        canDevice.setBitrate(canDeviceObject["bitrate"].toInt());

                    if(canDeviceObject.contains("driver"))
                        canDevice.setDriver(canDeviceObject["driver"].toString());

                    devices.insert(key, canDevice);
                }
            }

            bus->setDevices(devices);
        }

        if(systemInfo.contains("python") && systemInfo["python"].isObject())
        {
            QJsonObject pythonObject = systemInfo["python"].toObject();

            if(pythonObject.contains("version_string"))
                m_system->setPythonVersion(pythonObject["version_string"].toString());
        }
    }
}

void QKlipperConsole::machineProcStatsParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("params"))
    {
        QJsonArray paramArray = message->response()["params"].toArray();

        for(const QJsonValue &value : paramArray)
        {
            message->setResponse(value);
            machineProcStatsParser(message);
        }
    }

    QKlipperCpuInfo cpuInfo = m_system->cpuInfo();
    bool cpuInfoChanged = false;

    QKlipperMemoryStats *memory = m_system->memoryStats();
    QKlipperThrottleState *throttleState = m_system->throttleState();

    //Parse cpu usage status
    if(message->response().toObject().contains("system_cpu_usage"))
    {
        QJsonObject cpuLoadArray = message->response()["system_cpu_usage"].toObject();
        QStringList keys = cpuLoadArray.keys();
        QList<qreal> coreUsage;

        foreach(QString key, keys)
        {
            coreUsage.append(cpuLoadArray[key].toDouble());
        }

        cpuInfo.setUsage(coreUsage[0]);
        cpuInfo.setCoreUsage(coreUsage);

        cpuInfoChanged = true;
    }

    if(message->response().toObject().contains("cpu_temp"))
    {
        cpuInfo.setTemp(message->response()["cpu_temp"].toDouble());
        cpuInfoChanged = true;
    }

    if(cpuInfoChanged)
        m_system->setCpuInfo(cpuInfo);

    if(message->response().toObject().contains("throttled_state"))
    {
        QJsonObject throttleObject = message->response()["throttled_state"].toObject();

        if(throttleObject.contains("bits"))
            throttleState->setBits(throttleObject["bits"].toInt());

        if(throttleObject.contains("flags"))
        {
            QJsonArray flagsArray = throttleObject["flags"].toArray();
            QStringList flags;

            for(int i = 0; i < flagsArray.count(); i++)
                flags += flagsArray[i].toString();

            throttleState->setFlags(flags);
        }
    }

    if(message->response().toObject().contains("network"))
    {
        QJsonObject networkLoadObject = message->response()["network"].toObject();
        QStringList keys = networkLoadObject.keys();

        QMap<QString,QKlipperNetworkStatsEntry> stats = m_system->networkStats();

        foreach(QString key, keys)
        {
            QJsonObject statObject = networkLoadObject[key].toObject();
            QKlipperNetworkStatsEntry statEntry;

            if(statObject.contains("rx_bytes"))
                statEntry.setRxBytes(statObject["rx_bytes"].toDouble());

            if(statObject.contains("tx_bytes"))
                statEntry.setTxBytes(statObject["tx_bytes"].toDouble());

            if(statObject.contains("bandwidth"))
                statEntry.setBandwidth(statObject["bandwidth"].toDouble());

            stats[key] = statEntry;
        }

        m_system->setNetworkStats(stats);
    }

    //Parse memory status
    if(message->response().toObject().contains("system_memory"))
    {
        QJsonObject memoryLoadObject = message->response()["system_memory"].toObject();

        if(memoryLoadObject.contains("total"))
            memory->setTotal(memoryLoadObject["total"].toDouble());

        if(memoryLoadObject.contains("used"))
            memory->setUsed(memoryLoadObject["used"].toDouble());

        memory->setAverage(memory->used() / memory->total());
    }

    if(message->response().toObject().contains("system_uptime"))
        m_system->setUptime(message->response()["system_uptime"].toDouble());

    if(message->response().toObject().contains("websocket_connections"))
        m_system->setConnectionCount(message->response()["websocket_connections"].toInt());
}

void QKlipperConsole::machinePeripheralsUSBParser(QKlipperMessage *message)
{
    if(message->response()["usb_devices"].isArray())
    {
        QJsonArray usbArray = message->response()["usb_devices"].toArray();
        QList<QKlipperUsbPeripheral> devices;

        for(int i = 0; i < usbArray.count(); i++)
        {
            QJsonObject usbDeviceObject = usbArray[i].toObject();
            QKlipperUsbPeripheral usbDevice;

            usbDevice.setDeviceNumber(usbDeviceObject["device_num"].toInt());
            usbDevice.setBusNumber(usbDeviceObject["bus_num"].toInt());

            usbDevice.setLocation(usbDeviceObject["vendor_id"].toString());
            usbDevice.setProductId(usbDeviceObject["product_id"].toString());
            usbDevice.setManufacturer(usbDeviceObject["manufacturer"].toString());
            usbDevice.setProduct(usbDeviceObject["product"].toString());
            usbDevice.setClassName(usbDeviceObject["class"].toString());
            usbDevice.setSubclassName(usbDeviceObject["subclass"].toString());
            usbDevice.setProtocol(usbDeviceObject["protocol"].toString());
            usbDevice.setDescription(usbDeviceObject["description"].toString());


            devices.append(usbDevice);
        }

        m_system->setUsbPeripherals(devices);
    }
}

void QKlipperConsole::machinePeripheralsSerialParser(QKlipperMessage *message)
{
    if(message->response()["serial_devices"].isArray())
    {
        QJsonArray usbArray = message->response()["usb_devices"].toArray();
        QList<QKlipperSerialPeripheral> devices;

        for(int i = 0; i < usbArray.count(); i++)
        {
            QJsonObject usbDeviceObject = usbArray[i].toObject();
            QKlipperSerialPeripheral serialDevice;

            serialDevice.setType(usbDeviceObject["device_type"].toString());
            serialDevice.setPath(usbDeviceObject["device_path"].toString());
            serialDevice.setName(usbDeviceObject["device_name"].toString());
            serialDevice.setDriver(usbDeviceObject["driver_name"].toString());
            serialDevice.setHardwarePath(usbDeviceObject["path_by_hardware"].toString());
            serialDevice.setPathById(usbDeviceObject["path_by_id"].toString());
            serialDevice.setUsbLocation(usbDeviceObject["usb_location"].toString());

            devices.append(serialDevice);
        }

        m_system->setSerialPeripherals(devices);
    }
}

void QKlipperConsole::machinePeripheralsVideoParser(QKlipperMessage *message)
{
    if(message->response()["v412_devices"].isArray())
    {
        QJsonArray v412DeviceArray = message->response()["v412_devices"].toArray();
        QList<QKlipperV412Device> devices;

        for(int i = 0; i < v412DeviceArray.count(); i++)
        {
            QJsonObject deviceObject = v412DeviceArray[i].toObject();
            QKlipperV412Device v412Device;

            v412Device.setName(deviceObject["device_name"].toString());
            v412Device.setPath(deviceObject["device_path"].toString());
            v412Device.setCameraName(deviceObject["camera_name"].toString());
            v412Device.setDriver(deviceObject["driver_name"].toString());
            v412Device.setAltName(deviceObject["alt_name"].toString());
            v412Device.setHardwareBus(deviceObject["hardware_bus"].toString());
            v412Device.setVersion(deviceObject["version"].toString());
            v412Device.setHardwarePath(deviceObject["path_by_hardware"].toString());
            v412Device.setPathById(deviceObject["path_by_id"].toString());
            v412Device.setUsbLocation(deviceObject["usb_location"].toString());

            if(deviceObject["modes"].isArray())
            {
                QJsonArray modes = deviceObject["modes"].toArray();

                for(int m = 0; m < modes.count(); m++)
                {
                    QJsonObject modeObject = modes[i].toObject();
                    QJsonArray modeFlagArray = modeObject["flags"].toArray();
                    QJsonArray resolutionArray = modeObject["resolutions"].toArray();
                    QKlipperV412Mode mode;

                    mode.setDescription(modeObject["description"].toString());
                    mode.setFormat(modeObject["format"].toString());

                    QStringList resolutions;

                    for(int r = 0; r < resolutionArray.count(); r++)
                        resolutions += resolutionArray[r].toString();

                    mode.setResolutions(resolutions);

                    QStringList flags;

                    for(int f = 0; f < modeFlagArray.count(); f++)
                        flags += modeFlagArray[f].toString();

                    mode.setFlags(flags);
                }
            }

            if(deviceObject["capabilities"].isArray())
            {
                QJsonArray capabilitiesArray = deviceObject["capabilities"].toArray();

                QStringList capabilities;

                for(int c = 0; c < capabilitiesArray.count(); c++)
                    capabilities += capabilitiesArray[c].toString();

                v412Device.setCapabilities(capabilities);
            }

            devices.append(v412Device);
        }

        m_system->setV412Devices(devices);
    }

    if(message->response()["libcamera_devices"].isArray())
    {
        QJsonArray cameraArray = message->response()["libcamera_devices"].toArray();
        QList<QKlipperLibcameraDevice> devices;

        for(int i = 0; i < cameraArray.count(); i++)
        {
            QJsonObject cameraObject = cameraArray[i].toObject();
            QJsonArray modesArray = cameraObject["modes"].toArray();
            QKlipperLibcameraDevice camera;

            camera.setId(cameraObject["libcamera_id"].toString());
            camera.setModel(cameraObject["model"].toString());

            for(int m = 0; m < modesArray.count(); m++)
            {
                QJsonObject modeObject = modesArray[i].toObject();
                QJsonArray resolutionArray = modeObject["resolutions"].toArray();
                QKlipperLibCameraMode mode;

                mode.format = (modeObject["format"].toString());

                QStringList resolutions;

                for(int r = 0; r < resolutionArray.count(); r++)
                    mode.resolutions += resolutionArray[r].toString();

                camera.modes().append(mode);
            }

            devices.append(camera);
        }

        m_system->setLibcameraDevices(devices);
    }
}

void QKlipperConsole::machinePeripheralsCanbusParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
    // QString interfaceName = message->params()["interface"].toString();
    // interfaceName.remove("can");

    // qint32 interfaceNumber = interfaceName.toInt();

    // if(message->response()["can_uuids"].isArray())
    // {
    //     QJsonArray canArray = message->response()["can_uuids"].toArray();
    //     QKlipperCanBus canBus;

    //     for(int i = 0; i < canArray.count(); i++)
    //     {
    //         QJsonObject canObject = canArray[i].toObject();
    //         QKlipperCanBus::Interface interface;

    //         interface.application = canObject["application"].toString();
    //         interface.uuid = canObject["uuid"].toString();
    //     }

    //     m_printer->system()->canBusses().insert(interfaceNumber, canBus);
    // }
}

void QKlipperConsole::machineUpdateStatusParser(QKlipperMessage *message)
{
    QKlipperUpdateManager *updateState = m_system->updateManager();

    updateState->setIsBusy(message->response()["busy"].toBool());
    updateState->setGithubLimitResetTime(message->response()["github_limit_reset_time"].toInt());
    updateState->setGithubRateLimit(message->response()["github_rate_limit"].toInt());
    updateState->setGithubRequestsRemaining(message->response()["github_requests_remaining"].toInt());

    //Grab the version info
    QJsonObject versionObject = message->response()["version_info"].toObject();
    QStringList keys = versionObject.keys();
    QMap<QString, QKlipperUpdatePackage> packages;

    QStringList systemPackages = m_system->updateManager()->systemPackages();

    foreach(QString key, keys)
    {
        //System version info
        if(key == QString("system"))
        {
            QJsonObject systemObject = versionObject[key].toObject();
            QJsonArray packageArray = systemObject["package_list"].toArray();

            updateState->setSystemPackageCount(systemObject["package_count"].toInt());

            for(int i = 0; i < packageArray.count(); i++)
            {
                if(!systemPackages.contains(packageArray[i].toString()))
                    systemPackages += packageArray[i].toString();
            }
        }
        //Package states
        else
        {
            QJsonObject packageObject = versionObject[key].toObject();

            QJsonArray commitsArray = packageObject["commits_behind"].toArray();
            QJsonArray tagsArray = packageObject["info_tags"].toArray();
            QJsonArray gitArray = packageObject["git_messages"].toArray();
            QJsonArray warningArray = packageObject["warnings"].toArray();
            QJsonArray anomaliesArray = packageObject["anomalies"].toArray();

            //Package information
            QKlipperUpdatePackage packageState;

            //Strings
            packageState.setChannel(packageObject["channel"].toString());
            packageState.setConfiguredType(packageObject["configured_type"].toString());
            packageState.setDetectedType(packageObject["detected_type"].toString());
            packageState.setRemoteAlias(packageObject["remote_alias"].toString());
            packageState.setBranch(packageObject["branch"].toString());
            packageState.setOwner(packageObject["owner"].toString());
            packageState.setRepoName(packageObject["repo_name"].toString());
            packageState.setVersion(packageObject["version"].toString());
            packageState.setRemoteVersion(packageObject["remote_version"].toString());
            packageState.setRollbackVersion(packageObject["rollback_version"].toString());
            packageState.setCurrentHash(packageObject["current_hash"].toString());
            packageState.setRemoteHash(packageObject["remote_hash"].toString());
            packageState.setFullVersionString(packageObject["full_version_string"].toString());
            packageState.setRecoveryUrl(packageObject["recovery_url"].toString());
            packageState.setRemoteUrl(packageObject["remote_url"].toString());

            //Bools
            packageState.setDebugEnabled(packageObject["debug_enabled"].toBool());
            packageState.setIsValid(packageObject["is_valid"].toBool());
            packageState.setCorrupt(packageObject["corrupt"].toBool());
            packageState.setIsDirty(packageObject["is_dirty"].toBool());
            packageState.setDetached(packageObject["detached"].toBool());
            packageState.setPristine(packageObject["pristine"].toBool());

            //Tags
            QStringList tags;

            for(int i = 0; i < tagsArray.count(); i++)
                tags += tagsArray[i].toString();

            packageState.setInfoTags(tags);

            //Git messages
            QStringList gitMessages;

            for(int i = 0; i < gitArray.count(); i++)
                gitMessages += gitArray[i].toString();

            packageState.setGitMessages(gitMessages);

            //Warning messages
            QStringList warnings;

            for(int i = 0; i < warningArray.count(); i++)
                warnings += warningArray[i].toString();

            packageState.setWarnings(warnings);

            //Anomalies messages
            QStringList anomalies;

            for(int i = 0; i < anomaliesArray.count(); i++)
                anomalies += anomaliesArray[i].toString();

            packageState.setAnomalies(anomalies);

            QList<QKlipperUpdateCommit> commits;

            //Commits behind information
            for(int i = 0; i < commitsArray.count(); i++)
            {
                QJsonObject commitObject = commitsArray[i].toObject();
                QKlipperUpdateCommit commit;

                commit.setSha(commitObject["sha"].toString());
                commit.setAuthor(commitObject["author"].toString());
                commit.setDate(commitObject["date"].toString());
                commit.setSubject(commitObject["subject"].toString());
                commit.setMessage(commitObject["message"].toString());
                commit.setTag(commitObject["tag"].toString());

                commits.append(commit);
            }

            packageState.setCommitsBehind(commits);

            //Add to packages map
            packages.insert(key, packageState);
        }
    }

    updateState->setPackages(packages);
    updateState->setSystemPackages(systemPackages);
}

void QKlipperConsole::machinePowerDeviceListParser(QKlipperMessage *message)
{
    if(message->response().isObject() && message->response().toObject().contains("devices"))
    {
        QJsonArray deviceArray = message->response().toObject().value("devices").toArray();

        for(const QJsonValue &value : deviceArray)
        {
            if(value.isObject())
            {
                QJsonObject deviceObject = value.toObject();
                QString name = deviceObject["device"].toString();
                QString type = deviceObject["type"].toString();
                bool isOn = (deviceObject["status"].toString() == "on");
                bool lockedWhilePrinting = deviceObject["locked_while_printing"].toBool();
                QKlipperPowerDevice *device;

                if(m_system->powerDevices().contains(name))
                {
                    device = m_system->m_powerDevices[name];
                    device->setName(name);
                    device->setTypeString(type);
                    device->setIsOn(isOn);
                    device->setLockedWhilePrinting(lockedWhilePrinting);
                }
                else
                {
                    device = new QKlipperPowerDevice(m_system);
                    device->setName(name);
                    device->setTypeString(type);
                    device->setIsOn(isOn);
                    device->setLockedWhilePrinting(lockedWhilePrinting);
                    m_system->setPowerDevice(device);
                }
            }
        }
    }
}

void QKlipperConsole::machinePowerDeviceParser(QKlipperMessage *message)
{
    Q_UNUSED(message)
}

void QKlipperConsole::machineLedStripListParser(QKlipperMessage *message)
{
    if(message->response().isObject())
    {
        QJsonObject response = message->response().toObject();
        QStringList keys = response.keys();

        for(QString &key : keys)
        {
            QJsonObject ledObject = response[key].toObject();
            QKlipperLedStrip *ledStrip;

            bool newStrip = false;

            if(!m_system->ledStrips().contains(key))
            {
                ledStrip = new QKlipperLedStrip(m_system);
                newStrip = true;
            }
            else
                ledStrip = m_system->ledStrips()[key];

            ledStrip->setBrightness(ledObject["brightness"].toInt());
            ledStrip->setChainCount(ledObject["chain_count"].toInt());
            ledStrip->setError(ledObject["error"].toString());
            ledStrip->setName(ledObject["strip"].toString());
            ledStrip->setStatus(ledObject["status"].toString());
            ledStrip->setIntensity(ledObject["intensity"].toInt());
            ledStrip->setPreset(ledObject["preset"].toInt());
            ledStrip->setSpeed(ledObject["speed"].toInt());

            if(newStrip)
                m_system->setLedStrip(ledStrip);
        }
    }
}

void QKlipperConsole::machineSensorListParser(QKlipperMessage *message)
{
    if(message->response().isObject() && message->response().toObject().contains("sensors"))
    {
        QJsonObject sensorsObject = message->response().toObject()["sensors"].toObject();
        QStringList sensorKeys = sensorsObject.keys();

        for(QString &key : sensorKeys)
        {
            QJsonObject sensorObject = sensorsObject[key].toObject();
            QKlipperSensor *sensor;

            if(m_system->m_sensors.contains(key))
                sensor = m_system->m_sensors[key];
            else
                sensor = new QKlipperSensor(m_system);

            if(sensorObject.contains("id"))
                sensor->setId(sensorObject["id"].toString());

            if(sensorObject.contains("type"))
                sensor->setType(sensorObject["id"].toString());

            if(sensorObject.contains("friendly_name"))
                sensor->setFriendlyName(sensorObject["friendly_name"].toString());

            if(sensorObject.contains("values"))
            {
                QJsonObject valuesObject = sensorObject["values"].toObject();
                QStringList valueKeys = valuesObject.keys();

                for(QString &valueKey : valueKeys)
                {
                    if(valuesObject[valueKey].isArray())
                    {
                        QJsonArray valueArray = valuesObject[valueKey].toArray();

                        for(const QJsonValue &value : valueArray)
                            sensor->addValue(valueKey, value.toVariant());
                    }
                    else
                        sensor->addValue(valueKey, valuesObject[valueKey].toVariant());
                }
            }

            if(sensorObject.contains("parameter_info"))
            {
                QJsonArray parameterArray = sensorObject["parameter_info"].toArray();

                for(const QJsonValue &parameter : parameterArray)
                {
                    if(parameter.isObject())
                    {
                        QJsonObject parameterObject = parameter.toObject();

                        if(parameterObject.contains("units") && parameterObject.contains("name"))
                        sensor->addParameter(parameterObject["name"].toString(), parameterObject["units"].toString());
                    }
                }
            }

            if(sensorObject.contains("history_fields"))
            {
                QJsonArray historyArray = sensorObject["history_fields"].toArray();

                for(const QJsonValue &historyValue : historyArray)
                {
                    if(historyValue.isObject())
                    {
                        QJsonObject historyObject = historyValue.toObject();
                        QKlipperSensorData data;

                        if(historyObject.contains("field"))
                            data.m_field = historyObject["field"].toString();

                        if(historyObject.contains("provider"))
                            data.m_provider = historyObject["provider"].toString();

                        if(historyObject.contains("description"))
                            data.m_description = historyObject["description"].toString();

                        if(historyObject.contains("strategy"))
                            data.m_strategy = historyObject["strategy"].toString();

                        if(historyObject.contains("units"))
                            data.m_units = historyObject["units"].toString();

                        if(historyObject.contains("init_tracker"))
                            data.m_initTracker = historyObject["init_tracker"].toBool();

                        if(historyObject.contains("exclude_paused"))
                            data.m_excludePaused = historyObject["exclude_paused"].toBool();

                        if(historyObject.contains("report_total"))
                            data.m_reportTotal = historyObject["report_total"].toBool();

                        if(historyObject.contains("report_maximum"))
                            data.m_reportMaximum = historyObject["report_maximum"].toBool();

                        if(historyObject.contains("precision"))
                            data.m_precision = historyObject["precision"].toInt();

                        if(historyObject.contains("parameter"))
                            data.m_parameter = historyObject["parameter"].toString();

                        if(data.isValid())
                            sensor->addHistory(data);
                    }
                }
            }

            if(!m_system->m_sensors.contains(key))
                m_system->addSensor(sensor);
        }
    }
}

void QKlipperConsole::machineSensorParser(QKlipperMessage *message)
{
    if(message->response().isObject() && message->response().toObject().contains("id"))
    {
        QJsonObject rootObject;
        QJsonObject sensorsObject;
        QJsonObject sensorObject = message->response().toObject();

        sensorsObject[sensorObject["id"].toString()] = sensorObject;
        rootObject["sensors"] = sensorsObject;

        message->setResponse(rootObject);
        machineSensorListParser(message);
    }
}

void QKlipperConsole::machineSensorMeasurementParser(QKlipperMessage *message)
{
    QJsonObject sensorsObject;
    sensorsObject["sensors"] = message->response();
    message->setResponse(sensorsObject);

    machineSensorListParser(message);
}

void QKlipperConsole::printerInfoParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains(QString("state")))
    {
        QString state = message->response()["state"].toString();

        if(state == QString("ready"))
            m_printer->setStatus(QKlipperPrinter::Ready);
        else if(state == QString("error"))
            m_printer->setStatus(QKlipperPrinter::Error);
        else if(state == QString("printing"))
            m_printer->setStatus(QKlipperPrinter::Printing);
        else if(state == QString("paused"))
            m_printer->setStatus(QKlipperPrinter::Paused);
        else if(state == QString("cancelled"))
            m_printer->setStatus(QKlipperPrinter::Cancelled);
    }

    if(message->response().toObject().contains(QString("config_file")))
        m_printer->setConfigFile(message->response()["config_file"].toString());

    if(message->response().toObject().contains(QString("state_message")))
        m_printer->setStatusMessage(message->response()["state_message"].toString());

    if(message->response().toObject().contains(QString("software_version")))
        m_printer->setFirmwareVersion(message->response()["software_version"].toString());

    if(message->response().toObject().contains(QString("hostname")))
        m_system->setHostname(message->response()["hostname"].toString());
}

void QKlipperConsole::printerObjectsListParser(QKlipperMessage *message)
{
    if(message->response()["objects"].isArray())
    {
        QStringList objects;
        QStringList macros;

        QJsonArray objectsArray = message->response()["objects"].toArray();
        int count = objectsArray.count();

        for(int i = 0; i < count; i++)
        {
            QString object = objectsArray.at(i).toString();

            if(object.startsWith("gcode_macro"))
                macros.append(objectsArray.at(i).toString());
            else
                objects.append(objectsArray.at(i).toString());
        }

        m_server->setAvailableObjects(objects);
        m_server->setGcodeMacroObjects(macros);
    }
}

void QKlipperConsole::printerObjectsQueryParser(QKlipperMessage *message)
{
    if(!message->response().toObject().contains("status"))
    {
        //error
        qDebug() << "Query object did not return status";
        return;
    }

    QJsonObject statusObject = message->response()["status"].toObject();

    if(statusObject.contains("mcu"))
    {
        QJsonObject mcuObject = statusObject["mcu"].toObject();
        QKlipperMCU *mcu = m_printer->mcu();

        if(mcuObject.contains("mcu_version"))
            mcu->setFirmwareVersion(mcuObject["mcu_version"].toString());

        if(mcuObject.contains("mcu_constants"))
        {
            QJsonObject constraintsObject = mcuObject["mcu_constants"].toObject();

            mcu->setHardwareVersion(constraintsObject["MCU"].toString());
            mcu->setFrequency(constraintsObject["CLOCK_FREQ"].toDouble());
        }

        if(mcuObject.contains("last_stats"))
        {
            QJsonObject statsObject = message->response()["last_stats"].toObject();

            mcu->setAwake(statsObject["mcu_awake"].toDouble());
            mcu->setBytesAvailable(statsObject["bytes_available"].toInteger());
            mcu->setBytesInvalid(statsObject["bytes_invalid"].toInteger());
            mcu->setBytesRead(statsObject["bytes_read"].toInteger());
            mcu->setBytesWritten(statsObject["bytes_write"].toInteger());
            mcu->setBytesRetransmitted(statsObject["bytes_retransmit"].toInteger());
            mcu->setBytesUpcoming(statsObject["bytes_upcoming"].toInteger());
            mcu->setDevAverage(statsObject["mcu_task_stddev"].toDouble());
            mcu->setTaskAverage(statsObject["mcu_task_avg"].toDouble());
            mcu->setSequenceRecieved(statsObject["receive_seq"].toInteger());
            mcu->setSequenceSent(statsObject["send_seq"].toInteger());
            mcu->setSequenceRetransmitted(statsObject["retransmit_seq"].toInteger());
        }
    }
}

void QKlipperConsole::printerSubscribeParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("params"))
    {
        QJsonArray paramsArray = message->response()["params"].toArray();

        for(const QJsonValue &value : paramsArray)
        {
            message->setResponse(value);
            printerSubscribeParser(message);
        }
    }

    if(message->response().toObject().count() == 2)
        message->setResponse(message->response().toObject()["status"]);

    //First parse the config file object to construct all printer details
    //    as set in the config
    if(message->response().toObject().contains(QString("configfile")))
    {
        QJsonObject configFileObject = message->response()["configfile"].toObject();
        parsePrinterConfig(configFileObject);
    }

    //Parse extruders
    for(int index = 0; true; index++)
    {
        QString extruderName = QString("extruder") + ((index > 0) ? QString::number(index) : QString(""));

        if(!message->response().toObject().contains(extruderName))
            break;

        QJsonObject status = message->response()[extruderName].toObject();
        parseHeaterStatus(m_printer->extruder(extruderName), status);

        if(status.contains("pressure_advance"))
        {
            qreal advance = status["pressure_advance"].toDouble();
            m_printer->extruder(extruderName)->setPressureAdvanceData(advance);
        }

        if(status.contains("can_extrude"))
        {
            bool canExtrude = status["can_extrude"].toBool();
            m_printer->extruder(extruderName)->setCanExtrude(canExtrude);
        }
    }

    if(message->response().toObject().contains("mcu"))
    {
        QJsonObject mcuObject = message->response().toObject()["mcu"].toObject();
        QKlipperMCU *mcu = m_printer->mcu();

        if(mcuObject.contains("mcu_version"))
            mcu->setFirmwareVersion(mcuObject["mcu_version"].toString());

        if(mcuObject.contains("mcu_constants"))
        {
            QJsonObject constraintsObject = mcuObject["mcu_constants"].toObject();

            mcu->setHardwareVersion(constraintsObject["MCU"].toString());
            mcu->setFrequency(constraintsObject["CLOCK_FREQ"].toDouble());
        }

        if(mcuObject.contains("last_stats"))
        {
            QJsonObject statsObject = message->response()["last_stats"].toObject();

            mcu->setAwake(statsObject["mcu_awake"].toDouble());
            mcu->setBytesAvailable(statsObject["bytes_available"].toInteger());
            mcu->setBytesInvalid(statsObject["bytes_invalid"].toInteger());
            mcu->setBytesRead(statsObject["bytes_read"].toInteger());
            mcu->setBytesWritten(statsObject["bytes_write"].toInteger());
            mcu->setBytesRetransmitted(statsObject["bytes_retransmit"].toInteger());
            mcu->setBytesUpcoming(statsObject["bytes_upcoming"].toInteger());
            mcu->setDevAverage(statsObject["mcu_task_stddev"].toDouble());
            mcu->setTaskAverage(statsObject["mcu_task_avg"].toDouble());
            mcu->setSequenceRecieved(statsObject["receive_seq"].toInteger());
            mcu->setSequenceSent(statsObject["send_seq"].toInteger());
            mcu->setSequenceRetransmitted(statsObject["retransmit_seq"].toInteger());
        }
    }

    //Parse fan status
    if(message->response().toObject().contains("fan"))
    {
        QJsonObject status = message->response()["fan"].toObject();
        parseFanStatus(m_printer->toolhead()->partsFan(), status);
    }

    //Parse MCU status
    if(message->response().toObject().contains("mcu"))
    {
        QJsonObject resultObject = message->response()["mcu"].toObject();
        QJsonObject mcuObject = resultObject["last_stats"].toObject();
        QKlipperMCU *mcu = m_printer->mcu();

        mcu->setAwake(mcuObject["mcu_awake"].toDouble());
        mcu->setBytesAvailable(mcuObject["bytes_available"].toInteger());
        mcu->setBytesInvalid(mcuObject["bytes_invalid"].toInteger());
        mcu->setBytesRead(mcuObject["bytes_read"].toInteger());
        mcu->setBytesWritten(mcuObject["bytes_write"].toInteger());
        mcu->setBytesRetransmitted(mcuObject["bytes_retransmit"].toInteger());
        mcu->setBytesUpcoming(mcuObject["bytes_upcoming"].toInteger());
        mcu->setDevAverage(mcuObject["mcu_task_stddev"].toDouble());
        mcu->setTaskAverage(mcuObject["mcu_task_avg"].toDouble());
        mcu->setSequenceRecieved(mcuObject["receive_seq"].toInteger());
        mcu->setSequenceSent(mcuObject["send_seq"].toInteger());
        mcu->setSequenceRetransmitted(mcuObject["retransmit_seq"].toInteger());
    }

    //Parse toolhead status
    if(message->response().toObject().contains("toolhead"))
    {
        QJsonObject toolhead = message->response()["toolhead"].toObject();

        if(toolhead.contains("axis_maximum") && toolhead["axis_maximum"].isArray())
        {
            QJsonArray positionArray = toolhead["axis_maximum"].toArray();

            if(positionArray.count() >= 4)
            {
                QKlipperPosition position;

                position["x"] = positionArray[0].toDouble();
                position["y"] = positionArray[1].toDouble();
                position["z"] = positionArray[2].toDouble();
                position["e"] = positionArray[3].toDouble();

                for(int i = 4; i < positionArray.count(); i++)
                {
                    QString name = QString("e%1").arg(QString::number(i - 4));
                    position[name] = positionArray[i].toDouble();
                }

                m_printer->toolhead()->setMaxPosition(position);
            }
        }
        if(toolhead.contains("axis_minimum") && toolhead["axis_minimum"].isArray())
        {
            QJsonArray positionArray = toolhead["axis_minimum"].toArray();

            if(positionArray.count() >= 4)
            {
                QKlipperPosition position;

                position["x"] = positionArray[0].toDouble();
                position["y"] = positionArray[1].toDouble();
                position["z"] = positionArray[2].toDouble();
                position["e"] = positionArray[3].toDouble();

                for(int i = 4; i < positionArray.count(); i++)
                {
                    QString name = QString("e%1").arg(QString::number(i - 4));
                    position[name] = positionArray[i].toDouble();
                }

                m_printer->toolhead()->setMinPosition(position);
            }
        }

        if(toolhead.contains("position") && toolhead["position"].isArray())
        {
            QJsonArray positionArray = toolhead["position"].toArray();

            if(positionArray.count() >= 4)
            {
                QKlipperPosition position;

                position["x"] = positionArray[0].toDouble();
                position["y"] = positionArray[1].toDouble();
                position["z"] = positionArray[2].toDouble();
                position["e"] = positionArray[3].toDouble();

                for(int i = 4; i < positionArray.count(); i++)
                {
                    QString name = QString("e%1").arg(QString::number(i - 4));
                    position[name] = positionArray[i].toDouble();
                }

                m_printer->toolhead()->setDestination(position);
            }
        }

        if(toolhead.contains("homed_axes") && toolhead["homed_axes"].isString())
        {
            QString homed = toolhead["homed_axes"].toString();

            m_printer->toolhead()->setXHomed(homed.contains(QString("x")));
            m_printer->toolhead()->setYHomed(homed.contains(QString("y")));
            m_printer->toolhead()->setZHomed(homed.contains(QString("z")));

            qDebug() << homed << "Homed Axes";
        }

        if(toolhead.contains("estimated_print_time"))
        {
            double time = toolhead["estimated_print_time"].toDouble();
            QDateTime date = QDateTime::currentDateTime().addSecs(time);
            m_printer->setPrintEnding(date);
        }

        if(toolhead.contains("extruder"))
        {
            //string
            QString extruder = toolhead["extruder"].toString();
            m_printer->toolhead()->setCurrentExtruderName(extruder);
        }

        if(toolhead.contains("max_accel"))
        {
            m_printer->toolhead()->setMaxAcceleration(toolhead["max_accel"].toInt());
        }
        if(toolhead.contains("max_accel_to_decel"))
        {
            m_printer->toolhead()->setMaxAccelerationToDeceleration(toolhead["max_accel_to_decel"].toInt());
        }
        if(toolhead.contains("max_velocity"))
        {
            m_printer->toolhead()->setMaxVelocity(toolhead["max_velocity"].toInt());
        }
        if(toolhead.contains("print_time"))
        {
            m_printer->setPrintTime(toolhead["print_time"].toInt());
        }
        if(toolhead.contains("square_corner_velocity"))
        {
            m_printer->toolhead()->setSquareCornerVelocity(toolhead["square_corner_velocity"].toDouble());
        }
        if(toolhead.contains("stalls"))
        {
            m_printer->toolhead()->setStalls(toolhead["stalls"].toInt());
        }
    }

    //Parse heatbed status
    if(message->response().toObject().contains("heater_bed"))
    {
        QJsonObject status = message->response()["heater_bed"].toObject();
        parseHeaterStatus(m_printer->bed(), status);
    }

    if(message->response().toObject().contains("motion_report"))
    {
        QJsonObject motion = message->response()["motion_report"].toObject();
        if(motion.contains("live_position"))
        {
            QJsonArray positionArray = motion["live_position"].toArray();
            if(positionArray.count() >= 4)
            {
                QKlipperPosition position;

                position["x"] = positionArray[0].toDouble();
                position["y"] = positionArray[1].toDouble();
                position["z"] = positionArray[2].toDouble();
                position["e"] = positionArray[3].toDouble();

                for(int i = 4; i < positionArray.count(); i++)
                {
                    QString name = QString("e%1").arg(QString::number(i - 4));
                    position[name] = positionArray[i].toDouble();
                }

                m_printer->toolhead()->setPositionData(position);
            }
        }
    }

    if(message->response().toObject().contains("print_stats"))
    {
        QJsonObject printStats = message->response()["print_stats"].toObject();

        if(printStats.contains("filename"))
            m_printer->printJob()->setFilename(printStats["filename"].toString());
        if(printStats.contains("total_duration"))
            m_printer->printJob()->setTotalDuration(printStats["total_duration"].toDouble());
        if(printStats.contains("print_duration"))
            m_printer->printJob()->setPrintDuration(printStats["print_duration"].toDouble());
        if(printStats.contains("filament_used"))
            m_printer->printJob()->setFilamentUsed(printStats["filament_used"].toDouble());
        if(printStats.contains("message"))
            m_printer->printJob()->setMessage(printStats["message"].toString());
        if(printStats.contains("info"))
        {
            QJsonObject printInfo = printStats["info"].toObject();

            if(printInfo.contains(QString("total_layer")))
                m_printer->printJob()->setTotalLayers(printInfo["total_layer"].toInt());
            if(printInfo.contains(QString("current_layer")))
                m_printer->printJob()->setCurrentLayer(printInfo["current_layer"].toInt());
        }
        if(printStats.contains("state"))
            m_printer->printJob()->setState(printStats["state"].toString());
    }

    //Parse known G-Code Macros
    foreach (QString object, m_server->gcodeMacroObjects())
    {
        if(message->response().toObject().contains(QString("gcode_macro ") + object.toLower()))
        {
            QJsonObject entryObject = message->response()[QString("gcode_macro ") + object.toLower()].toObject();

            QKlipperGCodeMacro macro;
            macro.macro = object.toUpper();

            if(entryObject.contains(QString("gcode")))
                macro.gcode = entryObject["gcode"].toString();

            if(entryObject.contains(QString("description")))
                macro.description = entryObject["description"].toString();

            //TODO: parse variable entries

            m_server->setGcodeMacro(macro);
        }
    }

    if(message->response().toObject().contains(QString("gcode")))
    {
        QJsonObject gcodeObject = message->response()["gcode"].toObject();
        QJsonObject commandsObject = gcodeObject["commands"].toObject();
        QStringList keys = commandsObject.keys();

        foreach (QString key, keys)
        {
            QJsonValue commandObject = commandsObject[key];

            QKlipperGCodeMacro macro;
            macro.macro = key.toUpper();
            macro.gcode = key.toUpper();
            macro.description = commandObject["help"].toString();

            m_server->setGcodeMacro(macro);
        }
    }

    if(message->response().toObject().contains(QString("gcode_move")))
    {
        QJsonObject gcodeObject = message->response()["gcode_move"].toObject();

        QKlipperGCodeMove move;
        move.absoluteCoordinates = gcodeObject["absolute_coordinates"].toBool();
        move.absoluteExtrude = gcodeObject["absolute_extrude"].toBool();
        move.extrusionFactor = gcodeObject["extrude_factor"].toDouble();
        move.speed = gcodeObject["speed"].toDouble();
        move.speedFactor = gcodeObject["speed_factor"].toDouble();

        //Set the extrusion factor on the current extruder
        if(m_printer->toolhead()->currentExtruder())
            m_printer->toolhead()->currentExtruder()->setExtrusionFactorData(move.extrusionFactor);

        //Gcode position
        QJsonArray gcodePositionArray = gcodeObject["gcode_position"].toArray();

        for(int i = 0; i < gcodePositionArray.count(); i++)
        {
            //0 = X, 1 = Y, 2 = Z, 3 = E, 4+ = E#
            if(i == 0)
                move.gcodePosition["x"] = gcodePositionArray[i].toDouble();
            else if(i == 1)
                move.gcodePosition["y"] = gcodePositionArray[i].toDouble();
            else if(i == 2)
                move.gcodePosition["z"] = gcodePositionArray[i].toDouble();
            else if(i == 3)
                move.gcodePosition["e"] = gcodePositionArray[i].toDouble();
            else if(i > 3)
            {
                qint16 extruderNum = (i - 3);
                QString extruderName = QString("e") + QString::number(extruderNum);
                move.gcodePosition[extruderName] = gcodePositionArray[i].toDouble();
            }
        }

        //Homing Origin position
        QJsonArray homingPositionArray = gcodeObject["homing_origin"].toArray();

        for(int i = 0; i < homingPositionArray.count(); i++)
        {
            //0 = X, 1 = Y, 2 = Z, 3 = E, 4+ = E#
            if(i == 0)
                move.homingOrigin["x"] = homingPositionArray[i].toDouble();
            else if(i == 1)
                move.homingOrigin["y"] = homingPositionArray[i].toDouble();
            else if(i == 2)
                move.homingOrigin["z"] = homingPositionArray[i].toDouble();
            else if(i == 3)
                move.homingOrigin["e"] = homingPositionArray[i].toDouble();
            else if(i > 3)
            {
                qint16 extruderNum = (i - 3);
                QString extruderName = QString("e") + QString::number(extruderNum);
                move.homingOrigin[extruderName] = homingPositionArray[i].toDouble();
            }
        }

        m_printer->setGcodeMove(move);

        //Position
        QJsonArray positionArray = gcodeObject["position"].toArray();

        for(int i = 0; i < positionArray.count(); i++)
        {
            //0 = X, 1 = Y, 2 = Z, 3 = E, 4+ = E#
            if(i == 0)
                move.position["x"] = positionArray[i].toDouble();
            else if(i == 1)
                move.position["y"] = positionArray[i].toDouble();
            else if(i == 2)
                move.position["z"] = positionArray[i].toDouble();
            else if(i == 3)
                move.position["e"] = positionArray[i].toDouble();
            else if(i > 3)
            {
                qint16 extruderNum = (i - 3);
                QString extruderName = QString("e") + QString::number(extruderNum);
                move.position[extruderName] = positionArray[i].toDouble();
            }
        }
    }

    //Parse bed mesh data
    if(message->response().toObject().contains(QString("bed_mesh")))
    {
        QKlipperPrintBedMesh *bedMesh = m_printer->bed()->bedMesh();

        QJsonObject bedMeshObject = message->response()["bed_mesh"].toObject();
        QJsonArray meshMaxArray = bedMeshObject["mesh_max"].toArray();
        QJsonArray meshMinArray = bedMeshObject["mesh_min"].toArray();

        QVector2D maximum, minimum;

        maximum.setX(meshMaxArray[0].toDouble());
        maximum.setY(meshMaxArray[1].toDouble());

        bedMesh->setMaximum(maximum);

        minimum.setX(meshMinArray[0].toDouble());
        minimum.setY(meshMinArray[1].toDouble());

        bedMesh->setMinimum(minimum);

        //Parse the 2D array of probed values
        QJsonArray probedArray = bedMeshObject["probed_matrix"].toArray();

        QList<QList<qreal>> probed(probedArray.count());

        for(int i = 0; i < probedArray.count(); i++)
        {
            QJsonArray probedEntriesArray = probedArray[i].toArray();
            probed[i].resize(probedEntriesArray.count());

            for(int e = 0; e < probedEntriesArray.count(); e++)
                probed[i][e] = probedEntriesArray[e].toDouble();
        }

        bedMesh->setProbed(probed);

        //Parse the 2D array of calculated values
        QJsonArray matrixArray = bedMeshObject["mesh_matrix"].toArray();

        QList<QList<qreal>> matrix(matrixArray.count());

        for(int i = 0; i < matrixArray.count(); i++)
        {
            QJsonArray matrixEntriesArray = matrixArray[i].toArray();
            matrix[i].resize(matrixEntriesArray.count());

            for(int e = 0; e < matrixEntriesArray.count(); e++)
                matrix[i][e] = matrixEntriesArray[e].toDouble();
        }

        bedMesh->setMatrix(matrix);

        //Parse profile data
        QJsonArray profilesArray = bedMeshObject["profiles"].toArray();
        bedMesh->setProfileName(bedMeshObject["profile_name"].toString());

        QStringList profiles;

        for(int i = 0; i < profilesArray.count(); i++)
            profiles += profilesArray[i].toString();

        bedMesh->setProfiles(profiles);
    }

    //Parse stepper motor activity
    if(message->response().toObject().contains(QString("stepper_enable")))
    {
        QJsonObject stepperEnabledObject = message->response()["stepper_enable"].toObject();
        QJsonObject steppersObject = stepperEnabledObject["steppers"].toObject();
        QStringList keys = steppersObject.keys();

        foreach(QString key, keys)
        {
            bool newMotor = false;
            QKlipperStepperMotor *motor;

            if(!m_printer->stepperMotors().contains(key))
            {
                motor = new QKlipperStepperMotor(m_printer);
                newMotor = true;
            }
            else
                motor = m_printer->stepperMotor(key);

            motor->setEnabled(steppersObject[key].toBool());

            if(newMotor)
                m_printer->setStepperMotor(key, motor);
        }
    }

    //Parse manual probe status
    if(message->response().toObject().contains(QString("manual_probe")))
    {
        QJsonObject probeObject = message->response()["manual_probe"].toObject();

        QKlipperProbeData probeData = m_printer->probeData();

        if(probeObject.contains("is_active"))
            probeData.setIsManual(probeObject["is_active"].toBool());

        if(probeObject.contains("z_position"))
            probeData.setZPosition(probeObject["z_position"].toDouble());

        if(probeObject.contains("z_position_lower"))
            probeData.setZPositionLower(probeObject["z_position_lower"].toDouble());

        if(probeObject.contains("z_position_upper"))
            probeData.setZPositionUpper(probeObject["z_position_upper"].toDouble());

        probeData.setName("Manual Probe");

        m_printer->setProbeData(probeData);
    }

    //Parse probe status
    if(message->response().toObject().contains(QString("probe")))
    {
        QJsonObject probeObject = message->response()["probe"].toObject();

        QKlipperProbeData probeData = m_printer->probeData();

        if(probeObject.contains("last_query"))
            probeData.setLastQuery(probeObject["last_query"].toBool());

        if(probeObject.contains("last_z_result"))
            probeData.setLastZResult(probeObject["last_z_result"].toDouble());

        if(probeObject.contains("name"))
            probeData.setName(probeObject["name"].toString());

        m_printer->setProbeData(probeData);
    }

    //Parse virtual SD card status
    if(message->response().toObject().contains(QString("virtual_sdcard")))
    {
        QJsonObject virtualSDObject = message->response()["virtual_sdcard"].toObject();

        QKlipperVirtualSDCard *virtualSDCard = m_system->virtualSDCard();

        if(virtualSDObject.contains("file_path"))
            virtualSDCard->setFilePath(virtualSDObject["file_path"].toString());

        if(virtualSDObject.contains("file_position"))
            virtualSDCard->setFilePosition(virtualSDObject["file_position"].toInt());

        if(virtualSDObject.contains("file_size"))
            virtualSDCard->setFileSize(virtualSDObject["file_size"].toInt());

        if(virtualSDObject.contains("is_active"))
            virtualSDCard->setIsActive(virtualSDObject["is_active"].toBool());

        if(virtualSDObject.contains("progress"))
            virtualSDCard->setValue(virtualSDObject["progress"].toDouble());
    }

    //Parse declared fan and heater objects
    foreach(QString key, m_server->availableObjects())
    {
        //Check for controller_fan objects
        if(key.startsWith(QString("controller_fan"))
            || key.startsWith(QString("heater_fan"))
            || key.startsWith(QString("fan_generic"))
            || key.startsWith(QString("temperature_fan")))
        {
            if(message->response().toObject().contains(key))
            {
                QJsonObject status = message->response()[key].toObject();
                QKlipperFan *fan = m_printer->fan(key);

                if(fan)
                    parseFanStatus(fan, status);
            }
        }

        //Check for heater_generic objects
        else if(key.startsWith(QString("heater_generic")))
        {
            if(message->response().toObject().contains(key))
            {
                QJsonObject status = message->response()[key].toObject();
                QKlipperHeater *heater = m_printer->heater(key);

                if(heater)
                    parseHeaterStatus(heater, status);
            }
        }
    }

    //Parse adjustment screw settings
    if(message->response().toObject().contains(QString("screws_tilt_adjust")))
    {
        QJsonObject screwsTiltObject = message->response()["screws_tilt_adjust"].toObject();
        QJsonObject screwsTiltResultsObject = screwsTiltObject["results"].toObject();

        if(screwsTiltResultsObject.contains(QString("error")))
            m_printer->bed()->m_adjustmentScrewsError = (screwsTiltResultsObject["error"].toBool());

        if(screwsTiltResultsObject.contains(QString("max_deviation")))
            m_printer->bed()->setAdjustmentScrewsMaxDeviation(screwsTiltResultsObject["max_deviation"].toDouble());

        QMap<QString, QKlipperAdjustmentScrew*> adjustmentScrews = m_printer->bed()->adjustmentScrews();
        bool hasResult = false;

        for(int i = 1; ; i++)
        {
            QString screwString = QString("screw") + QString::number(i);

            if(!screwsTiltResultsObject.contains(screwString))
                break;

            QJsonObject screwObject = screwsTiltResultsObject[screwString].toObject();
            QKlipperAdjustmentScrew *adjustmentScrew = adjustmentScrews[screwString];
            bool newScrew = false;

            if(adjustmentScrew == nullptr)
            {
                adjustmentScrew = new QKlipperAdjustmentScrew(m_printer->bed());
                newScrew = true;
            }

            QKlipperAdjustmentScrew::Adjustment adjustment = adjustmentScrew->adjustment();

            if(screwObject.contains(QString("adjust")))
                adjustment.amount = screwObject["adjust"].toString();

            if(screwObject.contains(QString("is_base")))
                adjustment.isBase = screwObject["is_base"].toBool();

            if(screwObject.contains(QString("sign")))
            {
                QString sign = screwObject["sign"].toString();
                adjustment.sign = sign;

                if(sign == QString("CW"))
                    adjustment.direction = QKlipperAdjustmentScrew::Adjustment::Clockwise;
                else if(sign == QString("CCW"))
                    adjustment.direction = QKlipperAdjustmentScrew::Adjustment::CounterClockwise;
            }

            if(screwObject.contains(QString("z")))
                adjustment.z = screwObject["z"].toDouble();

            adjustmentScrew->setAdjustment(adjustment);

            if(newScrew)
                m_printer->bed()->setAdjustmentScrew(screwString, adjustmentScrew);

            hasResult = true;
        }

        if(hasResult)
            m_printer->bed()->setHasAdjustmentScrewResult(true);
    }
}

void QKlipperConsole::printerQueryEndstopsParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("x"))
    {
        QString status = message->response()["x"].toString();
        m_printer->endstopStatus()["x"] = (status.toLower() == QString("triggered"));
    }

    if(message->response().toObject().contains("y"))
    {
        QString status = message->response()["y"].toString();
        m_printer->endstopStatus()["y"] = (status.toLower() == QString("triggered"));
    }

    if(message->response().toObject().contains("z"))
    {
        QString status = message->response()["z"].toString();
        m_printer->endstopStatus()["z"] = (status.toLower() == QString("triggered"));
    }
}

void QKlipperConsole::serverInfoParser(QKlipperMessage *message)
{
    if(message->response()["klippy_connected"].toBool())
    {
        if(!hasConnectionState(KlipperConnected))
        {
            /*if(m_klipperRestartTimer)
            {
                m_klipperRestartTimer->stop();
                m_klipperRestartTimer->deleteLater();

                m_klipperRestartTimer = nullptr;
            }*/

            addConnectionState(KlipperConnected);
        }
        if(message->response()["klippy_state"].toString() == "ready")
        {
        }
        else if(message->response()["klippy_state"].toString() == "error")
        {
            addConnectionState(Error);

            //emit klipperError(QString("Klipper Error"), QString("Moonraker connected, but klipper is not."));
        }
    }
    else
    {
        /*if(!m_klipperRestartTimer)
        {
            m_klipperRestartTimer = new QTimer();

            connect(m_klipperRestartTimer, SIGNAL(timeout()), this, SLOT(on_klipperRestartTimer_timeout()));

            m_klipperRestartTimer->setInterval(1000);
            m_klipperRestartTimer->setSingleShot(false);
            m_klipperRestartTimer->start();
        }*/

        removeConnectionState(KlipperConnected);
    }

    if(message->response().toObject().contains(QString("warnings")))
    {
        QJsonArray warningArray = message->response()[QString("warnings")].toArray();

        for(int i = 0; i < warningArray.count(); i++)
            m_server->setWarning(warningArray[i].toString());
    }

    if(message->response().toObject().contains(QString("components")))
    {
        QJsonArray componentsArray = message->response()[QString("components")].toArray();
        QStringList components;

        for(int i = 0; i < componentsArray.count(); i++)
            components += componentsArray[i].toString();

        m_server->setComponents(components);
    }

    if(message->response().toObject().contains(QString("failed_components")))
    {
        QJsonArray componentsArray = message->response()[QString("failed_components")].toArray();
        QStringList components;

        for(int i = 0; i < componentsArray.count(); i++)
            components += componentsArray[i].toString();

        m_server->setFailedComponents(components);
    }
}

void QKlipperConsole::serverConfigParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverFileRootsParser(QKlipperMessage *message)
{
    for(QJsonValueConstRef reference : message->response().toArray())
    {
        if(reference.isObject())
        {
            QJsonObject referenceObject = reference.toObject();
            if(referenceObject.contains(QString("name")))
            {
                if(referenceObject["name"] == QString("gcodes"))
                {
                    m_server->setGcodesLocation(referenceObject["path"].toString());
                }
                else if(referenceObject["name"] == QString("config"))
                {
                    m_server->setConfigLocation(referenceObject["path"].toString());
                }
                else if(referenceObject["name"] == QString("config_examples"))
                {

                }
                else if(referenceObject["name"] == QString("docs"))
                {

                }
            }
        }
    }
}

void QKlipperConsole::serverTemperatureStoreParser(QKlipperMessage *message)
{
    for(int index = 0; true; index++)
    {
        QString extruderName = QString("extruder") + ((index > 0) ? QString::number(index) : QString(""));

        if(message->response().toObject().contains(extruderName))
        {
            //The documentation suggests that the count for each of these arrays should be the same
            QJsonArray temperatureValuesArray = message->response()[extruderName].toObject()["temperatures"].toArray();
            QJsonArray targetValuesArray = message->response()[extruderName].toObject()["targets"].toArray();
            QJsonArray powerValuesArray = message->response()[extruderName].toObject()["targets"].toArray();

            int count = temperatureValuesArray.count();

            for(int i = 0; i < count; i++)
            {
                QKlipperTemperatureStoreValue storedValue;

                storedValue.temperature = temperatureValuesArray[i].toDouble();
                storedValue.target = targetValuesArray[i].toDouble();
                storedValue.power = powerValuesArray[i].toDouble();

                m_printer->toolhead()->extruder(extruderName)->setTemperatureStoreValue(storedValue);
            }
        }
        else
            break;
    }
}

void QKlipperConsole::serverGCodeStoreParser(QKlipperMessage *message)
{
    if(message->response()["gcode_store"].isArray())
    {
        QJsonArray gCodeEntries = message->response()["gcode_store"].toArray();
        int count = gCodeEntries.count();

        for(int i = 0; i < count; i++)
        {
            QKlipperGCodeStoreValue gCode;

            QJsonObject gCodeEntry = gCodeEntries.at(i).toObject();

            if(gCodeEntry.contains(QString("message")))
                gCode.setMessage(gCodeEntry["message"].toString());

            if(gCodeEntry.contains(QString("time")))
                gCode.setTime(gCodeEntry["time"].toDouble());

            if(gCodeEntry.contains(QString("type")))
            {
                if(gCodeEntry["type"] == QString("command"))
                    gCode.setGcodeType(QKlipperGCodeStoreValue::Command);
                else
                    gCode.setGcodeType(QKlipperGCodeStoreValue::Response);
            }

            m_server->gcodeStore()->setValue(gCode);
        }
    }
}

void QKlipperConsole::serverLogsRolloverParser(QKlipperMessage *message)
{
    if(!message->response()["failed"].toObject().isEmpty())
    {
        QJsonObject failures = message->response()["failed"].toObject();

        QJsonObject::Iterator failureIterator = failures.begin();

        while(!failureIterator->isNull())
        {
            QString application = failureIterator.key();
            QString value = failureIterator.value().toString();

            QString errorMessage = QString("Could not rollover logfile %1: %2").arg(application, value);
            QKlipperError error;
            error.setErrorString(errorMessage);
            error.setErrorTitle(message->method());
            error.setOrigin(message->method());
            error.setType(QKlipperError::Moonraker);
        }
    }
}

void QKlipperConsole::serverWebsocketIdParser(QKlipperMessage *message)
{
    if(message->response().toObject().contains("websocket_id"))
        m_server->setWebsocketId(QString::number(message->response()["websocket_id"].toInt()));
}

void QKlipperConsole::serverFilesMetadataParser(QKlipperMessage *message)
{
    QString path = QString("gcodes/%1").arg(message->param("filename").toString());

    QKlipperFile *file = m_server->file(path);
    QKlipperMetadata metadata;

    metadata.printStartTime = message->response()["print_start_time"].toDouble();
    metadata.jobId = message->response()["job_id"].toInt();
    metadata.size = message->response()["size"].toInt();
    metadata.modified = message->response()["modified"].toDouble();
    metadata.slicer = message->response()["slicer"].toString();
    metadata.slicerVersion = message->response()["slicer_version"].toString();
    metadata.layerHeight = message->response()["layer_height"].toDouble();
    metadata.firstLayerHeight = message->response()["first_layer_height"].toDouble();
    metadata.objectHeight = message->response()["object_height"].toDouble();
    metadata.filamentTotal = message->response()["filament_total"].toDouble();
    metadata.nozzleDiameter = message->response()["nozzle_diameter"].toDouble();
    metadata.filamentTotalWeight = message->response()["filament_weight_total"].toDouble();
    metadata.estimatedTime = message->response()["estimated_time"].toInt();
    metadata.firstLayerBedTemp = message->response()["first_layer_bed_temp"].toDouble();
    metadata.firstLayerExtruderTemp = message->response()["first_layer_extr_temp"].toDouble();
    metadata.gcodeStartByte = message->response()["gcode_start_byte"].toInt();
    metadata.gcodeEndByte = message->response()["gcode_end_byte"].toInt();
    metadata.filename = message->response()["filename"].toString();
    metadata.uuid = message->response()["uuid"].toString();
    metadata.filamentName = message->response()["filament_name"].toString();

    QJsonArray thumbnailArray = message->response()["thumbnails"].toArray();

    for(int i = 0; i < thumbnailArray.count(); i++)
    {
        QKlipperThumbnail thumbnail;

        thumbnail.height = message->response()["height"].toInt();
        thumbnail.width = message->response()["width"].toInt();
        thumbnail.size = message->response()["size"].toInt();
        thumbnail.relativePath = message->response()["relative_path"].toString();

        metadata.thumbnails.append(thumbnail);
    }

    if(file)
        file->setMetadata(metadata);
}

void QKlipperConsole::serverFilesDeleteParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverFilesMoveParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverFilesCopyParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverFilesListParser(QKlipperMessage *message)
{
    QJsonObject rootInfo = message->response()["root_info"].toObject();
    QJsonArray files = message->response()["files"].toArray();
    QJsonArray directories = message->response()["dirs"].toArray();
    QKlipperFileList fileList;

    if(message->response().toObject().contains(QString("disk_usage")))
    {
        QJsonObject driveUsage = message->response().toObject()["disk_usage"].toObject();
        m_system->setDriveUsage(driveUsage["used"].toInteger());
        m_system->setDriveCapacity(driveUsage["total"].toInteger());
        m_system->setDriveFree(driveUsage["free"].toInteger());
    }

    QString directory = message->param("path").toString();
    QString root = rootInfo["name"].toString();

    if(!root.endsWith("/"))
        root += "/";

    if(!directory.endsWith("/"))
        directory += "/";

    if(directory.startsWith(root))
        directory.remove(0, root.length());

    for(QJsonValueConstRef directoryRef : directories)
    {
        if(directoryRef.isObject())
        {
            QJsonObject directoryObject = directoryRef.toObject();

            QKlipperFile *directoryListing = m_server->file(directory);

            if(!directoryListing)
                directoryListing = new QKlipperFile(m_server);

            QString directoryName = directoryObject["dirname"].toString();

            directoryListing->setFilename(directoryName);
            directoryListing->setRoot(root);
            directoryListing->setPath(directory);
            directoryListing->setFileSize(directoryObject["size"].toDouble());
            directoryListing->setDateModified(directoryObject["modified"].toDouble());
            directoryListing->setFileType(QKlipperFile::Directory);

            fileList += directoryListing;
        }
    }

    for(QJsonValueConstRef fileRef : files)
    {
        if(fileRef.isObject())
        {
            QJsonObject directoryObject = fileRef.toObject();

            QString filename = directoryObject["filename"].toString();

            QKlipperFile *file = m_server->file(directory + filename);

            if(!file)
                file = new QKlipperFile(m_server);

            file->setFilename(filename);
            file->setRoot(root);
            file->setPath(directory);
            file->setFileSize(directoryObject["size"].toDouble());
            file->setDateModified(directoryObject["modified"].toDouble());
            file->setFileType(QKlipperFile::Directory);

            if(root == QString("gcodes/"))
                file->setFileType(QKlipperFile::GCode);
            else if(root == QString("config/"))
                file->setFileType(QKlipperFile::Config);

            fileList += file;
        }
    }

    m_server->setFileList(root + directory, fileList);
}

void QKlipperConsole::serverDirectoryPostParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverDirectoryDeleteParser(QKlipperMessage *message)
{
    Q_UNUSED(message);
}

void QKlipperConsole::serverWebcamListParser(QKlipperMessage *message)
{
    QJsonArray webcamArray = message->response()["webcams"].toArray();

    QKlipperWebcamList list;

    for(int i = 0; i < webcamArray.count(); i++)
    {
        QJsonObject webcamObject = webcamArray[i].toObject();
        QKlipperWebcam webcam;

        webcam.setName(webcamObject["name"].toString());
        webcam.setLocation(webcamObject["location"].toString());
        webcam.setService(webcamObject["service"].toString());
        webcam.setEnabled(webcamObject["enabled"].toBool());
        webcam.setIconString(webcamObject["icon"].toString());
        webcam.setTargetFps(webcamObject["target_fps"].toInt());
        webcam.setTargetFpsIdle(webcamObject["target_fps_idle"].toInt());
        webcam.setStreamUrl(webcamObject["stream_url"].toString());
        webcam.setSnapshotUrl(webcamObject["snapshot_url"].toString());
        webcam.setFlipHorizontal(webcamObject["flip_horizontal"].toBool());
        webcam.setFlipVertical(webcamObject["flip_vertical"].toBool());
        webcam.setRotation(webcamObject["aspect_ratio"].toInt());
        webcam.setAspectRatio(webcamObject["aspect_ratio"].toString());
        webcam.setSource(webcamObject["source"].toString());
        webcam.setUid(webcamObject["uid"].toString());

        list.append(webcam);
    }

    m_system->setWebcams(list);
}

void QKlipperConsole::serverWebcamCreateParser(QKlipperMessage *message)
{
    QJsonObject webcamObject = message->response()["webcam"].toObject();
    QKlipperWebcam webcam;
    QKlipperWebcamList list = m_system->webcams();

    webcam.setName(webcamObject["name"].toString());
    webcam.setLocation(webcamObject["location"].toString());
    webcam.setService(webcamObject["service"].toString());
    webcam.setEnabled(webcamObject["enabled"].toBool());
    webcam.setIconString(webcamObject["icon"].toString());
    webcam.setTargetFps(webcamObject["target_fps"].toInt());
    webcam.setTargetFpsIdle(webcamObject["target_fps_idle"].toInt());
    webcam.setStreamUrl(webcamObject["stream_url"].toString());
    webcam.setSnapshotUrl(webcamObject["snapshot_url"].toString());
    webcam.setFlipHorizontal(webcamObject["flip_horizontal"].toBool());
    webcam.setFlipVertical(webcamObject["flip_vertical"].toBool());
    webcam.setRotation(webcamObject["aspect_ratio"].toInt());
    webcam.setAspectRatio(webcamObject["aspect_ratio"].toString());
    webcam.setSource(webcamObject["source"].toString());
    webcam.setUid(webcamObject["uid"].toString());

    if(list.contains(webcam))
        return;

    list.append(webcam);
    m_system->setWebcams(list);
}

void QKlipperConsole::serverWebcamDeleteParser(QKlipperMessage *message)
{
    QJsonObject webcamObject = message->response()["webcam"].toObject();
    QKlipperWebcam webcam;
    QKlipperWebcamList list = m_system->webcams();

    webcam.setName(webcamObject["name"].toString());
    webcam.setLocation(webcamObject["location"].toString());
    webcam.setService(webcamObject["service"].toString());
    webcam.setEnabled(webcamObject["enabled"].toBool());
    webcam.setIconString(webcamObject["icon"].toString());
    webcam.setTargetFps(webcamObject["target_fps"].toInt());
    webcam.setTargetFpsIdle(webcamObject["target_fps_idle"].toInt());
    webcam.setStreamUrl(webcamObject["stream_url"].toString());
    webcam.setSnapshotUrl(webcamObject["snapshot_url"].toString());
    webcam.setFlipHorizontal(webcamObject["flip_horizontal"].toBool());
    webcam.setFlipVertical(webcamObject["flip_vertical"].toBool());
    webcam.setRotation(webcamObject["aspect_ratio"].toInt());
    webcam.setAspectRatio(webcamObject["aspect_ratio"].toString());
    webcam.setSource(webcamObject["source"].toString());
    webcam.setUid(webcamObject["uid"].toString());

    if(!list.contains(webcam))
        return;

    list.removeAll(webcam);
    m_system->setWebcams(list);
}

void QKlipperConsole::serverAnnouncementsListParser(QKlipperMessage *message)
{
    QJsonArray announcementArray = message->response()["entries"].toArray();
    QKlipperAnnouncementList list;

    for(int i = 0; i < announcementArray.count(); i++)
    {
        QJsonObject announcementObject = announcementArray[i].toObject();
        QKlipperAnnouncement announcement;

        announcement.setEntryId(announcementObject["entry_id"].toString());
        announcement.setUrl(announcementObject["url"].toString());
        announcement.setTitle(announcementObject["title"].toString());
        announcement.setDescription(announcementObject["description"].toString());
        announcement.setPriority(announcementObject["priority"].toString());
        announcement.setDate(announcementObject["date"].toDouble());
        announcement.setIsDismissed(announcementObject["dismissed"].toBool());
        announcement.setDateDismissed(announcementObject["date_dismissed"].toDouble());
        announcement.setDateDismissedWake(announcementObject["dismiss_wake"].toDouble());
        announcement.setSource(announcementObject["source"].toString());
        announcement.setFeed(announcementObject["feed"].toString());

        list.append(announcement);
    }

    m_server->setAnnouncements(list);
}

void QKlipperConsole::serverAnnouncementsUpdateParser(QKlipperMessage *message)
{
    QJsonArray announcementArray = message->response()["entries"].toArray();
    QKlipperAnnouncementList list;

    for(int i = 0; i < announcementArray.count(); i++)
    {
        QJsonObject announcementObject = announcementArray[i].toObject();
        QKlipperAnnouncement announcement;

        announcement.setEntryId(announcementObject["entry_id"].toString());
        announcement.setUrl(announcementObject["url"].toString());
        announcement.setTitle(announcementObject["title"].toString());
        announcement.setDescription(announcementObject["description"].toString());
        announcement.setPriority(announcementObject["priority"].toString());
        announcement.setDate(announcementObject["date"].toDouble());
        announcement.setIsDismissed(announcementObject["dismissed"].toBool());
        announcement.setDateDismissed(announcementObject["date_dismissed"].toDouble());
        announcement.setDateDismissedWake(announcementObject["dismiss_wake"].toDouble());
        announcement.setSource(announcementObject["source"].toString());
        announcement.setFeed(announcementObject["feed"].toString());

        list.append(announcement);
    }

    m_server->setAnnouncements(list);
}

void QKlipperConsole::serverAnnouncementDismissParser(QKlipperMessage *message)
{
    QString entryId = message->response()["entry_id"].toString();
    QKlipperAnnouncementList list = m_server->announcements();
    bool changed = false;

    for(int i = 0; i < list.count(); i++)
    {
        QKlipperAnnouncement announcement = list.at(i);

        if(entryId == announcement.entryId())
        {
            list.remove(i);
            changed = true;
            break;
        }
    }

    if(changed)
        m_server->setAnnouncements(list);
}

void QKlipperConsole::serverJobQueueStatusParser(QKlipperMessage *message)
{
    QJsonArray jobArray = message->response()["queued_jobs"].toArray();

    QKlipperJobQueue *queue = m_server->jobQueue();

    for(int i = 0; i < jobArray.count(); i++)
    {
        QJsonObject jobObject = jobArray[i].toObject();

        bool newJob = false;
        QKlipperPrintJob *job = m_server->jobQueue()->job(jobObject["job_id"].toString());

        if(!job)
        {
            job = new QKlipperPrintJob(m_server->jobQueue());
            newJob = true;
        }

        job->setFilename(jobObject["filename"].toString());
        job->setId(jobObject["job_id"].toString());
        job->setTimeAdded(jobObject["time_added"].toDouble());
        job->setTimeInQueue(jobObject["time_in_queue"].toDouble());

        if(newJob)
            m_server->jobQueue()->addJob(job);
    }

    if(message->response()["queue_state"].toString() == QString("loading"))
        queue->setState(QKlipperJobQueue::Loading);

    else if(message->response()["queue_state"].toString() == QString("paused"))
        queue->setState(QKlipperJobQueue::Paused);

    else if(message->response()["queue_state"].toString() == QString("starting"))
        queue->setState(QKlipperJobQueue::Starting);

    else if(message->response()["queue_state"].toString() == QString("ready"))
        queue->setState(QKlipperJobQueue::Ready);

    else
        queue->setState(QKlipperJobQueue::Error);
}

void QKlipperConsole::serverJobQueueStartParser(QKlipperMessage *message)
{
    serverJobQueueStatusParser(message);
}

void QKlipperConsole::serverJobQueuePauseParser(QKlipperMessage *message)
{
    serverJobQueueStatusParser(message);
}

void QKlipperConsole::serverJobQueueJumpParser(QKlipperMessage *message)
{
    serverJobQueueStatusParser(message);
}

void QKlipperConsole::serverJobQueueAddParser(QKlipperMessage *message)
{
    serverJobQueueStatusParser(message);
}

void QKlipperConsole::serverJobQueueDeleteParser(QKlipperMessage *message)
{
    QJsonArray jobArray = message->response()["queued_jobs"].toArray();
    QKlipperJobQueue *queue = m_server->jobQueue();
    QStringList keys;

    foreach(QKlipperPrintJob *job, m_server->jobQueue()->queue())
        keys += job->id();

    for(int i = 0; i < jobArray.count(); i++)
    {
        QJsonObject jobObject = jobArray[i].toObject();

        bool newJob = false;
        QKlipperPrintJob *job = m_server->jobQueue()->job(jobObject["job_id"].toString());

        if(!job)
        {
            job = new QKlipperPrintJob(m_server->jobQueue());
            newJob = true;
        }

        job->setFilename(jobObject["filename"].toString());
        job->setId(jobObject["job_id"].toString());
        job->setTimeAdded(jobObject["time_added"].toDouble());
        job->setTimeInQueue(jobObject["time_in_queue"].toDouble());

        keys.removeAll(jobObject["job_id"].toString());

        if(newJob)
            m_server->jobQueue()->addJob(job);
    }

    if(message->response()["queue_state"].toString() == QString("loading"))
        queue->setState(QKlipperJobQueue::Loading);

    else if(message->response()["queue_state"].toString() == QString("paused"))
        queue->setState(QKlipperJobQueue::Paused);

    else if(message->response()["queue_state"].toString() == QString("starting"))
        queue->setState(QKlipperJobQueue::Starting);

    else if(message->response()["queue_state"].toString() == QString("ready"))
        queue->setState(QKlipperJobQueue::Ready);

    else
        queue->setState(QKlipperJobQueue::Error);

    //remaining keys are the ones removed from the queue
    foreach(QString key, keys)
    {
        QKlipperPrintJob *job = m_server->jobQueue()->job(key);

        if(job)
        {
            m_server->jobQueue()->removeJob(key);
            job->deleteLater();
        }
    }
}

bool QKlipperConsole::parseFanSettings(QKlipperFan *fan, QJsonObject settings)
{
    if(settings.contains("pin"))
        fan->setPin(settings["pin"].toString());

    if(settings.contains("off_below"))
        fan->setOffBelow(settings["off_below"].toDouble());

    if(settings.contains("kick_start_time"))
        fan->setKickStartTime(settings["kick_start_time"].toDouble());

    if(settings.contains("cycle_time"))
        fan->setCycleTime(settings["cycle_time"].toDouble());

    if(settings.contains("hardware_pwm"))
        fan->setHardwarePin(settings["hardware_pwm"].toBool());

    if(settings.contains("shutdown_speed"))
        fan->setShutdownSpeed(settings["shutdown_speed"].toDouble());

    if(settings.contains("max_power"))
        fan->setMaxPower(settings["max_power"].toDouble());

    if(settings.contains("tachometer_pin"))
    {
        QKlipperFan::Tachometer tachometer = fan->tachometer();
        tachometer.pin = settings["tachometer_pin"].toString();

        if(settings.contains("tachometer_ppr"))
            tachometer.pulsesPerRotation = settings["tachometer_ppr"].toInt();

        if(settings.contains("tachometer_poll_interval"))
            tachometer.pollInterval = settings["tachometer_poll_interval"].toDouble();

        tachometer.enabled = true;

        fan->setTachometer(tachometer);
    }

    return true;
}

bool QKlipperConsole::parseFanStatus(QKlipperFan *fan, QJsonObject status)
{
    if(status.contains(QString("rpm")))
        fan->setRpmData(status["rpm"].toDouble());

    if(status.contains(QString("speed")))
        fan->setSpeedData(status["speed"].toDouble());

    return true;
}

bool QKlipperConsole::parseHeaterSettings(QKlipperHeater *heater, QJsonObject settings)
{
    if(settings.contains("control"))
    {
        QString control = settings["control"].toString();
        heater->setControl(control);
    }

    if(settings.contains(QString("gcode_id")))
        heater->setGcodeId(settings["gcode_id"].toString());

    if(settings.contains("heater_pin"))
    {
        QString pin = settings["heater_pin"].toString();
        heater->setHeaterPin(pin);
    }

    if(settings.contains("inline_resistor"))
    {
        qreal resistor = settings["inline_resistor"].toDouble();
        heater->setInlineResistor(resistor);
    }

    if(settings.contains("max_power"))
    {
        qreal power = settings["max_power"].toDouble();
        heater->setMaxPower(power);
    }

    if(settings.contains("max_temp"))
    {
        qreal temp = settings["max_temp"].toDouble();
        heater->setMaxTemp(temp);
    }

    if(settings.contains("min_temp"))
    {
        qreal temp = settings["min_temp"].toDouble();
        heater->setMinTemp(temp);
    }

    if(settings.contains("pid_kd"))
    {
        qreal pid = settings["pid_kd"].toDouble();
        heater->setPidKD(pid);
    }

    if(settings.contains("pid_ki"))
    {
        qreal pid = settings["pid_ki"].toDouble();
        heater->setPidKI(pid);
    }

    if(settings.contains("pid_kp"))
    {
        qreal pid = settings["pid_kp"].toDouble();
        heater->setPidKP(pid);
    }

    if(settings.contains("pullup_resistor"))
    {
        qreal resistor = settings["pullup_resistor"].toDouble();
        heater->setPullupResistor(resistor);
    }

    if(settings.contains("pwm_cycle_time"))
    {
        qreal cycle = settings["pwm_cycle_time"].toDouble();
        heater->setPwmCycleTime(cycle);
    }

    if(settings.contains("sensor_pin"))
    {
        QString pin = settings["sensor_pin"].toString();
        heater->setSensorPin(pin);
    }

    if(settings.contains("sensor_type"))
    {
        QString type = settings["sensor_type"].toString();
        heater->setSensorType(type);
    }

    if(m_printer->powerProfile().contains(heater->name()))
        heater->setMaxWatts(m_printer->powerProfile()[heater->name()]);

    return true;
}

bool QKlipperConsole::parseHeaterStatus(QKlipperHeater *heater, QJsonObject settings)
{
    if(settings.contains("temperature"))
        heater->setCurrentTemp(settings["temperature"].toDouble());

    if(settings.contains("target"))
        heater->setTargetTempData(settings["target"].toDouble());

    if(settings.contains("power"))
        heater->setPower(settings["power"].toDouble());

    return true;
}

bool QKlipperConsole::parsePrinterConfig(QJsonObject config)
{
    //Most of the config file is useless at the beta stage
    //    a lot of these will be blank
    if(config.contains(QString("config")))
    {
        QJsonObject configObject = config["config"].toObject();
    }
    if(config.contains(QString("settings")))
    {
        QJsonObject settingsObject = config["settings"].toObject();

        if(settingsObject.contains(QString("bed_mesh")))
        {
            QJsonObject bedMeshObject = settingsObject["bed_mesh"].toObject();

            QKlipperPrintBedMesh *bedMesh = m_printer->bed()->bedMesh();

            if(bedMeshObject.contains("adaptive_margin"))
                bedMesh->setAdaptiveMargin(bedMeshObject["adaptive_margin"].toDouble());

            if(bedMeshObject.contains("algorithm"))
                bedMesh->setAlgorithm(bedMeshObject["algorithm"].toString());

            if(bedMeshObject.contains("bicubic_tension"))
                bedMesh->setTension(bedMeshObject["bicubic_tension"].toDouble());

            if(bedMeshObject.contains("fade_end"))
                bedMesh->setFadeEnd(bedMeshObject["fade_end"].toDouble());

            if(bedMeshObject.contains("fade_start"))
                bedMesh->setFadeStart(bedMeshObject["fade_start"].toDouble());

            if(bedMeshObject.contains("fade_target"))
                bedMesh->setFadeTarget(bedMeshObject["fade_target"].toDouble());

            if(bedMeshObject.contains("horizontal_move_z"))
                bedMesh->setHorizontalMoveZ(bedMeshObject["horizontal_move_z"].toDouble());

            if(bedMeshObject.contains("move_check_distance"))
                bedMesh->setMoveCheckDistance(bedMeshObject["move_check_distance"].toDouble());

            if(bedMeshObject.contains("speed"))
                bedMesh->setSpeed(bedMeshObject["speed"].toDouble());

            if(bedMeshObject.contains("split_delta_z"))
                bedMesh->setSplitDeltaZ(bedMeshObject["split_delta_z"].toDouble());

            //Parse limits
            QJsonArray meshMaxArray = bedMeshObject["mesh_max"].toArray();
            QJsonArray meshMinArray = bedMeshObject["mesh_min"].toArray();
            QJsonArray probeCountArray = bedMeshObject["probe_count"].toArray();

            QVector2D minimum, maximum, probeCount;

            maximum.setX(meshMaxArray[0].toDouble());
            maximum.setY(meshMaxArray[1].toDouble());

            minimum.setX(meshMinArray[0].toDouble());
            minimum.setY(meshMinArray[1].toDouble());

            probeCount.setX(probeCountArray[0].toDouble());
            probeCount.setY(probeCountArray[1].toDouble());

            bedMesh->setProbeCount(probeCount);
            bedMesh->setMaximum(maximum);
            bedMesh->setMinimum(minimum);
        }

        //Parse printer settings
        if(settingsObject.contains(QString("printer")))
        {
            QJsonObject printerObject = settingsObject["printer"].toObject();

            if(printerObject.contains("kinematics"))
                m_printer->setKinematics(printerObject["kinematics"].toString());

            if(printerObject.contains("max_accel"))
                m_printer->setMaxAcceleration(printerObject["max_accel"].toDouble());

            if(printerObject.contains("max_velocity"))
                m_printer->setMaxVelocity(printerObject["max_velocity"].toDouble());

            if(printerObject.contains("max_z_accel"))
                m_printer->setMaxZAcceleration(printerObject["max_z_accel"].toDouble());

            if(printerObject.contains("max_z_velocity"))
                m_printer->setMaxZVelocity(printerObject["max_z_velocity"].toDouble());

            if(printerObject.contains("square_corner_velocity"))
                m_printer->setSquareCornerVelocity(printerObject["square_corner_velocity"].toDouble());

            if(printerObject.contains("minimum_cruise_ratio"))
                m_printer->setMinimumCruiseRatio(printerObject["minimum_cruise_ratio"].toDouble());
        }

        //Parse MCU settings
        if(settingsObject.contains(QString("mcu")))
        {
            QJsonObject mcuObject = settingsObject["mcu"].toObject();
            QKlipperMCU *mcu = m_printer->mcu();

            if(mcuObject.contains("serial"))
                mcu->setSerial(mcuObject["serial"].toString());

            if(mcuObject.contains("baud"))
                mcu->setBaudRate(mcuObject["baud"].toInt());

            if(mcuObject.contains("max_stepper_error"))
                mcu->setMaxStepperError(mcuObject["max_stepper_error"].toDouble());
        }

        //Parse Safe Z Home settings
        if(settingsObject.contains(QString("safe_z_home")))
        {
            QJsonObject safeZObject = settingsObject["safe_z_home"].toObject();
            QKlipperSafeZHome zHome = m_printer->safeZHome();

            if(safeZObject.contains(QString("home_xy_position")))
            {
                QJsonArray homePositionArray = safeZObject["home_xy_position"].toArray();

                if(homePositionArray.count() >= 2)
                {
                    zHome.setHomeXPosition(homePositionArray[0].toDouble());
                    zHome.setHomeYPosition(homePositionArray[1].toDouble());
                }
            }

            zHome.setMoveToPrevious(safeZObject["move_to_previous"].toBool());

            zHome.setSpeed(safeZObject["speed"].toDouble());
            zHome.setZHopSpeed(safeZObject["z_hop"].toDouble());
            zHome.setZHop(safeZObject["z_hop_speed"].toDouble());
        }

        //Parse adjustment screw settings
        if(settingsObject.contains(QString("screws_tilt_adjust")))
        {
            QJsonObject screwsTiltObject = settingsObject["screws_tilt_adjust"].toObject();

            QString thread = screwsTiltObject["screw_thread"].toString();
            qreal speed = screwsTiltObject["speed"].toDouble();

            for(int i = 1; ; i++)
            {
                QString screwString = QString("screw") + QString::number(i);
                QString screwNameString = screwString + QString("_name");

                if(!screwsTiltObject.contains(screwString))
                    break;

                QJsonArray screwArray = screwsTiltObject[screwString].toArray();

                QKlipperAdjustmentScrew *adjustmentScrew = m_printer->bed()->adjustmentScrew(screwString);

                bool newScrew = false;

                if(!adjustmentScrew)
                {
                    newScrew = true;
                    adjustmentScrew = new QKlipperAdjustmentScrew(m_printer->bed());
                }


                adjustmentScrew->setName(screwsTiltObject[screwNameString].toString());

                qreal tilt0 = screwArray[0].toDouble();
                qreal tilt1 = screwArray[1].toDouble();

                adjustmentScrew->setTiltAdjust(QPair<qreal,qreal>(tilt0, tilt1));
                adjustmentScrew->setSpeed(speed);
                adjustmentScrew->setThread(thread);

                if(newScrew)
                    m_printer->bed()->setAdjustmentScrew(screwString, adjustmentScrew);
            }
        }

        if(settingsObject.contains(QString("heater_bed")))
        {
            QJsonObject settings = settingsObject[QString("heater_bed")].toObject();

            parseHeaterSettings(m_printer->bed(), settings);
            m_printer->bed()->setType(QKlipperPrintBed::Heated);
        }

        //Parse extruders settings
        for(int index = 0; true; index++)
        {
            QString extruderName = QString("extruder") + ((index > 0) ? QString::number(index) : QString(""));

            if(!settingsObject.contains(extruderName))
                break;

            bool newExtruder = false;
            QKlipperExtruder *extruder;

            if(m_printer->extruder(extruderName))
                extruder = m_printer->extruder(extruderName);
            else
            {
                extruder = new QKlipperExtruder(m_printer);
                newExtruder = true;
            }

            extruder->setWatts(m_printer->powerProfile()[extruderName]);
            extruder->setName(extruderName);

            QJsonObject extruderObject = settingsObject[extruderName].toObject();
            parseHeaterSettings(extruder, extruderObject);

            if(extruderObject.contains("dir_pin"))
            {
                QString pin = extruderObject["dir_pin"].toString();
                extruder->setDirPin(pin);
            }

            if(extruderObject.contains("enable_pin"))
            {
                QString pin = extruderObject["enable_pin"].toString();
                extruder->setEnablePin(pin);
            }

            if(extruderObject.contains("filament_diameter"))
            {
                qreal diameter = extruderObject["filament_diameter"].toDouble();
                extruder->setFilamentDiameter(diameter);
            }

            if(extruderObject.contains("full_steps_per_rotation"))
            {
                qint32 steps = extruderObject["full_steps_per_rotation"].toInt();
                extruder->setFullStepsPerRotation(steps);
            }

            if(extruderObject.contains("gear_ratio"))
            {
                QJsonArray ratioArray = extruderObject["gear_ratio"].toArray();

                if(ratioArray.count() >= 2)
                {
                    QKlipperGearRatio ratio
                        (
                            ratioArray[0].toInt(),
                            ratioArray[1].toInt()
                            );

                    extruder->setGearRatio(ratio);
                }
            }

            if(extruderObject.contains("inline_resistor"))
            {
                qreal resistor = extruderObject["inline_resistor"].toDouble();
                extruder->setInlineResistor(resistor);
            }

            if(extruderObject.contains("instantaneous_corner_velocity"))
            {
                qreal velocity = extruderObject["instantaneous_corner_velocity"].toDouble();
                extruder->setInstantCornerVelocity(velocity);
            }

            if(extruderObject.contains("max_extrude_cross_section"))
            {
                qreal velocity = extruderObject["max_extrude_cross_section"].toDouble();
                extruder->setMaxExtrudeCrossSection(velocity);
            }

            if(extruderObject.contains("max_extrude_only_accel"))
            {
                qreal acceleration = extruderObject["max_extrude_only_accel"].toDouble();
                extruder->setMaxExtrudeOnlyAcceleration(acceleration);
            }

            if(extruderObject.contains("max_extrude_only_distance"))
            {
                qreal distance = extruderObject["max_extrude_only_distance"].toDouble();
                extruder->setMaxExtrudeOnlyDistance(distance);
            }

            if(extruderObject.contains("max_extrude_only_velocity"))
            {
                qreal velocity = extruderObject["max_extrude_only_velocity"].toDouble();
                extruder->setMaxExtrudeOnlyVelocity(velocity);
            }

            if(extruderObject.contains("microsteps"))
            {
                qint32 microsteps = extruderObject["microsteps"].toInt();
                extruder->setMicrosteps(microsteps);
            }

            if(extruderObject.contains("nozzle_diameter"))
            {
                qreal diameter = extruderObject["nozzle_diameter"].toDouble();
                extruder->setNozzleDiameter(diameter);
            }

            if(extruderObject.contains("pressure_advance"))
            {
                qreal advance = extruderObject["pressure_advance"].toDouble();
                extruder->setPressureAdvanceData(advance);
            }

            if(extruderObject.contains("pressure_advance_smooth_time"))
            {
                qreal smoothing = extruderObject["pressure_advance_smooth_time"].toDouble();
                extruder->setPressureAdvanceSmoothTimeData(smoothing);
            }

            if(extruderObject.contains("pullup_resistor"))
            {
                qreal resistor = extruderObject["pullup_resistor"].toDouble();
                extruder->setPullupResistor(resistor);
            }

            if(extruderObject.contains("rotation_distance"))
            {
                qreal distance = extruderObject["rotation_distance"].toDouble();
                extruder->setRotationDistance(distance);
            }

            if(extruderObject.contains("step_pin"))
            {
                QString pin = extruderObject["step_pin"].toString();
                extruder->setStepPin(pin);
            }

            if(newExtruder)
                m_printer->setExtruder(extruderName, extruder);
        }

        //Parse parts fan settings
        if(settingsObject.contains("fan"))
        {
            QJsonObject settings = settingsObject["fan"].toObject();
            parseFanSettings(m_printer->toolhead()->partsFan(), settings);

            m_printer->toolhead()->partsFan()->setIsControllable(true);
        }

        //Parse declared fan settings
        foreach(QString key, m_server->availableObjects())
        {
            //Check for heater_fan objects
            if(key.startsWith(QString("heater_fan")))
            {
                if(settingsObject.contains(key))
                {
                    bool newFan = false;
                    QKlipperHeaterFan *fan = qobject_cast<QKlipperHeaterFan*>(m_printer->fan(key));

                    if(!fan)
                    {
                        fan = new QKlipperHeaterFan(m_printer);
                        fan->setNameData(key);
                        newFan = true;
                    }

                    QJsonObject settings = settingsObject[key].toObject();
                    parseFanSettings(fan, settings);

                    if(settings.contains(QString("heater")))
                    {
                        QJsonArray heatersArray = settings["heater"].toArray();
                        QStringList heaters;

                        for(int i = 0; i < heatersArray.count(); i++)
                        {
                            QString heater = heatersArray[i].toString();
                            heaters += heater;

                            if(heater.toLower().startsWith("extruder"))
                            {
                                QKlipperExtruder *extruder = m_printer->extruder(heater);

                                if(extruder)
                                    extruder->setFan(m_printer->fan(key));
                            }
                        }

                        fan->setHeater(heaters.join(','));
                    }

                    if(settings.contains("fan_speed"))
                        fan->setFanSpeed(settings["fan_speed"].toDouble());

                    if(settings.contains("heater_temp"))
                        fan->setHeaterTemperature(settings["heater_temp"].toDouble());

                    if(newFan)
                        m_printer->setFan(key, fan);
                }
            }

            //Check for controller_fan objects
            else if(key.startsWith(QString("controller_fan")))
            {
                QJsonObject settings = settingsObject[key].toObject();

                QKlipperControllerFan *fan = qobject_cast<QKlipperControllerFan*>(m_printer->fan(key));
                bool newFan = false;

                if(!fan)
                {
                    newFan = true;
                    fan = new QKlipperControllerFan(m_printer);
                    fan->setNameData(key);
                }

                parseFanSettings(fan, settings);

                if(settings.contains(QString("heater")))
                {
                    QJsonArray heatersArray = settings["heater"].toArray();
                    QStringList heaters;

                    for(int i = 0; i < heatersArray.count(); i++)
                    {
                        QString heater = heatersArray[i].toString();
                        heaters += heater;

                        if(heater.toLower().startsWith("extruder"))
                        {
                            QKlipperExtruder *extruder = m_printer->extruder(heater);

                            if(extruder)
                                extruder->setFan(m_printer->fan(key));
                        }
                    }

                    fan->setHeater(heaters.join(','));
                }

                if(settings.contains(QString("stepper")))
                {
                    QJsonArray steppersArray = settings["stepper"].toArray();
                    QStringList steppers;

                    for(int i = 0; i < steppersArray.count(); i++)
                    {
                        steppers += steppersArray[i].toString();
                    }

                    fan->setStepper(steppers.join(','));
                }

                if(settings.contains("fan_speed"))
                    fan->setFanSpeed(settings["fan_speed"].toDouble());

                if(settings.contains("idle_timeout"))
                    fan->setIdleTimeout(settings["idle_timeout"].toDouble());

                if(settings.contains("idle_speed"))
                    fan->setIdleSpeed(settings["idle_speed"].toDouble());

                if(newFan)
                    m_printer->setFan(fan->name(), fan);
            }

            //Check for fan_generic objects
            else if(key.startsWith(QString("fan_generic")))
            {
                QJsonObject settings = settingsObject[key].toObject();

                QKlipperFan *fan = m_printer->fan(key);
                bool newFan = false;

                if(!fan)
                {
                    newFan = true;
                    fan = new QKlipperFan(m_printer);
                    fan->setNameData(key);
                    fan->setIsControllable(true);
                }

                parseFanSettings(fan, settings);

                if(newFan)
                    m_printer->setFan(fan->name(), fan);
            }

            else if(key.startsWith(QString("temperature_fan")))
            {
                QJsonObject settings = settingsObject[key].toObject();

                QKlipperTemperatureFan *fan = qobject_cast<QKlipperTemperatureFan*>(m_printer->fan(key));
                bool newFan = false;

                if(!fan)
                {
                    newFan = true;
                    fan = new QKlipperTemperatureFan(m_printer);
                    fan->setNameData(key);
                }

                parseFanSettings(fan, settings);

                if(settings.contains(QString("enable_pin")))
                    fan->setEnablePin(settings["enable_pin"].toString());

                if(settings.contains(QString("sensor_type")))
                    fan->setSensorType(settings["sensor_type"].toString());

                if(settings.contains(QString("sensor_pin")))
                    fan->setSensorPin(settings["sensor_pin"].toString());

                if(settings.contains(QString("gcode_id")))
                    fan->setGcodeId(settings["gcode_id"].toString());

                if(settings.contains(QString("max_delta")))
                    fan->setMaxDelta(settings["max_delta"].toDouble());

                if(settings.contains(QString("min_temp")))
                    fan->setMinTemperature(settings["min_temp"].toDouble());

                if(settings.contains(QString("max_temp")))
                    fan->setMaxTemperature(settings["max_temp"].toDouble());

                if(settings.contains(QString("pid_Kp")))
                    fan->setPidKp(settings["pid_Kp"].toDouble());

                if(settings.contains(QString("pid_Ki")))
                    fan->setPidKi(settings["pid_Ki"].toDouble());

                if(settings.contains(QString("pid_Kd")))
                    fan->setPidKd(settings["pid_Kd"].toDouble());

                if(settings.contains(QString("pid_deriv_time")))
                    fan->setPidDerivationTime(settings["pid_deriv_time"].toDouble());

                if(settings.contains(QString("target_temp")))
                    fan->setTargetTemperature(settings["target_temp"].toDouble());

                if(settings.contains(QString("max_speed")))
                    fan->setMaxSpeed(settings["max_speed"].toDouble());

                if(settings.contains(QString("min_speed")))
                    fan->setMinSpeed(settings["min_speed"].toDouble());

                if(newFan)
                    m_printer->setFan(fan->name(), fan);
            }

            //Check for heater_generic objects
            else if(key.startsWith(QString("heater_generic")))
            {
                QJsonObject settings = settingsObject[key].toObject();

                bool newHeater = false;
                QKlipperHeater *heater = m_printer->heater(key);

                if(!heater)
                {
                    newHeater = true;
                    heater = new QKlipperHeater(m_printer);
                    heater->setName(key);
                }

                parseHeaterSettings(heater, settings);

                if(newHeater)
                    m_printer->addHeater(heater);
            }
        }
    }

    return true;
}
