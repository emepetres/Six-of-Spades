#include "localdatabasedialog.h"
#include "ui_localdatabasedialog.h"
#include <qsqlquery.h>
#include <qfiledialog.h>

localdatabasedialog::localdatabasedialog(Configuration *config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::localdatabasedialog)
{
    this->config = config;
    ui->setupUi(this);

    ui->filePathText->setText(config->localName());
}

localdatabasedialog::~localdatabasedialog()
{
    delete ui;
}

bool localdatabasedialog::searchFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Buscar archivo"),
                                                     "",
                                                    trUtf8("Six-of-Spades (*.sos)"));
    ui->filePathText->setText(fileName);
    ui->errorLabel->setText("");
    return true;
}

bool localdatabasedialog::saveProperties()
{
    //check for .sos file
    QRegExp rx("*.sos");
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (!rx.exactMatch(ui->filePathText->text()))
    {
        ui->errorLabel->setText("error: El archivo debe ser de tipo .sos");
        return false;
    }

    config->OpenConfigDatabase();

    QSqlQuery query;
    query.exec(QString("update 'properties' set value='") +
               ui->filePathText->text() + QString("' where property='local_name';"));
    query.exec(QString("update 'properties' set value='1' where property='local';"));

    config->CloseDatabase();

    accept();
    return true;
}





