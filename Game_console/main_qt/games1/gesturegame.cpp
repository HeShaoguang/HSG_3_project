#include "gesturegame.h"
#include "ui_gesturegame.h"
#include <QPainter>
#include <QDebug>
#include <ctime>
#include <QMessageBox>
#include <ctime>

GestureGame::GestureGame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GestureGame)
{
    ui->setupUi(this);

    //1初始化一个对象。动态分配空间，注意不能指定父对象，因为后面要将该自定义的线程移到子线程中去
    myGestureThread = new GestureThread;
    //创建子进程
    snakeThread = new QThread(this);
    //把自定义的线程加入到子线程中
    myGestureThread->moveToThread(snakeThread);   //2.将对象添加到子线程中

    connect(myGestureThread, &GestureThread::PAJ1Signal, this, &GestureGame::dealValue);
    connect(this, &GestureGame::startPAJ1Thread, myGestureThread, &GestureThread::PAJ1Read);
    connect(this, &GestureGame::destroyed, this, &GestureGame::dealClose);//窗口关闭时候把子进程也关了


//    init(); //初始化
//    //m_Timer.start();
//    beginThread();

    connect(&m_Timer, &QTimer::timeout, [=](){

        //生成1
        itsGuetureCreate();
        //输入函数
        myGuetureCreate();
        myGuetureCreate2();
        //更新元素
        updatePosition();
        //碰撞检测
        collisionDetection1();

        //更新绘制
        update();
    });

    //随机数种子
    srand((unsigned int)time(NULL));


}

GestureGame::~GestureGame()
{
    delete ui;
}

void GestureGame::init()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle(GAME_TITLE);
    //定时器设置
    m_Timer.setInterval(GAME_INTERVAL);
    itsGes_recorder = 0;
    myGes_recorder = 0;
    myGes_recorder2 = 0;
    isover = false;
    life = GAME_LIFE;
    point = 0;
    ui->overLabel->setVisible(false);
    //m_Timer.stop();
}

void GestureGame::updatePosition()
{
    //更新地图坐标
    m_map.mapPosition();

    //更新生成手势的坐标
    for(int i=0; i < ITS_SIZE; i++)
    {
        if(itsGes[i].m_Free == false)
        {
           if(itsGes[i].updatePosition())
           {
               lifeReduce();
           }
        }
        if(itsGes2[i].m_Free == false)
        {
            if(itsGes2[i].updatePosition())
            {
                lifeReduce();
            }
        }
    }

    //更新输入手势的坐标
    for(int i=0; i < My_SIZE; i++)
    {
        if(myGes[i].m_Free == false)
        {
           if(myGes[i].updatePosition())
           {
               lifeReduce();
           }
        }

        if(myGes2[i].m_Free == false)
        {
           if(myGes2[i].updatePosition2())
           {
               lifeReduce();
           }
        }
    }

    //更新爆炸
    for(int i=0; i < BOMB_NUM; i++)
    {
        if(m_bombs[i].m_Free == false)
        {
           m_bombs[i].updatePosition();
        }
    }
}

void GestureGame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    //绘制地图
    pen.setWidth(1);
    pen.setColor(QColor(188, 98, 3));
    pen.setStyle(Qt::SolidLine); //SolidLine直线，DashLine虚线，DotLine虚点
    painter.setPen(pen);
    painter.drawPixmap(0, m_map.m_map1_posY, m_map.m_map1);
    painter.drawPixmap(0, m_map.m_map2_posY, m_map.m_map2);
    painter.drawLine(GAME_WIDTH/2, 0, GAME_WIDTH/2, GAME_HEIGHT);

    //绘制生成手势
    for(int i=0; i < ITS_SIZE; i++)
    {
        if(itsGes[i].m_Free == false)
        {
            painter.drawPixmap(itsGes[i].m_X, itsGes[i].m_Y, itsGes[i].m_pixArr[itsGes[i].direction]);
        }
        if(itsGes2[i].m_Free == false)
        {
            painter.drawPixmap(itsGes2[i].m_X, itsGes2[i].m_Y, itsGes2[i].m_pixArr[itsGes2[i].direction]);
        }
    }

    //绘制输入手势
    for(int i=0; i < My_SIZE; i++)
    {
        if(myGes[i].m_Free == false)
        {
            painter.drawPixmap(myGes[i].m_X, myGes[i].m_Y, myGes[i].m_pixArr[myGes[i].direction]);
        }
        if(myGes2[i].m_Free == false)
        {
            painter.drawPixmap(myGes2[i].m_X, myGes2[i].m_Y, myGes2[i].m_pixArr[myGes2[i].direction]);
        }
    }

    //绘制爆炸
    for(int i=0; i < BOMB_NUM; i++)
    {
        if(m_bombs[i].m_Free == false)
        {
            painter.drawPixmap(m_bombs[i].m_X, m_bombs[i].m_Y, m_bombs[i].m_pixArr[m_bombs[i].direction]);
        }
    }
}

