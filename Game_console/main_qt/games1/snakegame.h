#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <QWidget>
#include "snakethread.h"
#include <QThread>
#include <QTimer>
#include <QList>
#include <QRect>
#include <QPainter>
//#include <QKeyEvent>

namespace Ui {
class SnakeGame;
}

enum Direct
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

class SnakeGame : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeGame(QWidget *parent = nullptr);
    ~SnakeGame();

    void dealDirection(float xdire, float ydire);
   void dealClose();
   void paintEvent(QPaintEvent *event);
   //void keyPressEvent(QKeyEvent *event);

   void timeout();
   void addTop();
   void addDown();
   void addRight();
   void addLeft();
   void deleteLast();
   void addNewReword();
   bool checkContact();
   void restart();
   void beginThread();

signals:
    void startSnakeThread();   //启动子线程信号
    void snakeInsDatebase(int p);
private slots:

    void on_buttonStart_clicked();

    void on_returnButton_clicked();

signals:
    void closeSnake();

private:
    Ui::SnakeGame *ui;


    SnakeThread *mySnakeThread;
    QThread *snakeThread;

    int moveFlag; //移动方向
    //bool gameStart;  //游戏开启
    QTimer *timer;  //定时器
    QList<QRectF> snake; //蛇
    int nodeWidth = 32; //小方块宽度
    int nodeHeight = 32; //小方块高度
    QRectF rewardNode;
    int point;
    int beginlen;
    //int reflashtime;
    bool isover;
    //bool ismove;

    int up_down, left_right;
};

#endif // SNAKEGAME_H
