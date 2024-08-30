#include "qklipperconsole.h"
#include "qeventloop.h"

QKlipperConsole::QKlipperConsole(QObject *parent)
    : QObject{parent}
{
    generateParserMap();
    resetStartupSequence();
}

QKlipperConsole::~QKlipperConsole()
{
    QList<qint32> messageKeys = m_messageMap.keys();

    foreach(qint32 key, messageKeys)
    {
        QKlipperMessage *message = m_messageMap[key];
        m_messageMap.remove(key);
        delete message;
    }

    if(m_rpcUpdateSocket)
        delete m_rpcUpdateSocket;
}

void QKlipperConsole::connect()
{
    if(m_rpcUpdateSocket && m_rpcUpdateSocket->isOpen())
        return;

    addConnectionState(Connecting);
    addConnectionState(Startup);

    m_printer->setStatus(QKlipperPrinter::Connecting);

    if(m_server->connectionType() == QKlipperServer::Local)
    {
        if(!m_rpcUpdateSocket)
            m_rpcUpdateSocket = (QAbstractSocket*)(new QLocalSocket(this));

        ((QLocalSocket*)m_rpcUpdateSocket)->setServerName(m_server->websocketAddress());
        ((QLocalSocket*)m_rpcUpdateSocket)->connectToServer();

        if(!((QLocalSocket*)m_rpcUpdateSocket)->waitForConnected())
        {
            qDebug() << QString("Failed to connect to moonraker");
            //sendError("Could not connect to local socket");
            return;
        }

        addConnectionState(MoonrakerConnected);

        QObject::connect(m_rpcUpdateSocket, SIGNAL(readyRead()), this, SLOT(rpcUpdateSocketDataReady()));
    }
    else
    {
        if(!m_rpcUpdateSocket)
            m_rpcUpdateSocket = (QAbstractSocket*)(new QWebSocket());

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

        QObject::connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(rpcUpdateSocketDataReceived(QByteArray)));
        QObject::connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(rpcUpdateSocketDataReceived(QString)));

        socket->open(m_server->websocketAddress());
        loop.exec();

        if(errorOccurred)
        {
            qDebug() << QString("Failed to connect to moonraker") << m_server->websocketAddress() << m_server->port();
            qDebug() << socketError;
            //sendError("Could not connect to local socket");
            return;
        }

        addConnectionState(MoonrakerConnected);
    }

    if(m_startupSequence.count())
    {
        qDebug() << QString("Beginning Startup Sequence");
        StartupFunction function = m_startupSequence.dequeue();
        (this->*function)();
    }
}

void QKlipperConsole::disconnect()
{
    if(!m_rpcUpdateSocket)
        return;

    m_rpcUpdateSocket->disconnectFromHost();

    if(!m_rpcUpdateSocket->waitForDisconnected())
    {
        qDebug() << "Timeout waiting for disconnect";
    }

    delete m_rpcUpdateSocket;
    m_rpcUpdateSocket = nullptr;

    removeConnectionState(Syncronized);
}

void QKlipperConsole::machineShutdown(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.shutdown");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }
}

void QKlipperConsole::machineReboot(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.reboot");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }
}

void QKlipperConsole::machineSystemInfo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.system.info");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

bool QKlipperConsole::machineServiceRestart(QString service, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.restart");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineServiceStop(QString service, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.stop");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineServiceStart(QString service, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("service", service);
    message->setMethod("machine.service.start");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

void QKlipperConsole::machinePeripheralsUSB()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.usb");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machinePeripheralsSerial()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.serial");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machinePeripheralsVideo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.video");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machinePeripheralsCanbus(qint32 canBus)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("interface", canBus);
    message->setMethod("machine.peripherals.canbus");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machineProcStats()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.peripherals.video");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machineUpdateStatus()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.status");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::machineUpdateRefresh()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.refresh");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

