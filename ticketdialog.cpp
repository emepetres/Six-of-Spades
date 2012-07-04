#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include <qobject.h>
#include <qgridlayout.h>
#include <qsqlrecord.h>
#include <qsqlquery.h>
#include <QDebug>
#include <mainwindow.h>

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

    editDialog = false;
    isEditing = false;
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

        //get the fields of the ticket to edit
        QModelIndexList ticketIndexes = ticketSelectionModel->selectedIndexes();
        const QAbstractItemModel *ticketModel = ticketSelectionModel->model();

        id = ticketModel->data(ticketIndexes.at(0)).toInt();
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
        bool equalContribution = true;
        query.exec ("select user, percent from ticket_user where idTicket='"+ QString::number(id) +"';");
        while (query.next())
        {
            QString _user = query.value(0).toString();
            for (int i=0; i<usersNumber; ++i)
            {
                if (usersCheckBoxes[i]->text()==_user)
                {
                    usersCheckBoxes[i]->setChecked(true);
                    if (query.value(1).toString()=="0") userTicketData[i]->setText(trUtf8("automático"));
                    else { userTicketData[i]->setText(query.value(1).toString()); equalContribution = false; }
                    break;
                }
            }
        }
        if (equalContribution) ui->contributionCheckBox->setChecked(true);
        else ui->contributionCheckBox->setChecked(false);

        //deactivate all dialogs
        ui->comboBox->setDisabled(true);
        ui->conceptLineEdit->setDisabled(true);
        ui->costLineEdit->setDisabled(true);
        ui->dateEdit->setDisabled(true);
        ui->contributionCheckBox->setDisabled(true);
        for (int i=0; i<usersNumber; ++i)
        {
            usersCheckBoxes[i]->setDisabled(true);
            userTicketData[i]->setDisabled(true);
        }
        editDialog = true;
        isEditing = false;
    }
}

TicketDialog::~TicketDialog()
{
    delete ui;
    usersCheckBoxes.clear();
    userTicketData.clear();
}

bool TicketDialog::organizeDialog(QSqlQueryModel *userModel)
{
    ui->comboBox->setModel(userModel);
    ui->comboBox->setModelColumn(0);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->contributionCheckBox->setChecked(true);

    QGridLayout *gridLayout = (QGridLayout *)layout();

    //remove items added by the designer, to push them at the bottom of the dialog
    gridLayout->removeWidget(ui->cancel_Button);
    gridLayout->removeWidget(ui->add_Button);
    gridLayout->removeWidget(ui->addMore_Button);
    gridLayout->removeWidget(ui->errorLabel);

    //see if we have users
    usersNumber = userModel->rowCount();
    usersCheckBoxes.reserve(usersNumber);
    userTicketData.reserve(usersNumber);
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
        QLabel *label;

        //info labels
        label = new QLabel("contribuye en:");
        gridLayout->addWidget(label, 4, 2);

        for (int i = 0; i < usersNumber; i++)
        {
            checkBox = new QCheckBox(userModel->record(i).value(0).toString(), this);
            checkBox->setLayoutDirection(Qt::RightToLeft);
            gridLayout->addWidget(checkBox, i+5, 1);
            usersCheckBoxes.push_back(checkBox);

            lineEdit = new QLineEdit(trUtf8("automático"));
            gridLayout->addWidget(lineEdit, i+5, 2);
            lineEdit->setDisabled(true);
            userTicketData.push_back(lineEdit);
        }

        //finally add error label
        gridLayout->addWidget(ui->errorLabel, usersNumber+4, 0, 1, 4);

        return true;
    }
}

bool TicketDialog::changeContribution()
{
    if (ui->contributionCheckBox->isChecked()) {
        for (int i = 0; i < usersNumber; i++)
        {
            userTicketData[i]->setDisabled(true);
        }
    }
    else
    {
        for (int i = 0; i < usersNumber; i++)
        {
            userTicketData[i]->setEnabled(true);
        }
    }

    return true;
}

bool TicketDialog::addTicket()
{
    if (!editDialog)
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
    else
    {
        if (!isEditing)
        {
            ui->add_Button->setText("Guardar");
            isEditing = true;

            //activate all fields
            ui->comboBox->setEnabled(true);
            ui->conceptLineEdit->setEnabled(true);
            ui->costLineEdit->setEnabled(true);
            ui->dateEdit->setEnabled(true);
            ui->contributionCheckBox->setEnabled(true);
            for (int i=0; i<usersNumber; ++i)
            {
                usersCheckBoxes[i]->setEnabled(true);
                if (!ui->contributionCheckBox->isChecked()) userTicketData[i]->setEnabled(true);
            }
            return true;
        }
        else
        {
            QString updated = updateTicket();
            if (updated.compare("")==0)
            {
                QDialog::accept();
                return true;
            }
            else
            {
                ui->errorLabel->setText(updated);
                return false;
            }
        }
    }
}

