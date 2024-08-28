#include "qklipperinstance.h"

QKlipperInstance::QKlipperInstance(QObject *parent)
    : QObject{parent}
{
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
    m_printer->setConsole(m_console);

    m_console->setSystem(m_system);
    m_console->setServer(m_server);
    m_console->setPrinter(m_printer);

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

    if(m_instanceLocation.isEmpty())
    {
        m_server->setAddress(m_address);
        m_server->setPort(m_port);
        m_server->setConnectionType(QKlipperServer::Remote);
    }
    else
    {
        m_server->setAddress(m_address);
        m_server->setPort(m_port);
        m_server->setInstanceLocation(m_instanceLocation);
        m_server->setConnectionType(QKlipperServer::Local);
    }

    connect(m_printer, SIGNAL(statusMessageChanged()), this, SLOT(printerStateChanged()));

    if(m_autoConnect)
        m_console->connect();
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

    m_apiKey = apiKey;
    emit apiKeyChanged();
}

bool QKlipperInstance::autoConnect() const
{
    return m_autoConnect;
}

void QKlipperInstance::setAutoConnect(bool autoConnect)
{
    if (m_autoConnect == autoConnect)
        return;
    m_autoConnect = autoConnect;
    emit autoConnectChanged();
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

void QKlipperInstance::init()
{
    reset();
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