void GestureGame::itsGuetureCreate()
{
    itsGes_recorder++;
    if(itsGes_recorder < GESTURE_INTERVAL)
    {
        return;
    }

    itsGes_recorder = 0;

    for(int i = 0; i <ITS_SIZE; i++)
    {
        if(itsGes[i].m_Free)
        {
            itsGes[i].m_Free = false;
            itsGes[i].direction = rand()%(GESTURE_MAX);
            itsGes[i].m_X = ROW2;
            itsGes[i].m_Y = -GESTURE_SIZE;
            break;
        }
    }

    for(int i = 0; i <ITS_SIZE; i++)
    {
        if(itsGes2[i].m_Free)
        {
            itsGes2[i].m_Free = false;
            itsGes2[i].direction = rand()%(GESTURE_MAX);
            itsGes2[i].m_X = ROW3;
            itsGes2[i].m_Y = -GESTURE_SIZE;
            break;
        }
    }

}

void GestureGame::myGuetureCreate()
{
    if(myGes_recorder < GESTURE_INTERVAL2)
    {
        myGes_recorder++;
        return;
    }

    if(myInputGes.isEmpty())
    {
        return;
    }

    myGes_recorder = 0;

    for(int i = 0; i <My_SIZE; i++)
    {
        if(myGes[i].m_Free)
        {
            myGes[i].m_Free = false;
            myGes[i].direction = myInputGes.dequeue();
            myGes[i].m_X = ROW1;
            myGes[i].m_Y = GAME_HEIGHT;
            break;
        }
    }
}

void GestureGame::myGuetureCreate2()
{
    if(myGes_recorder2 < GESTURE_INTERVAL2)
    {
        myGes_recorder2++;
        return;
    }

    if(myInputGes2.isEmpty())
    {
        return;
    }

    myGes_recorder2 = 0;

    for(int i = 0; i <My_SIZE; i++)
    {
        if(myGes2[i].m_Free)
        {
            myGes2[i].m_Free = false;
            myGes2[i].direction = myInputGes2.dequeue();
            myGes2[i].m_X = ROW4;
            myGes2[i].m_Y = GAME_HEIGHT;
            break;
        }
    }
}


void GestureGame::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_W: myInputGes.enqueue(0);;break;
        case Qt::Key_S: myInputGes.enqueue(1);break;
        case Qt::Key_A: myInputGes.enqueue(2);break;
        case Qt::Key_D: myInputGes.enqueue(3);break;
        case Qt::Key_E: myInputGes.enqueue(4);break;
        case Qt::Key_Q: myInputGes.enqueue(5);break;
        case Qt::Key_I: myInputGes2.enqueue(0);;break;
        case Qt::Key_K: myInputGes2.enqueue(1);break;
        case Qt::Key_J: myInputGes2.enqueue(2);break;
        case Qt::Key_L: myInputGes2.enqueue(3);break;
        case Qt::Key_O: myInputGes2.enqueue(4);break;
        case Qt::Key_U: myInputGes2.enqueue(5);break;
        default:break;
    }
}

void GestureGame::collisionDetection1()
{
    for(int i=0; i< My_SIZE; i++)
    {
        if(myGes[i].m_Free)
        {
            continue;
        }

        for(int j = 0; j< ITS_SIZE;j++)
        {
            if(itsGes[j].m_Free)
            {
                continue;
            }

            if(myGes[i].m_Rect.intersects(itsGes[j].m_Rect) && myGes[i].direction == itsGes[j].direction)
            {
                myGes[i].m_Free = true;
                itsGes[j].m_Free = true;
                pointAdd();
                for(int k = 0; k<BOMB_NUM; k++)
                {
                    if(m_bombs[k].m_Free)
                    {
                        m_bombs[k].m_Free = false;
                        m_bombs[k].m_X = itsGes[j].m_X;
                        m_bombs[k].m_Y = itsGes[j].m_Y+15;
                        break;
                    }
                }
            }
        }
    }

    for(int i=0; i< My_SIZE; i++)
    {
        if(myGes2[i].m_Free)
        {
            continue;
        }

        for(int j = 0; j< ITS_SIZE;j++)
        {
            if(itsGes2[j].m_Free)
            {
                continue;
            }

            if(myGes2[i].m_Rect.intersects(itsGes2[j].m_Rect) && myGes2[i].direction == itsGes2[j].direction)
            {
                myGes2[i].m_Free = true;
                itsGes2[j].m_Free = true;
                pointAdd();
                for(int k = 0; k<BOMB_NUM; k++)
                {
                    if(m_bombs[k].m_Free)
                    {
                        m_bombs[k].m_Free = false;
                        m_bombs[k].m_X = itsGes2[j].m_X;
                        m_bombs[k].m_Y = itsGes2[j].m_Y+15;
                        break;
                    }
                }
            }
        }
    }
}


