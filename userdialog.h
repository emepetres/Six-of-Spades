#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>

namespace Ui {
    class UserDialog;
}

class UserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserDialog(QWidget *parent = 0);
    ~UserDialog();

private:
    Ui::UserDialog *ui;

private slots:
    bool addUser();
    bool deleteUser();
};

#endif // USERDIALOG_H
