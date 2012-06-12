#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ticketdialog.h"
#include "userdialog.h"
#include "configuration.h"
#include "databasedialog.h"
#include "about.h"
#include "localdatabasedialog.h"

#include <qmessagebox.h>
#include <QtSql/qsqlquery.h>
#include <qstandarditemmodel.h>
#include <qtextstream.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    isDatabaseInitialized = false;
    ui->setupUi(this);

    //write the TicketsView
    //ticketsView = new TicketsView();
    /*QTableView *list = new QTableView(0);
    list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout *gridLayout = (QGridLayout *)layout();
    gridLayout->addWidget(list, 1, 0, Qt::AlignLeft);*/

    setHelp();

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
    //delete ticketsView;
}

bool MainWindow::connectDB()
{
    if (isDatabaseInitialized)
    {
        if (db->isValid() && db->isOpen()) return true;
        else closeDB();
    }

    QSqlQueryModel *table;
    QSqlQuery *query;
    bool validQuery = true;

    if (config->isLocal()) //local database
    {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
        db->setDatabaseName(config->localName());

        if (!db->open()) { QMessageBox::critical(0,trUtf8("Error de conexión local"), db->lastError().text()); return false; }

        //writes database if it is empty
        query = new QSqlQuery;
        table = new QSqlQueryModel;
        table->setQuery("SELECT name FROM sqlite_master WHERE type='table';");
        if (table->rowCount()==0)
        {
            //user table
            validQuery = query->exec ("CREATE TABLE user (user VARCHAR(40) NOT NULL PRIMARY KEY, passwd VARCHAR(40));");

            //ticket table
            validQuery = validQuery && query->exec ("CREATE TABLE ticket (id INTEGER PRIMARY KEY AUTOINCREMENT, concept VARCHAR(256) NOT NULL, created DATE NOT NULL,payed DATE ,amount FLOAT  NOT NULL DEFAULT 0, user VARCHAR(40) NOT NULL, FOREIGN KEY(user) REFERENCES user (user) ON DELETE CASCADE ON UPDATE CASCADE);");

            //ticket_user table
            validQuery = validQuery && query->exec ("CREATE TABLE ticket_user (idTicket INTEGER NOT NULL, user VARCHAR(40) NOT NULL, percent FLOAT UNSIGNED NOT NULL, FOREIGN KEY(idTicket) REFERENCES ticket (id) ON DELETE CASCADE ON UPDATE CASCADE, FOREIGN KEY (user) REFERENCES user (user) ON DELETE CASCADE ON UPDATE CASCADE, PRIMARY KEY(idTicket, user));");
        }
    }
    else
    {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL"));
        db->setHostName(config->hostName());
        db->setDatabaseName(config->databaseName());
        db->setUserName(config->userName());
        db->setPassword(config->password());

        if (!db->open()) { QMessageBox::warning(0,trUtf8("Error de conexión"), trUtf8("Vaya a \"Herramientas\"->\"Configurar BBDD\" e introduzca los datos de conexión válidos.")); return false; }

        //writes database if it is empty
        query = new QSqlQuery;
        table = new QSqlQueryModel;
        table->setQuery("show tables;");
        if (table->rowCount()==0)
        {
            //user table
            validQuery = query->exec ("CREATE TABLE `user` (`user` VARCHAR(40) NOT NULL,`passwd` VARCHAR(40), PRIMARY KEY (`user`)) ENGINE = InnoDB;");

            //ticket table
            validQuery = validQuery && query->exec ("CREATE TABLE `ticket` (`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,`concept` VARCHAR(256) NOT NULL,`created` DATE NOT NULL,`payed` DATE ,`amount` FLOAT  NOT NULL DEFAULT 0, `user` VARCHAR(40) NOT NULL, PRIMARY KEY (`id`), CONSTRAINT `user_ticket` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");

            //ticket_user table
            validQuery = validQuery && query->exec ("CREATE TABLE `ticket_user` (`idTicket` INT UNSIGNED NOT NULL, `user` VARCHAR(40) NOT NULL,`percent` FLOAT UNSIGNED NOT NULL, PRIMARY KEY (`idTicket`, `user`), CONSTRAINT `ticket` FOREIGN KEY `ticket` (`idTicket`) REFERENCES `ticket` (`id`) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT `user` FOREIGN KEY `user` (`user`) REFERENCES `user` (`user`) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE = InnoDB;");
        }
    }
    isDatabaseInitialized = true;
    delete table;

    if (!validQuery)
    {
        QMessageBox::critical(0,"Database error",  query->lastError().text());
        delete query;
        return false;
    }
    delete query;

    return true;
}