void GestureGame::pointAdd()
{
    point++;
    ui->pointLabel->setText(QString("分数：%1").arg(point));
    if(point < 101) m_Timer.setInterval(GAME_INTERVAL-point/10);
}

void GestureGame::lifeReduce()
{
    life--;
    ui->lifeLabel->setText(QString("生命：%1").arg(life));
    if(life == 0)
    {
        emit gesInsDatebase(point);
        restart();
    }
}

void GestureGame::restart()
{
    m_Timer.stop();
    m_Timer.setInterval(GAME_INTERVAL);

    //QMessageBox::information(this, "游戏结束", QString("游戏结束，总得分：%1").arg(point));
    ui->overLabel->setText(QString("游戏结束，总得分：%1").arg(point));
    ui->overLabel->setVisible(true);
    itsGes_recorder = 0;
    myGes_recorder = 0;
    myGes_recorder2 = 0;
    isover = true;
    life = GAME_LIFE;
    point = 0;
    ui->lifeLabel->setText(QString("生命：%1").arg(life));
    ui->pointLabel->setText(QString("分数：%1").arg(point));
    //重置生成手势
    for(int i=0; i < ITS_SIZE; i++)
    {
        itsGes[i].m_Free = true;
        itsGes2[i].m_Free = true;
    }
    //重置输入手势
    for(int i=0; i < My_SIZE; i++)
    {
        myGes[i].m_Free = true;
        myGes2[i].m_Free = true;
    }
    //重置爆炸
    for(int i=0; i < BOMB_NUM; i++)
    {
        m_bombs[i].m_Free = true;
    }

    myInputGes.clear();
    myInputGes2.clear();

}

void GestureGame::on_pushButton_2_clicked()
{
    if(m_Timer.isActive() == false)
    {
        if(isover)
        {
            ui->overLabel->setVisible(false);
        }
        m_Timer.start();
        ui->pushButton_2->setText("暂停");
    }else{
        m_Timer.stop();
        ui->pushButton_2->setText("开始");
    }

}


void GestureGame::dealValue(unsigned char value1, unsigned char value2)
{
    if(value1 && myInputGes.size() < 10)
    {
        if(value1 & 0x01) myInputGes.enqueue(0); //上
        if(value1 & 0x02) myInputGes.enqueue(1); //下
        if(value1 & 0x04) myInputGes.enqueue(2); //左
        if(value1 & 0x08) myInputGes.enqueue(3); //右
        if(value1 & 0x40) myInputGes.enqueue(4); //顺
        if(value1 & 0x80) myInputGes.enqueue(5); //逆
    }

    if(value2 && myInputGes2.size() < 10)
    {
        if(value2 & 0x01) myInputGes2.enqueue(0); //上
        if(value2 & 0x02) myInputGes2.enqueue(1); //下
        if(value2 & 0x04) myInputGes2.enqueue(2); //左
        if(value2 & 0x08) myInputGes2.enqueue(3); //右
        if(value2 & 0x40) myInputGes2.enqueue(4); //顺
        if(value2 & 0x80) myInputGes2.enqueue(5); //逆
    }
}

void GestureGame::dealClose()
{
    if(snakeThread->isRunning() == true)
    {
        //4.像之前那样停止，多了个标志位，使得进程里面的循环可以结束
        myGestureThread->setFlag(true);
        snakeThread->quit();
        snakeThread->wait();  //等待线程中的内容处理完再结束
    }
}

void GestureGame::beginThread()
{
    if(snakeThread->isRunning() == false)
    {
        //3.启动线程，但是没有启动线程函数run
        snakeThread->start();
        myGestureThread->setFlag(false);
        //3.没有执行run，所以通过发送信号来使得，线程中的对象的函数得以执行，只能通过信号和槽方式调用
        emit startPAJ1Thread();
    }
}


void GestureGame::on_returnButton_clicked()
{
    restart();
    //update();
    ui->pushButton_2->setText("开始");
    dealClose();
    emit closeGesture();
}
