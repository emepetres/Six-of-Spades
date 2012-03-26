#include "databasedialog.h"
#include "ui_databasedialog.h"
#include <qsqlquery.h>

DatabaseDialog::DatabaseDialog(Configuration *config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseDialog)
{
    this->config = config;
    ui->setupUi(this);

    ui->localCheckBox->setChecked(config->isLocal());
    ui->hostEdit->setText(config->hostName());
    ui->databaseEdit->setText(config->databaseName());
    ui->userEdit->setText(config->userName());
    ui->passwordEdit->setText(config->password());
    setFields();

    this->connect(ui->saveButton, SIGNAL(clicked()), SLOT(saveProperties()));
    this->connect(ui->cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

DatabaseDialog::~DatabaseDialog()
{
    delete ui;
}

bool DatabaseDialog::saveProperties()
{
    config->OpenConfigDatabase();

    QSqlQuery query;
    query.exec(QString("update 'properties' set value='") +
               ui->hostEdit->text() + QString("' where property='hostName';"));
    query.exec(QString("update 'properties' set value='") +
               ui->databaseEdit->text() + QString("' where property='databaseName';"));
    query.exec(QString("update 'properties' set value='") +
               ui->userEdit->text() + QString("' where property='userName';"));
    query.exec(QString("update 'properties' set value='") +
               ui->passwordEdit->text() + QString("' where property='password';"));
    query.exec(QString("update 'properties' set value='") +
               (ui->localCheckBox->isChecked()? QString("1"):QString("0")) + QString("' where property='local';"));

    QString error = query.lastError().text();

    config->CloseDatabase();

    accept();
    return true;
}

bool DatabaseDialog::setFields()
{
    if (ui->localCheckBox->isChecked())
    {
        ui->hostEdit->setDisabled(true);
        ui->databaseEdit->setDisabled(true);
        ui->userEdit->setDisabled(true);
        ui->passwordEdit->setDisabled(true);
    }
    else
    {
        ui->hostEdit->setEnabled(true);
        ui->databaseEdit->setEnabled(true);
        ui->userEdit->setEnabled(true);
        ui->passwordEdit->setEnabled(true);
    }
    return true;
}