bool MainWindow::getTickets()
{
    if (connectDB())
    {
        QSqlQueryModel *ticketsTableModel = new QSqlQueryModel;
        ticketsTableModel->setQuery("SELECT id, payed, user, concept, amount, created FROM ticket ORDER BY id DESC;");

        ticketsTableModel->setHeaderData(0, Qt::Horizontal, "ID");
        ticketsTableModel->setHeaderData(1, Qt::Horizontal, "Pagado el");
        ticketsTableModel->setHeaderData(2, Qt::Horizontal, "Por");
        ticketsTableModel->setHeaderData(3, Qt::Horizontal, "Concepto");
        ticketsTableModel->setHeaderData(4, Qt::Horizontal, "Cantidad");
        ticketsTableModel->setHeaderData(5, Qt::Horizontal, "Modificado el");

        ui->ticketsTable->setModel(ticketsTableModel);
        ui->ticketsTable->setAlternatingRowColors(true);

        //logic to resize colums
        ui->ticketsTable->resizeColumnsToContents();
        int ticketsTableWidth = ui->ticketsTable->width();
        if (ticketsTableWidth!=100) //widget already painted
        {
            int not_concept_width = ui->ticketsTable->horizontalHeader()->length() - ui->ticketsTable->columnWidth(3);
            ui->ticketsTable->setColumnWidth(3, ticketsTableWidth - not_concept_width);
        }

        //ui->ticketsTable->horizontalHeader()->width()->setResizeMode(QHeaderView::Stretch);
        //ui->ticketsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

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
    QSqlQuery query;
    query.setForwardOnly(true);

    if (connectDB()) query.exec("SELECT idTicket, ticket.user, amount, ticket_user.user, percent, concept, payed FROM ticket, ticket_user WHERE id = idTicket AND payed>='" +
                                ui->startTicketDate->date().toString("yyyy-MM-dd") + "' AND payed<='" +
                                ui->endTicketDate->date().toString("yyyy-MM-dd") + "' ORDER BY idTicket ASC;");
    else return false;

    bool thereAreMore = query.next();
    if (thereAreMore)
    {

        startBillDate = ui->startTicketDate->date();
        endBillDate = ui->endTicketDate->date();

        //bill data
        QMap<QString, float> bill;
        QString ticketsDetail;

        int id, last_id = -1;
        QString user;
        float cost = 0;
        vector <QString> contributors;
        vector <float> contributorsPercentages;
        float percentAccum = 0;
        int contributorsNotEqual = 0;
        bool thereAreMore = true;
        while (true) //the loop breaks itself
        {
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
                ticketsDetail = ticketsDetail + "\nTicket " + QString::number(id) + ", pagado por "+user+" el "+query.value(6).toDate().toString("dd-MM-yyyy")+": "+query.value(5).toString()+", "+QString::number(cost)+trUtf8("€\n");

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
                ticketsDetail = ticketsDetail + "\t- " + query.value(3).toString() + " contribuye en un "+query.value(4).toString()+trUtf8("%.\n");
            }
            else ticketsDetail = ticketsDetail + "\t- " + query.value(3).toString() + " contribuye igual.\n";

            thereAreMore = query.next();
        }

        //write bill text
        QString billText("Cuentas comunes calculadas el " + QDate::currentDate().toString("dd-MM-yyyy") +
                         " a las " + QTime::currentTime().toString("hh:mm:ss") +
                         trUtf8(", desde el día ")+ ui->startTicketDate->date().toString("dd-MM-yyyy") +
                         trUtf8(" hasta el día ")+ ui->endTicketDate->date().toString("dd-MM-yyyy")+
                         ", ambos incluidos.\n\n");
        QMap<QString, float>::iterator it;
        for (it = bill.begin(); it != bill.end(); ++it)
            if (it.value()<0) billText = billText + it.key() + QString(" debe ") + QString::number(-it.value()) + QString(trUtf8("€ \n"));
            else billText = billText + it.key() + QString(" recibe ") + QString::number(it.value()) + QString(trUtf8("€ \n"));

        billText = billText + "\n" + ticketsDetail;

        ui->billTextBox->setText(billText);
        ui->saveButton->setEnabled(true);
        ui->saveAndRemoveButton->setEnabled(true);

        return true;
    }
    else
    {
        ui->billTextBox->setText("No existen tickets disponibles.");

        return false;
    }
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

bool MainWindow::saveBill()
{
    QFile file("cuentas " + QDate::currentDate().toString("yyyy-MM-dd") + ", " + QTime::currentTime().toString("hh'h'mm'm'ss's'")+".txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << ui->billTextBox->toPlainText() << "\n\n Creado por Seis de Picas.";

        file.close();
        return true;
    }
    else 
	{
            QMessageBox::warning(0,trUtf8("Error guardando las cuentas"), trUtf8("No se han podido guardar las cuentas."));
            return false;
	}
}

bool MainWindow::saveAndRemoveBill()
{
    int res = QMessageBox::question(0,trUtf8("Información"), trUtf8("Esta operación borrara todos los tickets, por lo que no se podrá modificar estas cuentas una vez aceptada la operación.\n¿Desea usted continuar?"),
                                    QMessageBox::Ok,QMessageBox::Cancel);
    if (res==QMessageBox::Ok && saveBill())
    {
        QSqlQuery query;
        bool valid;
        if (connectDB())
        {
            valid = query.exec("DELETE FROM ticket WHERE payed>='" + startBillDate.toString("yyyy-MM-dd") + "' AND payed<='" + endBillDate.toString("yyyy-MM-dd") + "';");
            getTickets();
            if (valid)
            {
                setHelp();
                ui->saveButton->setDisabled(true);
                ui->saveAndRemoveButton->setDisabled(true);
            }
            return valid;
        }
        else return false;
    } else return false;
}

void MainWindow::changeDataBase(bool local)
{
    //remove old connection
    closeDB();

    int result;
    if (local)
    {
        localdatabasedialog ldd(this->config, this);
        result = ldd.exec();
    }
    else
    {
        DatabaseDialog dd(this->config, this);
        result = dd.exec();
    }


    //open new connection (accepted or rejected we have to do that, as we lost previous one)
    delete config;
    config = new Configuration();

    if(connectDB()) getTickets();

    if (result==QDialog::Accepted)
    {
        setHelp();
        ui->saveButton->setDisabled(true);
        ui->saveAndRemoveButton->setDisabled(true);
    }
}

void MainWindow::on_actionAbrir_triggered()
{
    changeDataBase(true);
}

void MainWindow::on_actionAbrir_remoto_triggered()
{
    changeDataBase(false);
}

void MainWindow::on_actionSobre_Seis_De_Picas_triggered()
{
    About ab;
    ab.exec();
}

void MainWindow::setHelp()
{
    QString help = trUtf8("<p>Gestiona las cuentas compartidas fácilmente.</p>");
    ui->billTextBox->setText(help);
}
