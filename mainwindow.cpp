#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ticketdialog.h"
#include "userdialog.h"
#include "configuration.h"
#include "databasedialog.h"

#include <qmessagebox.h>
#include <QtSql/QSqlError>
#include <QtSql/qsqlquery.h>
#include <qstandarditemmodel.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    isDatabaseInitialized = false;
    ui->setupUi(this);

    //organize the window
    ui->endTicketDate->setDate(QDate::currentDate());

    config = new Configuration();

    getTickets();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete config;
    delete db;
}

bool MainWindow::connectDB()
{
    if (isDatabaseInitialized)
    {
        if (db->isValid() && db->isOpen()) return true;
        else closeDB();
    }

    db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL"));
    isDatabaseInitialized = true;

    db->setHostName(config->hostName());
    db->setDatabaseName(config->databaseName());
    db->setUserName(config->userName());
    db->setPassword(config->password());

    if (!db->open())
    {
        QMessageBox::critical(0,trUtf8("Error de conexión"), trUtf8("Vaya a \"Herramientas\"->\"Configurar BBDD\" e introduzca los datos de conexión válidos."));
        return false;
    }

    //writes database if it is empty
    QSqlQueryModel *table = new QSqlQueryModel;
    table->setQuery("show tables;");
    if (table->rowCount()==0) {
        QSqlQuery query;
        bool validQuery;

        //user table
        validQuery = query.exec ("CREATE TABLE `user` (`user` VARCHAR(40) NOT NULL,`passwd` VARCHAR(40), PRIMARY KEY (`user`)) ENGINE = InnoDB;");

        //ticket table
        validQuery = validQuery && query.exec ("CREATE TABLE `ticket` (`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,`concept` VARCHAR(256) NOT NULL,`created` DATE  NOT NULL,`payed` DATE ,`amount` FLOAT  NOT NULL DEFAULT 0,`user` VARCHAR(40) NOT NULL, PRIMARY KEY (`id`), CONSTRAINT `user_ticket` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");

        //ticket_user table
        validQuery = validQuery && query.exec ("CREATE TABLE `ticket_user` (`idTicket` INT UNSIGNED NOT NULL, `user` VARCHAR(40) NOT NULL,`percent` FLOAT UNSIGNED NOT NULL, PRIMARY KEY (`idTicket`, `user`), CONSTRAINT `ticket` FOREIGN KEY `ticket` (`idTicket`) REFERENCES `ticket` (`id`) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT `user` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");
        if (!validQuery)
        {
            QMessageBox::critical(0,"Database error",  query.lastError().text());
            return false;
        }
    }



    return true;
}

bool MainWindow::getTickets()
{
    if (connectDB())
    {
        QSqlQueryModel *ticketsTableModel = new QSqlQueryModel;
        ticketsTableModel->setQuery("SELECT id, payed, user, concept, amount, created FROM ticket LIMIT 0,50;");

        ticketsTableModel->setHeaderData(0, Qt::Horizontal, "ID");
        ticketsTableModel->setHeaderData(1, Qt::Horizontal, "Pagado el");
        ticketsTableModel->setHeaderData(2, Qt::Horizontal, "Por");
        ticketsTableModel->setHeaderData(3, Qt::Horizontal, "Concepto");
        ticketsTableModel->setHeaderData(4, Qt::Horizontal, "Cantidad");
        ticketsTableModel->setHeaderData(5, Qt::Horizontal, "Modificado el");

        ui->ticketsTable->setModel(ticketsTableModel);
        ui->ticketsTable->resizeColumnsToContents();
        int ticketsTableWidth = 672; //ui->ticketsTable->geometry().width();
        ui->ticketsTable->setColumnWidth(3, ticketsTableWidth - ui->ticketsTable->columnWidth(0) - ui->ticketsTable->columnWidth(1) - ui->ticketsTable->columnWidth(2) - ui->ticketsTable->columnWidth(4) - ui->ticketsTable->columnWidth(5) -15);
        ui->ticketsTable->setAlternatingRowColors(true);

        enableButtons(); //enable/disable buttons if there is some ticket selected or not

        return true;
    }
    else return false;
}

//Button slots
bool MainWindow::addTicket()
{
    //get users
    QSqlQueryModel userNameModel;
    if (connectDB()) userNameModel.setQuery("SELECT user FROM user;");
    else return false;

    TicketDialog td(&userNameModel, this);
    td.exec();

    getTickets();

    return true;
}

