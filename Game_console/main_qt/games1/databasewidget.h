#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

#include <QWidget>
#include "databasethread.h"
#include <QThread>
#include <QPainter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <mysqlquery.h>

namespace Ui {
class DatabaseWidget;
}

class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseWidget(QWidget *parent = nullptr);
    ~DatabaseWidget();

    void dealID(QString val);

    void dealClose();

    void restart();
    void updateDatabase();
    void beginThread();


   void paintEvent(QPaintEvent *event);

   QString nowID;

signals:
    void startRFIDThread();   //启动子线程信号
    void closeDatabase();

public slots:
    void insertFromGes(int pp);
    void insertFromSnake(int pp);

private slots:
    void on_loginButton_clicked();
    void on_returnButton_clicked();

private:
    Ui::DatabaseWidget *ui;

    DatabaseThread *myDatabaseThread;
    QThread *databaseThread;



    QSqlDatabase db;
    MySqlQuery *model1, *model2, *model3;
};

#endif // DATABASEWIDGET_H
