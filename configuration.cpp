#include "configuration.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QtSql/QSqlDatabase>

Configuration::Configuration(QString hostName, QString databaseName, QString userName, QString password)
{

    this->_hostName = hostName;
    this->_databaseName = databaseName;
    this->_userName = userName;
    this->_password = password;

}

bool Configuration::OpenConfigDatabase()
{
    if (closed)
    {
        //open/create sqlite configuration database
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
        db->setDatabaseName("config.sqlite");
        QString error = db->lastError().text();

        if (db->open())
        {
            //writes database if it is empty
            QSqlQuery query;
            QSqlQueryModel *table = new QSqlQueryModel;
            table->setQuery("show tables;");
            if (table->rowCount()==0)
            {
                bool validQuery;

                //properties table
                validQuery = query.exec ("CREATE TABLE `properties` (`property` VARCHAR(40) NOT NULL,`value` VARCHAR(40), PRIMARY KEY (`property`));");
            }

            //write properties if the are missing
            table->setQuery("select value from properties where property='hostName'");
            if (table->rowCount()==0) { query.exec("insert into properties (property, value) values ('hostName', '-');"); }
            table->setQuery("select value from properties where property='databaseName'");
            if (table->rowCount()==0) { query.exec("insert into properties (property, value) values ('databaseName', '-');"); }
            table->setQuery("select value from properties where property='userName'");
            if (table->rowCount()==0) { query.exec("insert into properties (property, value) values ('userName', '-');"); }
            table->setQuery("select value from properties where property='password'");
            if (table->rowCount()==0) { query.exec("insert into properties (property, value) values ('password', '-');"); }

            error = table->lastError().text();

            //read properties
            query.exec("select value from properties where property='hostName';");
            query.next(); _hostName = query.value(0).toString();
            query.exec("select value from properties where property='databaseName';");
            query.next(); _databaseName = query.value(0).toString();
            query.exec("select value from properties where property='userName';");
            query.next(); _userName = query.value(0).toString();
            query.exec("select value from properties where property='password';");
            query.next(); _password = query.value(0).toString();

            error = table->lastError().text();

            query.clear();
            table->clear();

            closed = false;
            return true;
        } else return false;
    } else return false;
}

bool Configuration::CloseDatabase()
{
    if (!closed)
    {
        QString connectionName;
        connectionName = db->connectionName();
        db->close();
        delete db;
        db->removeDatabase(connectionName);
        closed = true;

        return true;
    }
    else return false;
}

Configuration::Configuration()
{
    OpenConfigDatabase();

    CloseDatabase();
}