bool MainWindow::editTicket()
{
    //get users
    QSqlQueryModel userNameModel;
    if (connectDB()) userNameModel.setQuery("SELECT user FROM user;");
    else return false;

    //create the dialog with edit information
    TicketDialog td(&userNameModel, ui->ticketsTable->selectionModel(), this);
    td.exec();

    getTickets();

    return true;
}

bool MainWindow::deleteTicket()
{
    QModelIndexList ticketIndexes = ui->ticketsTable->selectionModel()->selectedIndexes();
    const QAbstractItemModel *ticketModel = ui->ticketsTable->model();

    int id = ticketModel->data(ticketIndexes.at(0)).toInt();

    QSqlQuery query;
    bool validQuery = query.exec ("delete from ticket where id="+QString::number(id));

    getTickets();

    return validQuery;
}

bool MainWindow::calculeBill()
{
    //SELECT idTicket, ticket.user, amount, ticket_user.user, percent FROM ticket, ticket_user WHERE id = idTicket AND payed>='2012-01-05' AND payed<='2012-02-13' ORDER BY idTicket ASC;

    QSqlQuery query;
    query.setForwardOnly(true);

    if (connectDB()) query.exec("SELECT idTicket, ticket.user, amount, ticket_user.user, percent FROM ticket, ticket_user WHERE id = idTicket AND payed>='" +
                                            ui->startTicketDate->date().toString("yyyy-MM-dd") + "' AND payed<='" +
                                            ui->endTicketDate->date().toString("yyyy-MM-dd") + "' ORDER BY idTicket ASC;");
    else return false;

    //bill data
    QMap<QString, float> bill;

    int id, last_id = -1;
    QString user;
    float cost = 0;
    vector <QString> contributors;
    vector <float> contributorsPercentages;
    float percentAccum = 0;
    int contributorsNotEqual = 0;
    bool thereAreMore = true;
    while (query.size()) {
        thereAreMore = query.next();

        if (thereAreMore) id = query.value(0).toInt();
        else id = -1;

        if (id != last_id)
        {
            if (last_id>=0)
            {
                //if it is not first time, update the bill with the ticket
                if (contributorsNotEqual==0) {
                    float percentMoney = cost / contributors.size();
                    for (unsigned int i=0; i<contributors.size(); i++) {
                        bill[contributors[i]] = bill[contributors[i]] - percentMoney;
                    }
                    bill[user] = bill[user] + cost;
                }
                else
                {
                    float percentMoney = cost*(100.0 - percentAccum)/(contributors.size() - contributorsNotEqual)/100.0;
                    for (unsigned int i=0; i<contributors.size(); i++) {
                        if (contributorsPercentages[i]>0)
                            bill[contributors[i]] = bill[contributors[i]] - cost*contributorsPercentages[i]/100.0;
                        else bill[contributors[i]] = bill[contributors[i]] - percentMoney;
                    }
                    bill[user] = bill[user] + cost;
                }
            }
            if (!thereAreMore) break;

            user = query.value(1).toString();
            cost = query.value(2).toFloat();
            last_id = id;

            contributors.clear();
            contributorsPercentages.clear();
            contributorsNotEqual = 0;
            percentAccum = 0;
        }

        //read and save the contributor information
        contributors.push_back(query.value(3).toString());
        contributorsPercentages.push_back(query.value(4).toFloat());
        if (query.value(4).toFloat()>0)
        {
            contributorsNotEqual++;
            percentAccum = percentAccum + query.value(4).toFloat();
        }
    }

    QString billText;
    QMap<QString, float>::iterator it;
    for (it = bill.begin(); it != bill.end(); ++it)
        billText = billText + it.key() + QString(": ") + QString::number(it.value()) + QString("\n");

    ui->billTextBox->setText(billText);

    return true;
}

bool MainWindow::enableButtons()
{
    //enable edit and remove buttons if we have a selection on ticketsTable
    bool enable = ui->ticketsTable->selectionModel()->hasSelection();
    ui->edit_Button->setEnabled(enable);
    ui->remove_Button->setEnabled(enable);

    return enable;
}


//Menu bar slots
void MainWindow::on_actionAddBuddy_triggered()
{
    UserDialog ud(this);
    ud.exec();

    getTickets();
}

void MainWindow::on_actionConfigurar_BBDD_triggered()
{
    //remove old connection
    closeDB();

    DatabaseDialog dd(this->config, this);
    int result = dd.exec();

    delete config;
    config = new Configuration();

    if(result==QDialog::Accepted && connectDB()) getTickets();

}

bool MainWindow::closeDB()
{
    QString connectionName;
    connectionName = db->connectionName();
    db->close();
    delete db;
    isDatabaseInitialized = false;
    db->removeDatabase(connectionName);

    return true;
}
