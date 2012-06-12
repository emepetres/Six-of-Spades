#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "configuration.h"
#include <qmainwindow.h>
#include <QtSql/qsqldatabase.h>
#include <qdatetime.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Configuration *config;
    bool getTickets();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    bool addTicket();
    bool editTicket();
    bool deleteTicket();
    bool enableButtons();
    bool calculeBill();
    void on_actionAddBuddy_triggered();
    bool saveBill();
	bool saveAndRemoveBill();

    void on_actionSobre_Seis_De_Picas_triggered();

    void on_actionAbrir_triggered();

    void on_actionAbrir_remoto_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase *db;
    bool isDatabaseInitialized;
	QDate startBillDate;
	QDate endBillDate;

    bool connectDB();
    bool closeDB();
    void changeDataBase(bool local);
    void setHelp();
};

#endif // MAINWINDOW_H
