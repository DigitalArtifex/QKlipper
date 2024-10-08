#include "qklippercommand.h"

QMap<QString, QKlipperCommand*> QKlipperCommand::m_commands;
bool QKlipperCommand::m_isInitialized = false;

QKlipperCommand::QKlipperCommand(QString command, QObject *parent)
    : QObject{parent}
{
    if(!m_isInitialized)
    {
        QFile commandsFile(QString(":/data/klipper_commands.json"));

        if(!commandsFile.open(QFile::ReadOnly))
        {
            qDebug() << QString("Could not open resource");
            return;
        }

        //Read the data from the resource
        QByteArray data = commandsFile.readAll();

        //Construct the document object
        QJsonDocument document;
        QJsonParseError documentError;

        document = QJsonDocument::fromJson(data, &documentError);

        //Check for errors
        if(documentError.error != QJsonParseError::NoError)
        {
            qDebug() << QString("Could not parse resource");
            return;
        }

        //Get klipper commands object
        QJsonObject rootObject = document["klipper_commands"].toObject();
        QStringList rootKeys = rootObject.keys();

        foreach(QString key, rootKeys)
        {
            //Parse through the objects of the section
            QJsonArray array = rootObject[key].toArray();

            for(int i = 0; i < array.count(); i++)
            {
                if(array[i].isObject())
                {
                    QJsonObject commandObject = array[i].toObject();

                    QKlipperCommand *command = new QKlipperCommand();
                    command->setCommand(commandObject["command"].toString());
                    command->setHelp(commandObject["help"].toString());

                    QJsonArray parametersArray = commandObject["parameters"].toArray();

                    for(int p = 0; p < parametersArray.count(); p++)
                        command->m_parameters += parametersArray[p].toString();

                    m_commands.insert(command->command().toUpper(), command);
                }
            }
        }
    }

    if(!command.isEmpty())
    {

    }
}

bool QKlipperCommand::isKlipperCommand(const QString &command)
{
    return m_commands.contains(command);
}

QString QKlipperCommand::command() const
{
    return m_command;
}

void QKlipperCommand::setCommand(const QString &command)
{
    if (m_command == command)
        return;

    m_command = command;
    emit commandChanged();
}

QString QKlipperCommand::help() const
{
    return m_help;
}

void QKlipperCommand::setHelp(const QString &help)
{
    if (m_help == help)
        return;

    m_help = help;
    emit helpChanged();
}

QStringList QKlipperCommand::parameters() const
{
    return m_parameters;
}

void QKlipperCommand::setParameters(const QStringList &parameters)
{
    if (m_parameters == parameters)
        return;

    m_parameters = parameters;
    emit parametersChanged();
}
