#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ticketdialog.h"
#include "userdialog.h"

#include <QMessageBox>
#include <QtSql/QSqlError>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    getTickets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::connectDB()
{
    if (db.isValid())
    {
        if (db.isOpen()) return true;
    }
    else db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("localhost");
    db.setDatabaseName("seisdepicas");
    db.setUserName("piso");
    db.setPassword("miketgood");

    if (!db.open())
    {
        QMessageBox::critical(0,"Database error", db.lastError().text());
        return false;
    }

    //writes database if it is empty
    QSqlQueryModel *table = new QSqlQueryModel;
    table->setQuery("show tables;");
    if (table->rowCount()==0) {
        QSqlQuery query;
        bool validQuery;

        //user table
        validQuery = query.exec ("CREATE TABLE `seisdepicas`.`user` (`user` VARCHAR(40) NOT NULL,`passwd` VARCHAR(40), PRIMARY KEY (`user`)) ENGINE = InnoDB;");

        //ticket table
        validQuery = validQuery && query.exec ("CREATE TABLE `seisdepicas`.`ticket` (`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,`concept` VARCHAR(256) NOT NULL,`created` DATE  NOT NULL,`payed` DATE ,`amount` INT  NOT NULL DEFAULT 0,`user` VARCHAR(40) NOT NULL, PRIMARY KEY (`id`), CONSTRAINT `user_ticket` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");

        //ticket_user table
        validQuery = validQuery && query.exec ("CREATE TABLE `seisdepicas`.`ticket_user` (`idTicket` INT UNSIGNED NOT NULL, `user` VARCHAR(40) NOT NULL,`percent` FLOAT UNSIGNED NOT NULL, PRIMARY KEY (`idTicket`, `user`), CONSTRAINT `ticket` FOREIGN KEY `ticket` (`idTicket`) REFERENCES `ticket` (`id`) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT `user` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");
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
        ticketsTableModel->setQuery("SELECT * FROM ticket LIMIT 0,50;");

        ticketsTableModel->setHeaderData(0, Qt::Horizontal, "ID");
        ticketsTableModel->setHeaderData(1, Qt::Horizontal, "Concepto");
        ticketsTableModel->setHeaderData(2, Qt::Horizontal, "Creado");
        ticketsTableModel->setHeaderData(3, Qt::Horizontal, "Pagado");
        ticketsTableModel->setHeaderData(4, Qt::Horizontal, "Cantidad");
        ticketsTableModel->setHeaderData(5, Qt::Horizontal, "Por");

        ui->ticketsTable->setModel(ticketsTableModel);
        ui->ticketsTable->resizeColumnsToContents();
        ui->ticketsTable->setAlternatingRowColors(true);

        return true;
    }
    else return false;
}

bool MainWindow::addTicket()
{
    QSqlQueryModel userNameModel;

    if (connectDB()) userNameModel.setQuery("SELECT user FROM user;");
    else return false;

    TicketDialog td(this, &userNameModel);
    td.exec();

    getTickets();

    return true;
}

void MainWindow::on_actionAddBuddy_triggered()
{
    QSqlQueryModel userNameModel;

    if (connectDB()) userNameModel.setQuery("SELECT user FROM user;");
    else return;

    UserDialog ud(this);
    ud.exec();
}
