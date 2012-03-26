#include "configuration.h"

#include <qsqlquerymodel.h>
#include <qsqlquery.h>
#include <QtSql/qsqldatabase.h>
#include <qmessagebox.h>

/*Configuration::Configuration(QString hostName, QString databaseName, QString userName, QString password)
{

    this->_hostName = hostName;
    this->_databaseName = databaseName;
    this->_userName = userName;
    this->_password = password;

}*/

bool Configuration::OpenConfigDatabase()
{
    if (closed)
    {
        //open/create sqlite configuration database
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
        db->setDatabaseName("config.sqlite");

        if (db->open())
        {
            //writes database if it is empty
            QSqlQuery query;
            QSqlQueryModel *table = new QSqlQueryModel;
            table->setQuery("SELECT name FROM sqlite_master WHERE type='table';");
            if (table->rowCount()==0)
            {
                //properties table
                query.exec ("CREATE TABLE `properties` (`property` VARCHAR(40) NOT NULL,`value` VARCHAR(40), PRIMARY KEY (`property`));");
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
            table->setQuery("select value from properties where property='local'");
            if (table->rowCount()==0) { query.exec("insert into properties (property, value) values ('local', '1');"); }

            //read properties
            query.exec("select value from properties where property='hostName';");
            query.next(); _hostName = query.value(0).toString();
            query.exec("select value from properties where property='databaseName';");
            query.next(); _databaseName = query.value(0).toString();
            query.exec("select value from properties where property='userName';");
            query.next(); _userName = query.value(0).toString();
            query.exec("select value from properties where property='password';");
            query.next(); _password = query.value(0).toString();
            query.exec("select value from properties where property='local';");
            query.next(); _local = query.value(0).toInt();

            query.clear();
            table->clear();

            closed = false;
            return true;
        } 
		else 
		{
			QMessageBox::warning(0,"No es posible configurar", db->lastError().text());
			return false;
		}
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
	closed = true;
    if (OpenConfigDatabase())
		CloseDatabase();
}
