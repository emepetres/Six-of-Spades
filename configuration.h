#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QFile>

class Configuration
{

private:
    QString _hostName;
    QString _databaseName;
    QString _userName;
    QString _password;

    bool closed;

public:
    QSqlDatabase *db;

    Configuration(QString hostName, QString databaseName, QString userName, QString password);
    Configuration();

    bool OpenConfigDatabase();
    bool CloseDatabase();
    bool CloseDatabase(const QString &database);

    QString hostName() { return _hostName; }
    QString databaseName() { return _databaseName; }
    QString userName() { return _userName; }
    QString password() { return _password; }
};

#endif // CONFIGURATION_H
