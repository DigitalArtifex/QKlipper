#ifndef QKLIPPERCOMMAND_H
#define QKLIPPERCOMMAND_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

class QKlipperCommand : public QObject
{
    Q_OBJECT
public:
    explicit QKlipperCommand(QString command = QString(), QObject *parent = nullptr);
    static bool isKlipperCommand(const QString &command);

    QString command() const;

    QString help() const;

    QStringList parameters() const;

public slots:
    void setCommand(const QString &command);

    void setHelp(const QString &help);

    void setParameters(const QStringList &parameters);

signals:

    void commandChanged();

    void helpChanged();

    void parametersChanged();

private:
    static QMap<QString, QKlipperCommand*> m_commands;
    static bool m_isInitialized;

    QString m_command;
    QString m_help;
    QStringList m_parameters;

    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged FINAL)
    Q_PROPERTY(QString help READ help WRITE setHelp NOTIFY helpChanged FINAL)
    Q_PROPERTY(QStringList parameters READ parameters WRITE setParameters NOTIFY parametersChanged FINAL)
};

#endif // QKLIPPERCOMMAND_H
