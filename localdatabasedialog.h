#ifndef LOCALDATABASEDIALOG_H
#define LOCALDATABASEDIALOG_H

#include "configuration.h"
#include <qdialog.h>

namespace Ui {
    class localdatabasedialog;
}

class localdatabasedialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit localdatabasedialog(Configuration *config, QWidget *parent = 0);
    ~localdatabasedialog();

public slots:
    bool saveProperties();
    bool searchFile();
    
private:
    Ui::localdatabasedialog *ui;
    Configuration *config;
};

#endif // LOCALDATABASEDIALOG_H
