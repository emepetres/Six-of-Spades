#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "configuration.h"
#include <qmainwindow.h>
#include <QtSql/qsqldatabase.h>
#include <QDate>

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

    void on_actionConfigurar_BBDD_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase *db;
    bool isDatabaseInitialized;
    QDate billStartDate;
    QDate billEndDate;

    bool connectDB();
    bool closeDB();
};

#endif // MAINWINDOW_H
