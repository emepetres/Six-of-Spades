#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QCheckBox>
#include <QLineEdit>

using namespace std;

namespace Ui {
    class TicketDialog;
}

class TicketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TicketDialog(QWidget *parent = 0, QSqlQueryModel *userModel = 0);
    ~TicketDialog();

public slots:
    bool addTicket();
    bool addMoreTickets();

private:
    vector<QCheckBox *> usersCheckBoxes;
    vector<QLineEdit *> percentages;
    int usersNumber;

    Ui::TicketDialog *ui;
    QString writeTicket();

};

#endif // TICKETDIALOG_H