bool QKlipperConsole::machineUpdateFull(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.full");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateMoonraker(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.moonraker");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateKlipper(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.klipper");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateClient(QString client, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", client);
    message->setMethod("machine.update.client");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateSystem(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("machine.update.system");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateRecover(QString name, bool hardRecover, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", name);
    message->setParam("hard", hardRecover);
    message->setMethod("machine.update.recover");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::machineUpdateRollback(QString name, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("name", name);
    message->setMethod("machine.update.rollback");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

void QKlipperConsole::printerInfo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.info");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error()) {
                qDebug() << reply->errorString() << message->method() << reply->url() ;
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
            {
                //sendError("Invalid response from server");
                return;
            }
            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::printerMCUInfo()
{
    QString object = "mcu";
    printerObjectsQuery(object);
}

void QKlipperConsole::printerObjectsList()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.objects.list");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error()) {
                qDebug() << reply->errorString() << message->method() << reply->url() ;
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
            {
                //sendError("Invalid response from server");
                return;
            }
            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::printerObjectsQuery(QString &object)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam(object, QVariant());
    message->setMethod("printer.objects.query");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::printerSubscribe()
{
    QKlipperMessage *message = new QKlipperMessage();

    foreach(QString object, m_availableObjects)
        message->setParam(object,QVariant());

    message->setMethod("printer.objects.subscribe");

    m_messageMap.insert(message->id(), message);
    sendRpcMessage(message);
}

bool QKlipperConsole::printerEmergencyStop(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.emergency_stop");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

void QKlipperConsole::printerQueryEndstops()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.query_endstops.status");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error()) {
                qDebug() << reply->errorString() << message->method() << reply->url() ;
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

bool QKlipperConsole::printerPrintStart(QString file, QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filename", file);
    message->setMethod("printer.print.start");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::printerPrintPause(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.print.pause");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::printerPrintResume(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.print.resume");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::printerPrintCancel(QKlipperConsoleError *error)
{

    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.printer.cancel");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.emergency_stop");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::printerGcodeScript(QString gcode, QKlipperConsoleError *error, QKlipperMessage::Origin origin)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setOrigin(origin);
    message->setParam("script", gcode);
    message->setMethod("printer.gcode.script");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->post(request, "");
    loop.exec();

    if (reply->error())
    {
        error->setErrorString(reply->errorString());
        error->setType(QKlipperConsoleError::Socket);
        error->setOrigin("printer.gcode.script");
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::restartKlipper(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.restart");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->get(request);
    loop.exec();

    if (reply->error()) {
        qDebug() << reply->errorString() << message->method() << reply->url() ;
        return false;
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::restartFirmware(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("printer.restart_firmware");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->get(request);
    loop.exec();

    if (reply->error()) {
        qDebug() << reply->errorString() << message->method() << reply->url() ;
        return false;
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}

bool QKlipperConsole::serverRestart(QKlipperConsoleError *error)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.restart");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop loop;
    QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

    QNetworkRequest request(m_server->address() + message->toUri());
    QNetworkReply *reply = manager->get(request);
    loop.exec();

    if (reply->error()) {
        qDebug() << reply->errorString() << message->method() << reply->url() ;
        return false;
    }

    //returns ok
    QByteArray data = reply->readAll();

    if(data == "ok")
    {
        if(error)
        {
            error->setErrorString("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    return false;
}


void QKlipperConsole::serverInfo()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.info");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));


            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverConfig()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.config");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFileRoots()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.files.roots");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFilesMetadata(QString fileName)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", fileName);
    message->setParam("extended", true);
    message->setMethod("server.files.metadata");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFilesMetadata(QKlipperFile *file)
{
    //path is relative to gcodes
    QString path;

    if(file->path().length() > 0)
        path = file->path() + QString("/") + file->filename();
    else
        path = file->filename();

    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", path);
    message->setParam("extended", true);
    message->setMethod("server.files.metadata");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFilesList(QString directory)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setParam("extended", true);
    message->setMethod("server.files.get_directory");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFileDelete(QString file)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", file);
    message->setParam("extended", true);
    message->setMethod("server.files.delete_file");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFileDelete(QKlipperFile *file)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", file->uri());
    message->setParam("extended", true);
    message->setMethod("server.files.delete_file");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFileMove(QString source, QString destination)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("source", source);
    message->setParam("dest", destination);
    message->setMethod("server.files.move");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {

            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());


            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverFileCopy(QString source, QString destination)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("source", source);
    message->setParam("dest", destination);
    message->setMethod("server.files.copy");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());


            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

QByteArray QKlipperConsole::serverFileDownload(QKlipperFile *file, QKlipperConsoleError *error)
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
            rootLocation += QDir::separator() + file->path();

        rootLocation += QDir::separator() + file->filename();

        QFile localFile(rootLocation);


        if(localFile.open(QFile::ReadOnly))
        {
            fileData = localFile.readAll();
            localFile.close();
        }
        else
        {
            error->setErrorString(localFile.errorString());
            error->setOrigin("Console - Server.Files.Download");
            error->setType(QKlipperConsoleError::File);
        }
    }
    else if(m_server->connectionType() == QKlipperServer::Remote)
    {
        QString address = m_server->address() + file->uri();

        QNetworkAccessManager *manager = new QNetworkAccessManager();

        QEventLoop loop;
        QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

        QNetworkRequest request(address);
        QNetworkReply *reply = manager->get(request);
        loop.exec();

        if(reply->errorString().length() > 0)
        {
            error->setErrorString(reply->errorString());
            error->setOrigin("Console - Server.Files.Download");
            error->setType(QKlipperConsoleError::Socket);
        }
        else
        {
            error->setErrorString("");
            error->setOrigin("");
            error->setType(QKlipperConsoleError::NoError);
        }
    }

    return fileData;
}

bool QKlipperConsole::serverFileUpload(QString root, QString directory, QString name, QByteArray data, QKlipperConsoleError *error)
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

            error->setErrorString("");
            error->setOrigin("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
        else
        {
            error->setErrorString(localFile.errorString());
            error->setOrigin("Console - Server.Files.Upload");
            error->setType(QKlipperConsoleError::File);
            return false;
        }
    }
    else if(m_server->connectionType() == QKlipperServer::Remote)
    {
        QString address = m_server->address();

        if(directory.isEmpty())
            address += root + QDir::separator() + name;
        else
            address += root + QDir::separator() + directory + QDir::separator() + name;

        QNetworkAccessManager *manager = new QNetworkAccessManager();

        QEventLoop loop;
        QObject::connect(manager, SIGNAL(finished()), &loop, SLOT(quit()));

        QNetworkRequest request(address);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");

        QString contentDisposition;

        if(directory.length() > 0)
            contentDisposition = QString("form-data; name=\"file\"; path=\"%1\" filename=\"%2\"").arg(directory, name);
        else
            contentDisposition = QString("form-data; name=\"file\"; filename=\"%1\"").arg(name);

        request.setHeader(QNetworkRequest::ContentDispositionHeader, contentDisposition);
        QNetworkReply *reply = manager->post(request, data);
        loop.exec();

        if(reply->errorString().length() > 0)
        {
            error->setErrorString(reply->errorString());
            error->setOrigin("Console - Server.Files.Upload");
            error->setType(QKlipperConsoleError::Socket);
            return false;
        }
        else
        {
            error->setErrorString("");
            error->setOrigin("");
            error->setType(QKlipperConsoleError::NoError);
            return true;
        }
    }

    error->setErrorString("Unknown Error");
    error->setOrigin("Console - Server.Files.Upload");
    error->setType(QKlipperConsoleError::Unspecified);
    return false;
}

void QKlipperConsole::serverDirectoryPost(QString directory)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setMethod("server.files.post_directory");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverDirectoryDelete(QString directory)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("path", directory);
    message->setMethod("server.files.delete_directory");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverTemperatureStore()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("include_monitors", false);
    message->setMethod("server.temperature_store");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverGcodeStore()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("count", 100);
    message->setMethod("server.gcode_store");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverLogsRollover()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.logs.rollover");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverLogsRollover(QString &application)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("application", application);
    message->setMethod("server.logs.rollover");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverWebsocketId()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.websocket.id");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverWebcamList()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.webcams.list");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
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
    message->setMethod("server.webcams.post_item");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
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
    message->setMethod("server.webcams.post_item");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverWebcamDelete(QKlipperWebcam *webcam)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("uid", webcam->uid());
    message->setMethod("server.webcams.delete_item");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverAnnouncementsList(bool includeDismissed)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("include_dismissed", includeDismissed);
    message->setMethod("server.announcements.list");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverAnnouncementsUpdate()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.announcements.update");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverAnnouncementDismiss(QString entryId, qint64 waketime)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("entry_id", entryId);
    message->setParam("waketime", waketime);
    message->setMethod("server.announcements.dismiss");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueueStatus()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.status");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueueStart()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.start");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueuePause()
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setMethod("server.job_queue.pause");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueueJump(QString id)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("job_id", id);
    message->setMethod("server.job_queue.jump");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueueAdd(QStringList filenames)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("filenames", filenames);
    message->setMethod("server.job_queue.post_job");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::serverJobQueueDelete(QStringList ids)
{
    QKlipperMessage *message = new QKlipperMessage();
    message->setParam("job_ids", ids);
    message->setMethod("server.job_queue.jump");

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply)
        {
            if (reply->error())
                message->setErrorString(reply->errorString());

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
}

void QKlipperConsole::clientIdentifier()
{
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

    m_messageMap.insert(message->id(), message);

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect
    (
        manager,
        &QNetworkAccessManager::finished,
        this, [message, this](QNetworkReply *reply) mutable
        {
            if (reply->error()) {
                qDebug() << reply->errorString() << message->method() << reply->url() ;
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonParseError responseDocumentError;
            QJsonDocument responseDocument(QJsonDocument::fromJson(responseData, &responseDocumentError));

            if(responseDocumentError.error != QJsonParseError::NoError)
                message->setErrorString("Invalid Response From Server");

            message->setResponse(responseDocument["result"].toObject());

            parseResponse(message);
        }
    );

    QString uri = m_server->address() + message->toUri();
    manager->get(QNetworkRequest(uri));
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
    if(m_rpcUpdateSocket->isOpen())
    {
        QByteArray incoming = m_rpcUpdateSocket->readAll();
        m_rpcDataBuffer += incoming;
    }

    scanRpcUpdateBuffer();
}

void QKlipperConsole::rpcUpdateSocketDataReceived(QString data)
{
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
        message->setErrorString(responseObject["error"].toString());

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

void QKlipperConsole::rpcUpdateSocketDataReceived(QByteArray data)
{
    //m_rpcDataBuffer += data;
    //scanRpcUpdateBuffer();
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
            message->setErrorString(responseObject["error"].toString());

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
        delete message;
        message = nullptr;
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
    setConnectionState((ConnectionState)(m_connectionState | state));

    if(hasConnectionState((ConnectionState)(KlipperConnected | MoonrakerConnected)) &&
            !hasConnectionState(Syncronized))
        addConnectionState(Syncronized);
}

void QKlipperConsole::removeConnectionState(ConnectionState state)
{
    if((state & KlipperConnected) == KlipperConnected || (state & MoonrakerConnected) == MoonrakerConnected)
    {
        if(hasConnectionState(Syncronized))
            m_connectionState = (ConnectionState)(m_connectionState & ~Syncronized);
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
}

void QKlipperConsole::resetConnectionState()
{
    setConnectionState({}); // TODO: Adapt to use your actual default value
}

void QKlipperConsole::resetStartupSequence()
{
    m_startupSequence.clear();

    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::clientIdentifier);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::accessUsersList);
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::machineSystemInfo);
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
    m_startupSequence.enqueue((StartupFunction)&QKlipperConsole::printerSubscribe);

    m_startupSequenceCount = m_startupSequence.count();
    addConnectionState(Startup);
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
    m_parserMap[QString("printer.objects.query")] = (ParserFunction)&QKlipperConsole::printerObjectsQueryParser;
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

QString QKlipperConsole::errorMessage() const
{
    return m_errorMessage;
}

void QKlipperConsole::setErrorMessage(const QString &errorMessage)
{
    if (m_errorMessage == errorMessage)
        return;
    m_errorMessage = errorMessage;
    emit errorMessageChanged();
}

qreal QKlipperConsole::startupSequenceProgress() const
{
    return m_startupSequenceProgress;
}

void QKlipperConsole::setStartupSequenceProgress(qreal startupSequenceProgress)
{
    if (qFuzzyCompare(m_startupSequenceProgress, startupSequenceProgress))
        return;

    m_startupSequenceProgress = startupSequenceProgress;
    emit startupSequenceProgressChanged();
}

void QKlipperConsole::resetStartupSequenceProgress()
{
    setStartupSequenceProgress(0); // TODO: Adapt to use your actual default value
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

            data = data.removeLast();

            qint64 sent = socket->sendTextMessage(data);

            if(sent != data.length())
            {
                qDebug() << QString("Sent %1 of %2").arg(QString::number(sent), QString::number(data.length()));
            }
        }
    }
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
        //string message
    }
    else if(message->method() == QString("notify_update_response"))
    {
        /*{
            "jsonrpc": "2.0",
            "method": "notify_update_response",
           "params": [
            {
              "application": "{app_name}",
              "proc_id": 446461,
              "message": "Update Response Message",
              "complete": false
            }
            ]
        }*/
    }
    else if(message->method() == QString("notify_update_refreshed"))
    {
        /*{
            "jsonrpc": "2.0",
            "method": "notify_update_response",
           "params": [
            {
              "application": "{app_name}",
              "proc_id": 446461,
              "message": "Update Response Message",
              "complete": false
            }
            ]
        }*/
    }
    else if(message->method() == QString("notify_cpu_throttled"))
    {
        /*{
            "jsonrpc": "2.0",
            "method": "notify_update_response",
           "params": [
            {
              "application": "{app_name}",
              "proc_id": 446461,
              "message": "Update Response Message",
              "complete": false
            }
            ]
        }*/
    }
    else if(message->method() == QString("notify_history_changed"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_history_changed",
            "params": [
                {
                    "action": "added OR finished",
                    "job": <job object>
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_user_created"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_user_created",
            "params": [
                {
                    "username": "<username>"
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_user_deleted"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_user_created",
            "params": [
                {
                    "username": "<username>"
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_user_logged_out"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_user_created",
            "params": [
                {
                    "username": "<username>"
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_service_state_changed"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_service_state_changed",
            "params": [
                {
                    "klipper": {
                        "active_state": "inactive",
                        "sub_state": "dead"
                    }
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_job_queue_changed"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_job_queue_changed",
            "params": [
                {
                    "action": "state_changed|jobs_added|jobs_removed|job_loaded",
                    "updated_queue": null,
                    "queue_state": "paused"
                }
            ]
        }
        */
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
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_announcement_dismissed",
            "params": [
                {
                    "entry_id": "arksine/moonlight/issue/3"
                }
            ]
        }
        */
    }
    else if(message->method() == QString("notify_announcement_wake"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_announcement_wake",
            "params": [
                {
                    "entry_id": "arksine/moonlight/issue/1"
                }
            ]
        }
        */
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
    else if(message->method() == QString("notify_sudo_alert"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "notify_webcams_changed",
            "params": [
                {
                    "webcams": [
                        {
                            "name": "tc2",
                            "location": "printer",
                            "service": "mjpegstreamer",
                            "enabled": true,
                            "icon": "mdiWebcam",
                            "target_fps": 15,
                            "target_fps_idle": 5,
                            "stream_url": "http://printer.lan/webcam?action=stream",
                            "snapshot_url": "http://printer.lan/webcam?action=snapshot",
                            "flip_horizontal": false,
                            "flip_vertical": false,
                            "rotation": 0,
                            "aspect_ratio": "4:3",
                            "extra_data": {},
                            "source": "database"
                        },
                        {
                            "name": "TestCam",
                            "location": "printer",
                            "service": "mjpegstreamer",
                            "enabled": true,
                            "icon": "mdiWebcam",
                            "target_fps": 15,
                            "target_fps_idle": 5,
                            "stream_url": "/webcam/?action=stream",
                            "snapshot_url": "/webcam/?action=snapshot",
                            "flip_horizontal": false,
                            "flip_vertical": false,
                            "rotation": 0,
                            "aspect_ratio": "4:3",
                            "extra_data": {},
                            "source": "database"
                        }
                    ]
                }
            ]
        }
        */
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
    else if(message->method() == QString("sensor_update"))
    {
        /*
        {
            "jsonrpc": "2.0",
            "method": "sensors:sensor_update",
            "params": [
                {
                    "sensor1": {
                        "humidity": 28.9,
                        "temperature": 22.4
                    }
                }
            ]
        }
        */
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
        }
        else
        {
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
            QJsonObject serviceState = systemInfo["service_state"].toObject();
            QStringList keys = serviceState.keys();

            QMap<QString, QKlipperServiceState> serviceStates;

            foreach(QString key, keys)
            {
                if(serviceState[key].isObject())
                {
                    QJsonObject stateObject = serviceState[key].toObject();
                    QKlipperServiceState state;

                    if(stateObject.contains("active_state"))
                        state.setActiveState(stateObject["active_state"].toString());

                    if(stateObject.contains("sub_state"))
                        state.setSubState(stateObject["sub_state"].toString());

                    state.setName(key);

                    serviceStates[key] = state;
                }
            }

            m_system->setServiceStates(serviceStates);
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
    QKlipperUpdateState *updateState = m_system->updateState();

    updateState->setIsBusy(message->response()["busy"].toBool());
    updateState->setGithubLimitResetTime(message->response()["github_limit_reset_time"].toInt());
    updateState->setGithubRateLimit(message->response()["github_rate_limit"].toInt());
    updateState->setGithubRequestsRemaining(message->response()["github_requests_remaining"].toInt());

    //Grab the version info
    QJsonObject versionObject = message->response()["version_info"].toObject();
    QStringList keys = versionObject.keys();
    QMap<QString, QKlipperUpdatePackage> packages;

    foreach(QString key, keys)
    {
        //System version info
        if(key == QString("system"))
        {
            QJsonObject systemObject = versionObject[key].toObject();
            QJsonArray packageArray = systemObject["package_list"].toArray();

            updateState->setSystemPackageCount(systemObject["package_count"].toInt());

            QStringList systemPackages;

            for(int i = 0; i < packageArray.count(); i++)
                systemPackages += packageArray[i].toString();

            updateState->setSystemPackages(systemPackages);
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
            }

            packageState.setCommitsBehind(commits);

            //Add to packages map
            packages.insert(key, packageState);
        }
    }
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
    //First parse the config file object to construct all printer details
    //    as set in the config
    if(message->response().toObject().contains(QString("configfile")))
    {
        QJsonObject configFileObject = message->response()["configfile"].toObject();

        //Most of the config file is useless at the beta stage
        //    a lot of these will be blank
        if(configFileObject.contains(QString("config")))
        {
            QJsonObject configObject = configFileObject["config"].toObject();
        }
        if(configFileObject.contains(QString("settings")))
        {
            QJsonObject settingsObject = configFileObject["settings"].toObject();

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

                QKlipperPrintBedMesh::Limit minimum, maximum, probeCount;

                maximum.x = meshMaxArray[0].toDouble();
                maximum.y = meshMaxArray[1].toDouble();

                minimum.x = meshMinArray[0].toDouble();
                minimum.y = meshMinArray[1].toDouble();

                probeCount.x = probeCountArray[0].toDouble();
                probeCount.y = probeCountArray[1].toDouble();

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
            }

            //Parse MCU settings
            if(settingsObject.contains(QString("mcu")))
            {
                QJsonObject mcuObject = settingsObject["mcu"].toObject();
                QKlipperMCU *mcu = m_printer->mcu();

                if(mcuObject.contains("baud"))
                    mcu->setBaudRate(mcuObject["baud"].toInt());

                if(mcuObject.contains("max_stepper_error"))
                    mcu->setMaxStepperError(mcuObject["max_stepper_error"].toDouble());
            }

            //Parse Safe Z Home settings
            if(settingsObject.contains(QString("safe_z_home")))
            {
                QJsonObject safeZObject = settingsObject["safe_z_home"].toObject();
                QKlipperSafeZHome *zHome = m_printer->safeZHome();

                if(safeZObject.contains(QString("home_xy_position")))
                {
                    QJsonArray homePositionArray = safeZObject["home_xy_position"].toArray();

                    if(homePositionArray.count() >= 2)
                    {
                        zHome->setHomeXPosition(homePositionArray[0].toDouble());
                        zHome->setHomeYPosition(homePositionArray[1].toDouble());
                    }
                }

                zHome->setMoveToPrevious(safeZObject["move_to_previous"].toBool());

                zHome->setSpeed(safeZObject["speed"].toDouble());
                zHome->setZHopSpeed(safeZObject["z_hop"].toDouble());
                zHome->setZHop(safeZObject["z_hop_speed"].toDouble());
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
                        m_printer->bed()->setAdjustmentScrew(screwNameString, adjustmentScrew);
                }
            }

            if(settingsObject.contains(QString("heater_bed")))
            {
                QJsonObject heaterBed = settingsObject[QString("heater_bed")].toObject();

                m_printer->bed()->setType(QKlipperPrintBed::Heated);

                if(heaterBed.contains("control"))
                {
                    QString control = heaterBed["control"].toString();
                    m_printer->bed()->setControl(control);
                }

                if(heaterBed.contains("heater_pin"))
                {
                    QString pin = heaterBed["heater_pin"].toString();
                    m_printer->bed()->setHeaterPin(pin);
                }

                if(heaterBed.contains("inline_resistor"))
                {
                    qreal resistor = heaterBed["inline_resistor"].toDouble();
                    m_printer->bed()->setInlineResistor(resistor);
                }

                if(heaterBed.contains("max_power"))
                {
                    qreal power = heaterBed["max_power"].toDouble();
                    m_printer->bed()->setMaxPower(power);
                }

                if(heaterBed.contains("max_temp"))
                {
                    qreal temp = heaterBed["max_temp"].toDouble();
                    m_printer->bed()->setMaxTemp(temp);
                }

                if(heaterBed.contains("min_temp"))
                {
                    qreal temp = heaterBed["min_temp"].toDouble();
                    m_printer->bed()->setMinTemp(temp);
                }

                if(heaterBed.contains("pid_kd"))
                {
                    qreal pid = heaterBed["pid_kd"].toDouble();
                    m_printer->bed()->setPidKD(pid);
                }

                if(heaterBed.contains("pid_ki"))
                {
                    qreal pid = heaterBed["pid_ki"].toDouble();
                    m_printer->bed()->setPidKI(pid);
                }

                if(heaterBed.contains("pid_kp"))
                {
                    qreal pid = heaterBed["pid_kp"].toDouble();
                    m_printer->bed()->setPidKP(pid);
                }

                if(heaterBed.contains("pullup_resistor"))
                {
                    qreal resistor = heaterBed["pullup_resistor"].toDouble();
                    m_printer->bed()->setPullupResistor(resistor);
                }

                if(heaterBed.contains("pwm_cycle_time"))
                {
                    qreal cycle = heaterBed["pwm_cycle_time"].toDouble();
                    m_printer->bed()->setPwmCycleTime(cycle);
                }

                if(heaterBed.contains("sensor_pin"))
                {
                    QString pin = heaterBed["sensor_pin"].toString();
                    m_printer->bed()->setSensorPin(pin);
                }

                if(heaterBed.contains("sensor_type"))
                {
                    QString type = heaterBed["sensor_type"].toString();
                    m_printer->bed()->setSensorType(type);
                }
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

                if(extruderObject.contains("control"))
                {
                    QString control = extruderObject["control"].toString();
                    extruder->setControl(control);
                }

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

                if(extruderObject.contains("heater_pin"))
                {
                    QString pin = extruderObject["heater_pin"].toString();
                    extruder->setHeaterPin(pin);
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

                if(extruderObject.contains("max_power"))
                {
                    qreal power = extruderObject["max_power"].toDouble();
                    extruder->setMaxPower(power);
                }

                if(extruderObject.contains("max_temp"))
                {
                    qreal temp = extruderObject["max_temp"].toDouble();
                    extruder->setMaxTemp(temp);
                }

                if(extruderObject.contains("microsteps"))
                {
                    qint32 microsteps = extruderObject["microsteps"].toInt();
                    extruder->setMicrosteps(microsteps);
                }

                if(extruderObject.contains("min_extrude_temp"))
                {
                    qreal temp = extruderObject["min_extrude_temp"].toDouble();
                    extruder->setMinExtrudeTemp(temp);
                }

                if(extruderObject.contains("min_temp"))
                {
                    qreal temp = extruderObject["min_temp"].toDouble();
                    extruder->setMinTemp(temp);
                }

                if(extruderObject.contains("nozzle_diameter"))
                {
                    qreal diameter = extruderObject["nozzle_diameter"].toDouble();
                    extruder->setNozzleDiameter(diameter);
                }

                if(extruderObject.contains("pid_kd"))
                {
                    qreal pid = extruderObject["pid_kd"].toDouble();
                    extruder->setPidKD(pid);
                }

                if(extruderObject.contains("pid_ki"))
                {
                    qreal pid = extruderObject["pid_ki"].toDouble();
                    extruder->setPidKI(pid);
                }

                if(extruderObject.contains("pid_kp"))
                {
                    qreal pid = extruderObject["pid_kp"].toDouble();
                    extruder->setPidKP(pid);
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

                if(extruderObject.contains("pwm_cycle_time"))
                {
                    qreal cycle = extruderObject["pwm_cycle_time"].toDouble();
                    extruder->setPwmCycle(cycle);
                }

                if(extruderObject.contains("rotation_distance"))
                {
                    qreal distance = extruderObject["rotation_distance"].toDouble();
                    extruder->setRotationDistance(distance);
                }

                if(extruderObject.contains("sensor_pin"))
                {
                    QString pin = extruderObject["sensor_pin"].toString();
                    extruder->setSensorPin(pin);
                }

                if(extruderObject.contains("sensor_type"))
                {
                    QString type = extruderObject["sensor_type"].toString();
                    extruder->setSensorType(type);
                }

                if(extruderObject.contains("smooth_time"))
                {
                    qreal smoothing = extruderObject["smooth_time"].toDouble();
                    extruder->setSmoothTimeData(smoothing);
                }

                if(extruderObject.contains("step_pin"))
                {
                    QString pin = extruderObject["step_pin"].toString();
                    extruder->setStepPin(pin);
                }

                if(newExtruder)
                {
                    m_printer->setExtruder(extruderName, extruder);
                }
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
                        QKlipperFan *fan;

                        if(!m_printer->fans().contains(key))
                        {
                            fan = new QKlipperFan(m_printer);
                            newFan = true;
                        }
                        else
                            fan = m_printer->fan(key);

                        QJsonObject fanObject = settingsObject[key].toObject();

                        if(fanObject.contains(QString("heater")))
                        {
                            QJsonArray heatersArray = fanObject["heater"].toArray();

                            for(int i = 0; i < heatersArray.count(); i++)
                            {
                                QString heater = heatersArray[i].toString();

                                if(heater.toLower().startsWith("extruder"))
                                {
                                    QKlipperExtruder *extruder = m_printer->extruder(heater);

                                    if(extruder)
                                    {
                                        extruder->setFan(m_printer->fan(key));
                                        break;
                                    }
                                }
                            }
                        }

                        if(newFan)
                            m_printer->setFan(key, fan);
                    }
                }

                //Check for controller_fan objects
                else if(key.startsWith(QString("controller_fan")))
                {
                }
            }
        }
    }

    //Parse extruders
    for(int index = 0; true; index++)
    {
        QString extruderName = QString("extruder") + ((index > 0) ? QString::number(index) : QString(""));

        if(!message->response().toObject().contains(extruderName))
            break;

        m_printer->extruder(extruderName)->m_watts = m_printer->powerProfile()[extruderName];
        m_printer->extruder(extruderName)->m_name = extruderName;

        QJsonObject extruder = message->response()[extruderName].toObject();
        if(extruder.contains("temperature"))
        {
            qreal temp = extruder["temperature"].toDouble();
            m_printer->extruder(extruderName)->setCurrentTemp(temp);
        }
        if(extruder.contains("target"))
        {
            qreal temp = extruder["target"].toDouble();
            m_printer->extruder(extruderName)->setTargetTempData(temp);
        }
        if(extruder.contains("pressure_advance"))
        {
            qreal advance = extruder["pressure_advance"].toDouble();
            m_printer->extruder(extruderName)->setPressureAdvanceData(advance);
        }
        if(extruder.contains("smooth_time"))
        {
            qreal smoothing = extruder["smooth_time"].toDouble();
            m_printer->extruder(extruderName)->setSmoothTimeData(smoothing);
        }
        if(extruder.contains("power"))
        {
            qreal power = extruder["power"].toDouble();
            m_printer->extruder(extruderName)->setPower(power);
        }
        if(extruder.contains("can_extrude"))
        {
            bool canExtrude = extruder["can_extrude"].toBool();
            m_printer->extruder(extruderName)->setCanExtrude(canExtrude);
        }
    }

    //Parse fan status
    if(message->response().toObject().contains("fan"))
    {
        QJsonObject fan = message->response()["fan"].toObject();
        if(fan.contains("speed"))
        {
            double speed =fan["speed"].toDouble();
            m_printer->toolhead()->partsFan()->setSpeedData(speed);
        }
        if(fan.contains("rpm"))
        {
            double rpm =fan["rpm"].toDouble();
            m_printer->toolhead()->partsFan()->setRpmData(rpm);
        }

        m_printer->toolhead()->partsFan()->setIsControllable(true);
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
                    position[name] = positionArray[3].toDouble();
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
                    position[name] = positionArray[3].toDouble();
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
                    position[name] = positionArray[3].toDouble();
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
        QJsonObject heater = message->response()["heater_bed"].toObject();

        if(heater.contains("temperature"))
        {
            double temp = heater["temperature"].toDouble();
            m_printer->bed()->setCurrentTemp(temp);
        }
        if(heater.contains("target"))
        {
            double temp = heater["target"].toDouble();
            m_printer->bed()->setTargetTempData(temp);
        }
        if(heater.contains("power"))
        {
            double power = heater["power"].toDouble();
            m_printer->bed()->setPower(power);
        }
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
                    position[name] = positionArray[3].toDouble();
                }

                m_printer->toolhead()->setPosition(position);
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
            QJsonObject commandObject = commandsObject[key].toObject();

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
            m_printer->toolhead()->currentExtruder()->setExtrusionFactor(move.extrusionFactor);

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

        QKlipperPrintBedMesh::Limit maximum, minimum;

        maximum.x = meshMaxArray[0].toDouble();
        maximum.y = meshMaxArray[1].toDouble();

        bedMesh->setMaximum(maximum);

        minimum.x = meshMinArray[0].toDouble();
        minimum.y = meshMinArray[1].toDouble();

        bedMesh->setMinimum(minimum);

        //Parse the 2D array of probed values
        QJsonArray probedArray = bedMeshObject["probed_matrix"].toArray();

        QList<QList<qreal>> probed(probedArray.count());

        for(int i = 0; i < probedArray.count(); i++)
        {
            QJsonArray probedEntriesArray = probedArray[i].toArray();
            probed[i].resize(probedEntriesArray.count());

            for(int e = 0; e < probedEntriesArray.count(); e++)
            {
                probed[i][e] = probedEntriesArray[e].toDouble();
            }
        }

        bedMesh->setProbed(probed);

        //Parse the 2D array of calculated values
        QJsonArray matrixArray = bedMeshObject["mesh_matrix"].toArray();

        QList<QList<qreal>> matrix(probedArray.count());

        for(int i = 0; i < matrixArray.count(); i++)
        {
            QJsonArray matrixEntriesArray = matrixArray[i].toArray();
            matrix[i].resize(matrixEntriesArray.count());

            for(int e = 0; e < matrixEntriesArray.count(); e++)
            {
                matrix[i][e] = matrixEntriesArray[e].toDouble();
            }
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
            virtualSDCard->setProgress(virtualSDObject["progress"].toDouble());
    }

    //Parse declared fan objects
    foreach(QString key, m_server->availableObjects())
    {
        //Check for heater_fan objects
        if(key.startsWith(QString("heater_fan")))
        {
            if(message->response().toObject().contains(key))
            {
                QJsonObject fanObject = message->response()[key].toObject();

                bool newFan = false;
                QKlipperFan *fan;

                if(!m_printer->fans().contains(key))
                {
                    fan = new QKlipperFan(m_printer);
                    newFan = true;
                }
                else
                    fan = m_printer->fan(key);

                if(fanObject.contains(QString("rpm")))
                    fan->setRpmData(fanObject["rpm"].toDouble());

                if(fanObject.contains(QString("speed")))
                    fan->setSpeedData(fanObject["speed"].toDouble());

                if(newFan)
                    m_printer->setFan(key, fan);
            }
        }

        //Check for controller_fan objects
        else if(key.startsWith(QString("controller_fan")))
        {
            if(message->response().toObject().contains(key))
            {
                QJsonObject fanObject = message->response()[key].toObject();

                bool newFan = false;
                QKlipperFan *fan;

                if(!m_printer->fans().contains(key))
                {
                    fan = new QKlipperFan(m_printer);
                    newFan = true;
                }
                else
                    fan = m_printer->fan(key);

                if(fanObject.contains(QString("rpm")))
                    fan->setRpmData(fanObject["rpm"].toDouble());

                if(fanObject.contains(QString("speed")))
                    fan->setSpeedData(fanObject["speed"].toDouble());

                if(newFan)
                    m_printer->setFan(key, fan);
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
                adjustmentScrews[screwString] = adjustmentScrew;
        }
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
                delete m_klipperRestartTimer;

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

}

void QKlipperConsole::serverFileRootsParser(QKlipperMessage *message)
{
    foreach(QJsonValueConstRef reference, message->response().toArray())
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
                    ;
                else if(referenceObject["name"] == QString("docs"))
                    ;
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

            QString error = QString("Could not rollover logfile %1: %2").arg(application, value);

            setErrorMessage(error);
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
    QString path = QString("gcodes/%1").arg(message->param("path").toString());

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

    m_server->setFileMetadata(path, metadata);
}

void QKlipperConsole::serverFilesDeleteParser(QKlipperMessage *message)
{

}

void QKlipperConsole::serverFilesMoveParser(QKlipperMessage *message)
{

}

void QKlipperConsole::serverFilesCopyParser(QKlipperMessage *message)
{

}

void QKlipperConsole::serverFilesListParser(QKlipperMessage *message)
{
    QJsonObject rootInfo = message->response()["root_info"].toObject();
    QJsonArray files = message->response()["files"].toArray();
    QJsonArray directories = message->response()["dirs"].toArray();
    QKlipperFileList fileList;

    if(message->response().toObject().contains(QString("disk_usage")))
    {
        QJsonObject driveUsage = message->response()["disk_usage"].toObject();
        m_system->setDriveUsage(driveUsage["used"].toInteger());
        m_system->setDriveCapacity(driveUsage["total"].toInteger());
        m_system->setDriveFree(driveUsage["free"].toInteger());
    }

    QString directory = message->param("path").toString();
    QString root = rootInfo["name"].toString();

    if(directory.contains(QString("/")))
    {
        if(directory.endsWith(QString("/")))
            directory = directory.mid(0, directory.length() -1);

        directory.remove(root);

        if(directory.startsWith(QString("/")))
            directory = directory.mid(1, directory.length() -1);

        directory = directory.mid(0, directory.lastIndexOf(QString("/")));
    }
    else
        directory.remove(root);

    foreach(QJsonValueConstRef directoryRef, directories)
    {
        if(directoryRef.isObject())
        {
            QJsonObject directoryObject = directoryRef.toObject();

            bool newDir = false;
            QKlipperFile *directoryListing = m_server->file(root + QString('/') + message->param("path").toString());

            if(!directoryListing)
            {
                newDir = true;
                directoryListing = new QKlipperFile(m_server);
            }

            QString directoryName = directoryObject["dirname"].toString();

            directoryListing->setFilename(directoryName);
            directoryListing->setRoot(root);
            directoryListing->setPath(directory);
            directoryListing->setFileSize(directoryObject["size"].toDouble());
            directoryListing->setDateModified(directoryObject["modified"].toDouble());
            directoryListing->setFileType(QKlipperFile::Directory);

            if(newDir)
                m_server->setFile(directoryListing);
        }
    }

    foreach(QJsonValueConstRef fileRef, files)
    {
        if(fileRef.isObject())
        {
            QJsonObject directoryObject = fileRef.toObject();

            bool newFile = false;
            QKlipperFile *file = m_server->file(root + QString('/') + message->param("path").toString());

            if(!file)
            {
                newFile = true;
                file = new QKlipperFile(m_server);
            }

            QString directoryName = directoryObject["filename"].toString();

            file->setFilename(directoryName);
            file->setRoot(root);
            file->setPath(directory);
            file->setFileSize(directoryObject["size"].toDouble());
            file->setDateModified(directoryObject["modified"].toDouble());
            file->setFileType(QKlipperFile::Directory);

            if(root == QString("gcodes"))
                file->setFileType(QKlipperFile::GCode);
            else if(root == QString("config"))
                file->setFileType(QKlipperFile::Config);

            if(newFile)
                m_server->setFile(file);
        }
    }
}

void QKlipperConsole::serverDirectoryPostParser(QKlipperMessage *message)
{

}

void QKlipperConsole::serverDirectoryDeleteParser(QKlipperMessage *message)
{

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
            delete job;
        }
    }
}
