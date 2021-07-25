#include "databasewidget.h"
#include "ui_databasewidget.h"
#include <QPainter>
#include <QVariant>

DatabaseWidget::DatabaseWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatabaseWidget)
{
    ui->setupUi(this);

    //1初始化一个对象。动态分配空间，注意不能指定父对象，因为后面要将该自定义的线程移到子线程中去
    myDatabaseThread = new DatabaseThread;
    //创建子进程
    databaseThread = new QThread(this);
    //把自定义的线程加入到子线程中
    myDatabaseThread->moveToThread(databaseThread);   //2.将对象添加到子线程中


    connect(myDatabaseThread, &DatabaseThread::RFIDSignal, this, &DatabaseWidget::dealID);
    connect(this, &DatabaseWidget::startRFIDThread, myDatabaseThread, &DatabaseThread::RFIDRead);
    connect(this, &DatabaseWidget::destroyed, this, &DatabaseWidget::dealClose);//窗口关闭时候把子进程也关了
    nowID = "00000000";
    ui->middleLabel->setVisible(false);

}

DatabaseWidget::~DatabaseWidget()
{
    delete ui;
}

void DatabaseWidget::dealID(QString val)
{
    nowID = val;
    ui->idLabel->setText("当前用户ID："+nowID);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mygames1.db");
    if (!db.open())
    {
        qDebug() << "Error: Failed to connect database." << db.lastError();
    }
    else
    {
        model1 = new MySqlQuery(this);
        model1->setQuery("select * from games where id='"+nowID+"' order by point desc;");
        ui->nowTableView1->setModel(model1);
        ui->nowTableView1->setColumnWidth(0, 130);
        ui->nowTableView1->setColumnWidth(1, 138);
        ui->nowTableView1->setColumnWidth(2, 80);
        model1->setHeaderData(0, Qt::Horizontal, "卡号");
        model1->setHeaderData(1, Qt::Horizontal, "游戏名");
        model1->setHeaderData(2, Qt::Horizontal, "分数");
    }
    db.close();
    ui->middleLabel->setVisible(false);
    dealClose();
}

void DatabaseWidget::dealClose()
{
    if(databaseThread->isRunning() == true)
    {
        //4.像之前那样停止，多了个标志位，使得进程里面的循环可以结束
        myDatabaseThread->setFlag(true);
        databaseThread->quit();
        databaseThread->wait();  //等待线程中的内容处理完再结束
    }
}

void DatabaseWidget::restart()
{
    update();
    updateDatabase();
}

void DatabaseWidget::updateDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mygames1.db");
    if (!db.open())
    {
        qDebug() << "Error: Failed to connect database." << db.lastError();
    }
    else
    {
        model1 = new MySqlQuery(this);
        model1->setQuery("select * from games where id='"+nowID+"' order by point desc;");
        ui->nowTableView1->setModel(model1);
        ui->nowTableView1->setColumnWidth(0, 130);
        ui->nowTableView1->setColumnWidth(1, 138);
        ui->nowTableView1->setColumnWidth(2, 80);
        model1->setHeaderData(0, Qt::Horizontal, "用户ID");
        model1->setHeaderData(1, Qt::Horizontal, "游戏名");
        model1->setHeaderData(2, Qt::Horizontal, "分数");

        model2 = new MySqlQuery(this);
        model2->setQuery("select id,point from games where name='snake' order by point desc limit 3;");
        ui->nowTableView2->setModel(model2);
        ui->nowTableView2->setColumnWidth(0, 177);
        ui->nowTableView2->setColumnWidth(1, 140);
        model2->setHeaderData(0, Qt::Horizontal, "用户ID");
        model2->setHeaderData(1, Qt::Horizontal, "分数");

        model3 = new MySqlQuery(this);
        model3->setQuery("select id,point from games where name='gesture' order by point desc limit 3;");
        ui->nowTableView3->setModel(model3);
        ui->nowTableView3->setColumnWidth(0, 177);
        ui->nowTableView3->setColumnWidth(1, 140);
        model3->setHeaderData(0, Qt::Horizontal, "用户ID");
        model3->setHeaderData(1, Qt::Horizontal, "分数");
    }
    db.close();
}



void DatabaseWidget::beginThread()
{
    if(databaseThread->isRunning() == false)
    {
        //3.启动线程，但是没有启动线程函数run
        databaseThread->start();
        myDatabaseThread->setFlag(false);
        //3.没有执行run，所以通过发送信号来使得，线程中的对象的函数得以执行，只能通过信号和槽方式调用
        emit startRFIDThread();
    }
}

void DatabaseWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //背景图片
    QPixmap pix;
    pix.load(":/images/images/data_background.jpg");
    painter.drawPixmap(0, 0, 800, 480, pix);
}

void DatabaseWidget::on_loginButton_clicked()
{
    ui->middleLabel->setVisible(true);
    beginThread();
}


void DatabaseWidget::on_returnButton_clicked()
{
    dealClose();
    emit closeDatabase();
}

void DatabaseWidget::insertFromGes(int pp)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mygames1.db");
    if (!db.open())
    {
        qDebug() << "Error: Failed to connect database." << db.lastError();
    }
    else
    {
        QVariant theID = nowID, thePoint = pp;
        QSqlQuery query(db);
        query.prepare("insert into games(id,name,point) values(:a,'gesture',:b);");
        query.bindValue(":a", theID);
        query.bindValue(":b", thePoint);
        if(query.exec()){
            qDebug() << "insertFromGes okok";
        }else{
            qDebug() << "insertFromGes wrong";
        }
    }
    db.close();

}

void DatabaseWidget::insertFromSnake(int pp)
{

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mygames1.db");
    if (!db.open())
    {
        qDebug() << "Error: Failed to connect database." << db.lastError();
    }
    else
    {
        QVariant theID = nowID, thePoint = pp;
        QSqlQuery query(db);
        query.prepare("insert into games(id,name,point) values(:a,'snake',:b);");
        query.bindValue(":a", theID);
        query.bindValue(":b", thePoint);
        if(query.exec()){
            qDebug() << "insertFromSnake okok";
        }else{
            qDebug() << "insertFromSnake wrong";
        }
    }
    db.close();
}
