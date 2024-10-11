#include "qklipperinstance.h"
#include "qsettings.h"

QKlipperInstance::QKlipperInstance(QObject *parent)
    : QObject{parent}
{
    m_id = QUuid::createUuid().toString();
    reset();
}

QKlipperInstance::~QKlipperInstance()
{
    if(m_system)
        delete m_system;
    if(m_printer)
        delete m_printer;
    if(m_server)
        delete m_server;
    if(m_console)
        delete m_console;
}

void QKlipperInstance::reset()
{
    if(m_system)
        delete m_system;
    if(m_printer)
        delete m_printer;
    if(m_server)
        delete m_server;
    if(m_console)
        delete m_console;

    m_console = new QKlipperConsole(this);
    m_system = new QKlipperSystem(this);
    m_server = new QKlipperServer(this);
    m_printer = new QKlipperPrinter(this);

    m_console->setSystem(m_system);
    m_console->setServer(m_server);
    m_console->setPrinter(m_printer);
}

QString QKlipperInstance::name() const
{
    return m_name;
}

void QKlipperInstance::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
}

QString QKlipperInstance::address() const
{
    return m_address;
}

void QKlipperInstance::setAddress(const QString &address)
{
    if (m_address == address)
        return;

    if(m_server)
        m_server->setAddress(address);

    m_address = address;
    emit addressChanged();
}

QString QKlipperInstance::id() const
{
    return m_id;
}

void QKlipperInstance::setId(const QString &id)
{
    if (m_id == id)
        return;

    m_id = id;
    emit idChanged();
}

QString QKlipperInstance::apiKey() const
{
    return m_apiKey;
}

void QKlipperInstance::setApiKey(const QString &apiKey)
{
    if (m_apiKey == apiKey)
        return;

    if(m_server)
        m_server->setApiKey(apiKey);

    m_apiKey = apiKey;
    emit apiKeyChanged();
}

qint16 QKlipperInstance::port() const
{
    return m_port;
}

void QKlipperInstance::setPort(qint16 port)
{
    if (m_port == port)
        return;

    m_port = port;
    emit portChanged();
}

QString QKlipperInstance::instanceLocation() const
{
    return m_instanceLocation;
}

void QKlipperInstance::setInstanceLocation(const QString &instanceLocation)
{
    if (m_instanceLocation == instanceLocation)
        return;

    m_instanceLocation = instanceLocation;
    emit instanceLocationChanged();
}

void QKlipperInstance::printerStateChanged()
{
    qDebug() << m_printer->statusMessage();
}

QKlipperServer::ConnectionType QKlipperInstance::connectionType() const
{
    return m_connectionType;
}

void QKlipperInstance::connect()
{
    QKlipperClientIdentifier identifier;

#ifndef QKLIPPER_IDENTIFIER_NAME
    identifier.name = "QKlipper App";
#else
    identifier.name = QKLIPPER_IDENTIFIER_NAME;
#endif

#ifndef QKLIPPER_IDENTIFIER_VERSION
    identifier.version = "0.0.1";
#else
    identifier.version = QKLIPPER_IDENTIFIER_VERSION;
#endif

#ifndef QKLIPPER_IDENTIFIER_TYPE
    identifier.type = "other";
#else
    identifier.type = QKLIPPER_IDENTIFIER_TYPE;
#endif

#ifndef QKLIPPER_IDENTIFIER_URL
    identifier.url = "n/a";
#else
    identifier.url = QKLIPPER_IDENTIFIER_URL;
#endif

    m_server->setClientIdentifier(identifier);
    m_server->setConnectionType(m_connectionType);
    m_server->setApiKey(m_apiKey);

    m_server->setAddress(m_address);
    m_server->setPort(m_port);

    if(m_instanceLocation.isEmpty())
        m_server->setConnectionType(QKlipperServer::Remote);
    else
    {
        m_server->setInstanceLocation(m_instanceLocation);
        m_server->setConnectionType(QKlipperServer::Local);
    }

    QObject::connect(m_printer, SIGNAL(statusMessageChanged()), this, SLOT(printerStateChanged()));
    QObject::connect(m_console, SIGNAL(connectionStateChanged()), this, SLOT(onConsoleConnectionStateChanged()));

    QObject::connect(m_server->jobQueue(), SIGNAL(jobAdded(QKlipperPrintJob*)), this, SLOT(onServerPrintJobAdded(QKlipperPrintJob*)));
    QObject::connect(m_server->jobQueue(), SIGNAL(jobRemoved(QKlipperPrintJob*)), this, SLOT(onServerPrintJobRemoved(QKlipperPrintJob*)));

    if(m_console)
        m_console->connect();
}

void QKlipperInstance::disconnect()
{
    if(m_console)
        m_console->disconnect();
}

void QKlipperInstance::setConnectionType(QKlipperServer::ConnectionType connectionType)
{
    if (m_connectionType == connectionType)
        return;

    if(m_server)
        m_server->setConnectionType(connectionType);

    m_connectionType = connectionType;
    emit connectionTypeChanged();
}

void QKlipperInstance::setConsole(QKlipperConsole *console)
{
    if (m_console == console)
        return;

    m_console = console;
    emit consoleChanged();
}

QString QKlipperInstance::profileColor() const
{
    return m_profileColor;
}

void QKlipperInstance::setProfileColor(const QString &profileColor)
{
    if (m_profileColor == profileColor)
        return;

    m_profileColor = profileColor;
    emit profileColorChanged();
}

void QKlipperInstance::onConsoleConnectionStateChanged()
{
    if(m_console && m_console->isConnected() && m_console->hasConnectionState(QKlipperConsole::Syncronized))
        setIsConnected(true);
    else
        setIsConnected(false);
}

void QKlipperInstance::onServerPrintJobAdded(QKlipperPrintJob *job)
{
    emit printJobAdded(this, job);
}

void QKlipperInstance::onServerPrintJobRemoved(QKlipperPrintJob *job)
{
    emit printJobRemoved(this, job);
}

void QKlipperInstance::onConsoleError(QKlipperError &error)
{
    if(!m_errors.contains(error.id()))
    {
        m_errors.insert(error.id(), error);
        emit this->error(this, error);
    }
}

bool QKlipperInstance::isConnected() const
{
    return m_isConnected;
}

void QKlipperInstance::setIsConnected(bool isConnected)
{
    if (m_isConnected == isConnected)
        return;

    if(isConnected && !m_isConnected)
    {
        m_isConnected = isConnected;
        emit connected(this);
    }
    else if(!isConnected && m_isConnected)
    {
        m_isConnected = isConnected;
        emit disconnected(this);
    }

    emit isConnectedChanged();
}

QKlipperPrinter *QKlipperInstance::printer() const
{
    return m_printer;
}

QKlipperSystem *QKlipperInstance::system() const
{
    return m_system;
}

QKlipperServer *QKlipperInstance::server() const
{
    return m_server;
}

QKlipperConsole *QKlipperInstance::console() const
{
    return m_console;
}
