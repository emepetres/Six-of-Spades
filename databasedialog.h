#ifndef DATABASEDIALOG_H
#define DATABASEDIALOG_H

#include "configuration.h"
#include <qdialog.h>

namespace Ui {
    class DatabaseDialog;
}

class DatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseDialog(Configuration *config, QWidget *parent = 0);
    ~DatabaseDialog();

public slots:
    bool saveProperties();
    bool setFields();

private:
    Ui::DatabaseDialog *ui;
    Configuration *config;
};

#endif // DATABASEDIALOG_H
