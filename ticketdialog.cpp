#include <qobject.h>
#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include <qgridlayout.h>
#include <qsqlrecord.h>
#include <qsqlquery.h>

TicketDialog::TicketDialog(QSqlQueryModel *userModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TicketDialog)
{
    ui->setupUi(this);
    organizeDialog(userModel);

    //write the add new ticket buttons
    if (usersNumber>0) {
        ((QGridLayout *)layout())->addWidget(ui->cancel_Button, usersNumber+5, 1);
        ((QGridLayout *)layout())->addWidget(ui->add_Button, usersNumber+5, 2);
        ((QGridLayout *)layout())->addWidget(ui->addMore_Button, usersNumber+5, 3);
    }
}

TicketDialog::TicketDialog(QSqlQueryModel *userModel, QItemSelectionModel *ticketSelectionModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TicketDialog)
{
    ui->setupUi(this);

    if (organizeDialog(userModel)) { //if the widget organization has success (users > 0)
        //write the edit buttons
        ((QGridLayout *)layout())->addWidget(ui->cancel_Button, usersNumber+5, 2);
        ui->add_Button->setText("Editar");
        ((QGridLayout *)layout())->addWidget(ui->add_Button, usersNumber+5, 3);
        ui->addMore_Button->hide();
        //TODO: PROVISIONAL
        ui->add_Button->setEnabled(false);

        //get the fields of the ticket to edit
        QModelIndexList ticketIndexes = ticketSelectionModel->selectedIndexes();
        const QAbstractItemModel *ticketModel = ticketSelectionModel->model();

        int id = ticketModel->data(ticketIndexes.at(0)).toInt();
        QDate payed = ticketModel->data(ticketIndexes.at(1)).toDate();
        QString user = ticketModel->data(ticketIndexes.at(2)).toString();
        QString concept = ticketModel->data(ticketIndexes.at(3)).toString();
        float amount = ticketModel->data(ticketIndexes.at(4)).toFloat();


        //write the fields of the dialog with the information to edit
        ui->comboBox->setCurrentIndex(ui->comboBox->findText(user));
        ui->conceptLineEdit->setText(concept);
        ui->costLineEdit->setText(QString::number(amount));
        ui->dateEdit->setDate(payed);

        QSqlQuery query;
        query.exec ("select user, percent from ticket_user where idTicket='"+ QString::number(id) +"';");
        while (query.next())
        {
            QString _user = query.value(0).toString();
            for (int i=0; i<usersNumber; ++i)
                if (usersCheckBoxes[i]->text()==_user)
                {
                    usersCheckBoxes[i]->setChecked(true);
                    if (query.value(1).toString()=="0") percentages[i]->setText("igual");
                    else percentages[i]->setText(query.value(1).toString());
                    break;
                }
        }
    }
}

TicketDialog::~TicketDialog()
{
    delete ui;
    usersCheckBoxes.clear();
    percentages.clear();
}

