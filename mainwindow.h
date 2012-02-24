#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <configuration.h>
#include <QMainWindow>
#include <QtSql/QSqlDatabase>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Configuration *config;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    bool addTicket();
    bool editTicket();
    bool deleteTicket();
    bool enableButtons();
    bool calculeBill();
    void on_actionAddBuddy_triggered();

    void on_actionConfigurar_BBDD_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase *db;
    bool isDatabaseInitialized;

    bool connectDB();
    bool getTickets();
    bool closeDB();
};

#endif // MAINWINDOW_H
