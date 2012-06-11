#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include "configuration.h"
#include <qdialog.h>
#include <QtSql/qsqlquerymodel.h>
#include <qitemselectionmodel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>

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
    bool changeContribution();

private:
    vector<QCheckBox *> usersCheckBoxes;
    vector<QLineEdit *> userTicketData;
    vector<QLabel *> userTicketLabel;
    int usersNumber;
    bool editDialog; //to know if we are adding a new ticket, or viewing/editing one
    bool isEditing; //if we are in edit mode
    int id; //id of the ticket being edited

    Ui::TicketDialog *ui;
    bool organizeDialog(QSqlQueryModel *userModel);
    QString writeTicket();
    QString updateTicket();

};

#endif // TICKETDIALOG_H