bool TicketDialog::addMoreTickets()
{
    QString written = writeTicket();

    if (written.compare("")==0)
    {
        ((MainWindow*)parent())->getTickets();
        ui->errorLabel->setText("");
        ui->conceptLineEdit->setText("");
        ui->costLineEdit->setText("");
        ui->dateEdit->setDate(QDate::currentDate());

        for (int i=0; i<usersNumber; i++) {
            usersCheckBoxes[i]->setChecked(false);
            userTicketData[i]->setText(trUtf8("automático"));
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
    if (!isnumber) return trUtf8("Error: El coste no es un numero válido.");

    bool anyChecked = false;
    for (int i=0; i<usersNumber; i++)
    {
        if (usersCheckBoxes[i]->isChecked()) { anyChecked = true; break; }
    }
    if (!anyChecked) { return trUtf8("Debe involucrar a algún usuario."); }

    float percent;
    int total_percentage = 0;
    bool automatic = false;
    for (int i=0; i<usersNumber; i++) {
        if (usersCheckBoxes[i]->isChecked())
        {
            if (userTicketData[i]->text().compare(trUtf8("automático"))!=0)
            {
                percent = userTicketData[i]->text().toFloat(&isnumber);
                isnumber = isnumber && percent > 0 && percent < 100;
                if (!isnumber) return "El porcentaje " + QString::number(i+1) + trUtf8(" no es válido.");
                else total_percentage += percent;
            }
            else automatic = true;
        }
    }
    if (automatic)
    {
        if (total_percentage >= 100) return "La suma de porcentages supera el 100%.";
    } else if (total_percentage != 100) return "La suma de porcentages debe ser 100.";

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

    if (!validQuery) return trUtf8("Datos de ticket no válidos.")+query.lastError().text();
    int ticketId = query.lastInsertId().toInt();


    //write the contributors of the ticket
    QString percentage;
    bool equalContribution = ui->contributionCheckBox->isChecked();
    for (int i=0; i<usersNumber && validQuery; i++)
    {
        if (usersCheckBoxes[i]->isChecked())
        {
            if (equalContribution || userTicketData[i]->text().compare(trUtf8("automático"))==0) percentage = "0";
            else percentage = userTicketData[i]->text();

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

QString TicketDialog::updateTicket()
{
    bool isnumber, validQuery;
    QString cost = ui->costLineEdit->text();

    //check that all number fields have numbers (cost and percentages)
    cost.toFloat(&isnumber);
    if (!isnumber) { return trUtf8("Error: El coste no es un numero valido."); }

    bool anyChecked = false;
    for (int i=0; i<usersNumber; i++)
    {
        if (usersCheckBoxes[i]->isChecked()) { anyChecked = true; break; }
    }
    if (!anyChecked) { trUtf8("Debe involucrar a algún usuario."); }

    float percent;
    int total_percentage = 0;
    bool automatic = false;
    for (int i=0; i<usersNumber; i++) {
        if (usersCheckBoxes[i]->isChecked())
        {
            if (userTicketData[i]->text().compare(trUtf8("automático"))!=0)
            {
                percent = userTicketData[i]->text().toFloat(&isnumber);
                isnumber = isnumber && percent > 0 && percent < 100;
                if (!isnumber) return "El porcentaje " + QString::number(i+1) + trUtf8(" no es válido.");
                else total_percentage += percent;
            }
            else automatic = true;
        }
    }
    if (automatic)
    {
        if (total_percentage >= 100) return "La suma de porcentages supera el 100%.";
    } else if (total_percentage != 100) return "La suma de porcentages debe ser 100.";

    //update ticket with the information
    QString user = ui->comboBox->currentText();
    QString concept = ui->conceptLineEdit->text();
    QDate emission_date = ui->dateEdit->date();
    QSqlQuery query;
    validQuery = query.exec ("update ticket set concept='" + concept +
                             "', created='" + QDate::currentDate().toString("yyyy-MM-dd") +
                             "', payed='" + emission_date.toString("yyyy-MM-dd") +
                             "', amount=" + cost +
                             ", user='" + user +
                             "' where id=" + QString::number(id) +";");

    if (!validQuery) return trUtf8("Datos nuevos no válidos.");

    //update the contributors of the ticket (remove olds and add the new ones)
    query.exec("delete from ticket_user where idTicket="+QString::number(id)+";");
    bool equalContribution = ui->contributionCheckBox->isChecked();
    QString percentage;
    for (int i=0; i<usersNumber && validQuery; i++) {
        if (usersCheckBoxes[i]->isChecked()) {
            if (equalContribution || userTicketData[i]->text().compare(trUtf8("automático"))==0) percentage = "0";
            else percentage = userTicketData[i]->text();

            validQuery = validQuery && query.exec ("insert into ticket_user (idTicket, user, percent) VALUES ("+
                                                   QString::number(id) + ", '"+
                                                   usersCheckBoxes[i]->text() + "', " +
                                                   percentage +");");
        }
    }

    if (!validQuery) return trUtf8("Datos de compañer@s no actualizados correctamente.");

    return "";
}
