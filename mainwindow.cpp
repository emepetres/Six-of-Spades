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
