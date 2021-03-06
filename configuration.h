#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <qstring.h>
#include <qobject.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qfile.h>

class Configuration
{

private:
    QString _hostName;
    QString _databaseName;
    QString _userName;
    QString _password;
    bool _local;
    QString _local_name;

    bool closed;

public:
    QSqlDatabase *db;

    //Configuration(QString hostName, QString databaseName, QString userName, QString password);
    Configuration();

    bool OpenConfigDatabase();
    bool CloseDatabase();
    bool CloseDatabase(const QString &database);

    QString hostName() { return _hostName; }
    QString databaseName() { return _databaseName; }
    QString userName() { return _userName; }
    QString password() { return _password; }
    bool isLocal() { return _local; }
    QString localName() { return _local_name; }
};

#endif // CONFIGURATION_H
