#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include <qdialog.h>
#include <QtSql/qsqlquerymodel.h>
#include <qitemselectionmodel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

using namespace std;

namespace Ui {
    class TicketDialog;
}

class TicketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TicketDialog(QSqlQueryModel *userModel, QWidget *parent = 0);
    TicketDialog(QSqlQueryModel *userModel, QItemSelectionModel *ticketModel, QWidget *parent = 0 );
    ~TicketDialog();

public slots:
    bool addTicket();
    bool addMoreTickets();

private:
    vector<QCheckBox *> usersCheckBoxes;
    vector<QLineEdit *> percentages;
    int usersNumber;

    Ui::TicketDialog *ui;
    bool organizeDialog(QSqlQueryModel *userModel);
    QString writeTicket();

};

#endif // TICKETDIALOG_H
