#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->titleVersion->setText(trUtf8("Seis de Picas - Versión 0.3.28"));
}

About::~About()
{
    delete ui;
}
