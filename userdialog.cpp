#include "userdialog.h"
#include "ui_userdialog.h"
#include <QtSql>

UserDialog::UserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserDialog)
{
    ui->setupUi(this);
    //reposition of the widgets
    ui->gridLayout->addWidget(ui->newUser, 0, 1, 1, 2);
    ui->gridLayout->addWidget(ui->cancelButton, 2, 1);
    ui->gridLayout->addWidget(ui->addButton, 2, 2);
    ui->gridLayout->addWidget(ui->errorLabel,1,0,1,3);
}

UserDialog::~UserDialog()
{
    delete ui;
}

bool UserDialog::addUser()
{
    QSqlQuery query;
    if (ui->newUser->text().length()>0)
    {
        bool validQuery = query.exec ("insert into user (user, passwd) VALUES ('"+
                                      ui->newUser->text() +"', '-');");
        if (validQuery) { accept(); return true; }
        else return false;
    }
    else
    {
        ui->errorLabel->setText(trUtf8("Nombre de compañer@ vacío"));
        return false;
    }
}
