#include <QObject>
#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include "qgridlayout.h"

TicketDialog::TicketDialog(QWidget *parent, QSqlQueryModel *userModel) :
    QDialog(parent),
    ui(new Ui::TicketDialog)
{
    ui->setupUi(this);

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
            usersCheckBoxes[i] = checkBox;

            lineEdit = new QLineEdit("igual");
            gridLayout->addWidget(lineEdit, i+4, 2);
            percentages[i] = lineEdit;

            percent = new QLabel("%");
            gridLayout->addWidget(percent, i+4, 3);
        }

        //finally add the buttons and error label
        gridLayout->addWidget(ui->errorLabel, usersNumber+4, 0, 1, 4);
        gridLayout->addWidget(ui->cancel_Button, usersNumber+5, 1);
        gridLayout->addWidget(ui->add_Button, usersNumber+5, 2);
        gridLayout->addWidget(ui->addMore_Button, usersNumber+5, 3);
    }

}

TicketDialog::~TicketDialog()
{
    delete ui;
    usersCheckBoxes.clear();
    percentages.clear();
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
        if (usersCheckBoxes[i]->isChecked() && usersCheckBoxes[i]->text().compare("equal")!=0)
        {
            percent = percentages[i]->text().toFloat(&isnumber);
            isnumber = isnumber && percent > 0 && percent < 100;
            if (!isnumber) return "El porcentaje " + QString::number(i+1) + "no es válido";
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
    for (int i=0; i<usersNumber && validQuery; i++) {
        if (usersCheckBoxes[i]->isChecked()) {
            validQuery = validQuery && query.exec ("insert into ticket_user (idTicket, user, percent) VALUES ("+
                                           QString::number(ticketId) + ", '"+
                                           usersCheckBoxes[i]->text() + "'', " +
                                           percentages[i]->text() +");");
        }
    }

    if (!validQuery) return "Datos de compañer@s de ticket no válidos.";
    else return "";

}
