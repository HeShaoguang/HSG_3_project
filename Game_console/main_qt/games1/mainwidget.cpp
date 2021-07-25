#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDebug>
#include <QMessageBox>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    snakegame = new SnakeGame;

    gesturegame = new GestureGame;
    databasewidget = new DatabaseWidget;

    scene = new QGraphicsScene;
    w = scene->addWidget(gesturegame);
    w->setRotation(90);
    view = new QGraphicsView(scene);
    view->resize(802, 482);

    connect(snakegame, &SnakeGame::closeSnake, this, &MainWidget::dealCloseSnake);
    connect(gesturegame, &GestureGame::closeGesture, this, &MainWidget::dealClosGesture);
    connect(databasewidget, &DatabaseWidget::closeDatabase, this, &MainWidget::dealCloseDatabse);
    connect(snakegame, &SnakeGame::snakeInsDatebase, databasewidget, &DatabaseWidget::insertFromSnake);
    connect(gesturegame, &GestureGame::gesInsDatebase, databasewidget, &DatabaseWidget::insertFromGes);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::dealCloseSnake()
{
    this->show();
    snakegame->close();
    //view->close();
}

void MainWidget::dealClosGesture()
{
    this->show();
    view->close();
}

void MainWidget::dealCloseDatabse()
{
    this->show();
    databasewidget->close();
}

void MainWidget::on_snakeButton_clicked()
{
    if(databasewidget->nowID == "00000000")
    {
        QMessageBox::information(this,"提醒","请先点击个人中心登陆","确认");
        return;
    }
    this->close();
    snakegame->restart();
    snakegame->beginThread();
    snakegame->show();
}

void MainWidget::on_gestureButton_clicked()
{
    if(databasewidget->nowID == "00000000")
    {
        QMessageBox::information(this,"提醒","请先点击个人中心登陆","确认");
        return;
    }
    this->close();
    gesturegame->init(); //初始化
    //m_Timer.start();
    gesturegame->beginThread();
    view->show();
}

void MainWidget::on_dataButton_clicked()
{
    this->close();
    databasewidget->restart();
   // databasewidget->beginThread();
    databasewidget->show();
}
