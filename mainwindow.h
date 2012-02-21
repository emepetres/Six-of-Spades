#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    bool addTicket();
    bool editTicket();
    bool deleteTicket();
    bool enableButtons();
    bool calculeBill();
    void on_actionAddBuddy_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;

    bool connectDB();
    bool getTickets();
};

#endif // MAINWINDOW_H
