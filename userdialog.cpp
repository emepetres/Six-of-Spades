#include "userdialog.h"
#include "ui_userdialog.h"
#include <QtSql>

UserDialog::UserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserDialog)
{
    ui->setupUi(this);
    //reposition of the widgets
    ui->gridLayout->addWidget(ui->usersLabel, 0, 0, 1, 3);
    ui->gridLayout->addWidget(ui->newUser, 1, 1, 1, 3);
    ui->gridLayout->addWidget(ui->cancelButton, 3, 1);
    ui->gridLayout->addWidget(ui->deleteButton, 3, 2);
    ui->gridLayout->addWidget(ui->addButton, 3, 3);
    ui->gridLayout->addWidget(ui->errorLabel,2,0,1,3);

    QSqlQuery query;
    query.exec ("SELECT user FROM user;");
    QString users = QString("Usuarios: ");
    while(query.next())
    {
        users = users + query.value(0).toString() + QString(", ");
    }
    ui->usersLabel->setText(users);
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
        else
        {
            ui->errorLabel->setText(trUtf8("El usuario ya existe o no es válido"));
            return false;
        }
    }
    else
    {
        ui->errorLabel->setText(trUtf8("Nombre de compañer@ vacío"));
        return false;
    }
}
