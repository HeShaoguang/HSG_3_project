#ifndef GESTUREGAME_H
#define GESTUREGAME_H

#include "bomb.h"
#include "mygesture.h"
#include "itsgesture.h"
#include "config.h"
#include "map.h"
#include "gesturethread.h"
#include <QThread>
#include <QWidget>
#include <QPaintEvent>
#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QQueue>
#include <QMessageBox>

namespace Ui {
class GestureGame;
}

class GestureGame : public QWidget
{
    Q_OBJECT

public:
    explicit GestureGame(QWidget *parent = nullptr);
    ~GestureGame();

    void init();
    /* 更新游戏中元素坐标 */
    void updatePosition();

    /* 绘制 */
    void paintEvent(QPaintEvent *);

    /* 生成手势函数 */
    void itsGuetureCreate();

    /* 输入手势函数 */
    void myGuetureCreate();
    void myGuetureCreate2();


    void keyPressEvent(QKeyEvent *event);
    //碰撞
    void collisionDetection1();

    //加分
    void pointAdd();
    void lifeReduce();


    void dealValue(unsigned char value1, unsigned char value2);
    void dealClose();
    void beginThread();


private slots:
    void on_pushButton_2_clicked();
    void restart();

    void on_returnButton_clicked();

signals:
    void closeGesture();
    void gesInsDatebase(int p);
    void startPAJ1Thread();   //启动子线程信号

private:
    Ui::GestureGame *ui;
    //地图对象
    Map m_map;
    //定时器
    QTimer m_Timer;
    //手势数组
    ItsGesture itsGes[ITS_SIZE];
    ItsGesture itsGes2[ITS_SIZE];
    MyGesTure myGes[My_SIZE];
    MyGesTure myGes2[My_SIZE];
    //爆炸数组
    Bomb m_bombs[BOMB_NUM];

    //生成的手势出场间隔记录
    int itsGes_recorder;
    //输入的手势出场间隔记录
    int myGes_recorder;
    int myGes_recorder2;
    //保存输入
    QQueue<int> myInputGes;
    QQueue<int> myInputGes2;

    //分数
    int point;
    int life;
    bool isover;

    GestureThread *myGestureThread;
    QThread *snakeThread;
};

#endif // GESTUREGAME_H