bool TicketDialog::organizeDialog(QSqlQueryModel *userModel)
{
    ui->comboBox->setModel(userModel);
    ui->comboBox->setModelColumn(0);
    ui->dateEdit->setDate(QDate::currentDate());

    QGridLayout *gridLayout = (QGridLayout *)layout();

    //remove items added by the designer, to push them at the bottom of the dialog
    gridLayout->removeWidget(ui->cancel_Button);
    gridLayout->removeWidget(ui->add_Button);
    gridLayout->removeWidget(ui->addMore_Button);
    gridLayout->removeWidget(ui->errorLabel);

    //see if we have users
    usersNumber = userModel->rowCount();
    usersCheckBoxes.reserve(usersNumber);
    percentages.reserve(usersNumber);
    if (usersNumber==0)
    {
        //add error label and cancel button
        ui->errorLabel->setText(trUtf8("Error: Por favor, añade un usuario primero."));
        gridLayout->addWidget(ui->errorLabel, 4, 0, 1, 4);

        gridLayout->addWidget(ui->cancel_Button, 5, 3);

        ui->add_Button->hide();
        ui->addMore_Button->hide();

        return false;
    }
    else
    {
        //add every user as possible ticket's parcipant
        QCheckBox *checkBox;
        QLineEdit *lineEdit;
        QLabel *percent;
        for (int i = 0; i < usersNumber; i++) {
            checkBox = new QCheckBox(userModel->record(i).value(0).toString(), this);
            checkBox->setLayoutDirection(Qt::RightToLeft);
            gridLayout->addWidget(checkBox, i+4, 1);
            usersCheckBoxes.push_back(checkBox);

            lineEdit = new QLineEdit("igual");
            gridLayout->addWidget(lineEdit, i+4, 2);
            percentages.push_back(lineEdit);

            percent = new QLabel("%");
            gridLayout->addWidget(percent, i+4, 3);
        }

        //finally add error label
        gridLayout->addWidget(ui->errorLabel, usersNumber+4, 0, 1, 4);

        return true;
    }
}

bool TicketDialog::addTicket()
{
    QString written = writeTicket();

    if (written.compare("")==0)
    {
        QDialog::accept();
        return true;
    }
    else
    {
        ui->errorLabel->setText(written);
        return false;
    }
}

bool TicketDialog::addMoreTickets()
{
    QString written = writeTicket();

    if (written.compare("")==0)
    {
        ui->errorLabel->setText("");
        ui->conceptLineEdit->setText("");
        ui->costLineEdit->setText("");
        ui->dateEdit->setDate(QDate::currentDate());

        for (int i=0; i<usersNumber; i++) {
            usersCheckBoxes[i]->setChecked(false);
            percentages[i]->setText("igual");
        }

        return true;
    }
    else return false;
}

QString TicketDialog::writeTicket()
{
    bool isnumber, validQuery;
    QString cost = ui->costLineEdit->text();

    //check that all number fields have numbers (cost and percentages)
    cost.toFloat(&isnumber);
    if (!isnumber) return "Error: El coste no es un numero valido.";

    float percent;
    for (int i=0; i<usersNumber && isnumber; i++) {
        if (usersCheckBoxes[i]->isChecked() && percentages[i]->text().compare("igual")!=0)
        {
            percent = percentages[i]->text().toFloat(&isnumber);
            isnumber = isnumber && percent > 0 && percent < 100;
            if (!isnumber) return "El porcentaje " + QString::number(i+1) + trUtf8(" no es válido");
        }
    }


    //write the ticket
    QString user = ui->comboBox->currentText();
    QString concept = ui->conceptLineEdit->text();
    QDate emission_date = ui->dateEdit->date();
    QSqlQuery query;
    validQuery = query.exec ("insert into ticket (concept, created, payed, amount, user) VALUES ('"+
                         concept +"', '"+
                         QDate::currentDate().toString("yyyy-MM-dd") +"', '"+
                         emission_date.toString("yyyy-MM-dd") +"', "+
                         cost +", '"+
                         user +"');");
    if (!validQuery) return "Datos de ticket no válidos.";
    int ticketId = query.lastInsertId().toInt();


    //write the contributors of the ticket
    QString percentage;
    for (int i=0; i<usersNumber && validQuery; i++) {
        if (usersCheckBoxes[i]->isChecked()) {
            if (percentages[i]->text().compare("igual")==0) percentage = "0";
            else percentage = percentages[i]->text();

            validQuery = validQuery && query.exec ("insert into ticket_user (idTicket, user, percent) VALUES ("+
                                           QString::number(ticketId) + ", '"+
                                           usersCheckBoxes[i]->text() + "', " +
                                           percentage +");");
        }
    }

    if (!validQuery) //delete the new ticket and return the error message
    {
        query.exec ("delete from ticket where id="+QString::number(ticketId));
        return trUtf8("Datos de compañer@s de ticket no válidos.");
    }
    else return "";

}
