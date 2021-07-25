#include "snakegame.h"
#include "ui_snakegame.h"
#include <QDebug>
#include <config.h>

SnakeGame::SnakeGame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SnakeGame)
{
    ui->setupUi(this);

    //1初始化一个对象。动态分配空间，注意不能指定父对象，因为后面要将该自定义的线程移到子线程中去
    mySnakeThread = new SnakeThread;
    //创建子进程
    snakeThread = new QThread(this);
    //把自定义的线程加入到子线程中
    mySnakeThread->moveToThread(snakeThread);   //2.将对象添加到子线程中


    connect(mySnakeThread, &SnakeThread::SnakeSignal, this, &SnakeGame::dealDirection);
    connect(this, &SnakeGame::startSnakeThread, mySnakeThread, &SnakeThread::SnakeTimeout);
    connect(this, &SnakeGame::destroyed, this, &SnakeGame::dealClose);//窗口关闭时候把子进程也关了

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &SnakeGame::timeout);
//    restart();
//    beginThread();
}

SnakeGame::~SnakeGame()
{
    delete ui;
    delete mySnakeThread;
}


void SnakeGame::dealClose()
{
    if(snakeThread->isRunning() == true)
    {
        //4.像之前那样停止，多了个标志位，使得进程里面的循环可以结束
        mySnakeThread->setFlag(true);
        snakeThread->quit();
        snakeThread->wait();  //等待线程中的内容处理完再结束
    }
}

void SnakeGame::dealDirection(float xdire, float ydire)
{
    QString xstr,ystr;

    if(xdire > 3000){
        xstr = "右";
        left_right = 2;
    }else if(xdire < -3000){
        xstr = "左";
        left_right = 1;
    }else{
        xstr = " ";
        left_right = 0;
    }

    if(ydire > 3000){
        ystr = "上";
        up_down = 1;
    }else if(ydire < -3000){
        ystr = "下";
        up_down = 2;
    }else{
        ystr = " ";
        up_down = 0;
    }

    ui->xlabel->setText(QString("方向:%1 %2").arg(xstr).arg(ystr));
}

void SnakeGame::restart()
{
    moveFlag = DIR_RIGHT;
    //gameStart = false;
    isover = false;
    //ismove = false;
    resize(800,480);

    //初始化蛇
    snake.clear();
    QRectF rect(0,160,nodeWidth,nodeHeight);
    snake.append(rect);
    point = 0;
    beginlen = 9;
    //reflashtime = 325;
    for(int i = 1; i<=beginlen ;i++)
    {
        addRight();
    }
    addNewReword();
}

void SnakeGame::beginThread()
{
    if(snakeThread->isRunning() == false)
    {
        //3.启动线程，但是没有启动线程函数run
        snakeThread->start();
        mySnakeThread->setFlag(false);
        //3.没有执行run，所以通过发送信号来使得，线程中的对象的函数得以执行，只能通过信号和槽方式调用
        emit startSnakeThread();
    }
}

void SnakeGame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    QBrush brush;

    //背景图片
    QPixmap pix;
    pix.load(":/images/images/sky.jpg");
    //pix.load("/images/snake/sky.jpg");
    painter.drawPixmap(0, 0, 800, 480, pix);

    //画蛇
    pen.setColor(Qt::black);
    brush.setColor(QColor(12,60,1));
    brush.setStyle(Qt::SolidPattern);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawRect(snake[0]);


    brush.setColor(QColor(0,173,25));
    painter.setBrush(brush);
    for(int i = 1; i<snake.length(); i++)
    {
        painter.drawRect(snake[i]);
    }

    //画奖品
    pen.setColor(Qt::red);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawEllipse(rewardNode);

    QFont font("方正舒体", 20, QFont::ExtraLight, false);
    painter.setFont(font);
    painter.drawText(660,470,QString("分数：%1").arg(point));

    //判断是否撞自己
    if(isover)
    {
        emit snakeInsDatebase(point);
        ui->buttonStart->setText("重新开始");
        QFont font("方正舒体", 30, QFont::ExtraLight, false);
        painter.setFont(font);
        painter.drawText((this->width()-300)/2,(this->height()-50)/2,QString("GAME OVER"));
        restart();
        timer->stop();
    }

}

void SnakeGame::timeout()
{
    if(moveFlag == DIR_DOWN ||  moveFlag == DIR_UP)
    {
        if(left_right == 1)
        {
            moveFlag = DIR_LEFT;
            //addLeft();
            left_right = 0;
        }else if(left_right == 2){
            moveFlag = DIR_RIGHT;
            //addRight();
            left_right = 0;
        }
    }else if(moveFlag == DIR_LEFT ||  moveFlag == DIR_RIGHT){
        if(up_down == 1)
        {
            moveFlag = DIR_UP;
            //addTop();
            up_down = 0;
        }else if(up_down == 2){
            moveFlag = DIR_DOWN;
            //addDown();
            up_down = 0;
        }
    }

    switch (moveFlag)
    {
        case DIR_UP:
            addTop();
            break;
        case DIR_DOWN:
            addDown();
            break;
        case DIR_RIGHT:
            addRight();
            break;
        case DIR_LEFT:
            addLeft();
            break;

        default:break;
    }

    //判断是否吃到
    if(snake[0].intersects(rewardNode))
    {
        point++;
        addNewReword();
    }else{
        deleteLast();
    }

    if(checkContact()) isover = true;
    update();//刷新界面

}

void SnakeGame::deleteLast()
{
    snake.removeLast();
}

void SnakeGame::addNewReword()
{
    int i=0;
    //防止出现在蛇身
    do{
       i=0;
       rewardNode = QRectF(qrand()%(this->width()/nodeWidth)*nodeWidth,
                           qrand()%(this->height()/nodeHeight)*nodeHeight,
                           nodeWidth,
                           nodeHeight);
       for(; i < snake.length(); i++){  //蛇头不碰到就好
           if(snake[i].intersects(rewardNode)){
               qDebug() << "addNewReword again";
               break;
           }
       }
    }while(i != snake.length());

}

bool SnakeGame::checkContact()
{
    for(int i=1; i < snake.length(); i++){  //蛇头不碰到就好
        if(snake[0] == snake[i]){
            return true;
        }
    }
    return false;
}

void SnakeGame::addTop()
{
    QPointF leftTop;
    QPointF rightBottom;

    if (snake[0].y() - nodeHeight < 0)
    {
        leftTop = QPoint(snake[0].x(), this->height() - nodeHeight);
        rightBottom = QPoint(snake[0].x() + nodeWidth, this->height());
    }else{
        leftTop = QPoint(snake[0].x(), snake[0].y() - nodeHeight);
        rightBottom = snake[0].topRight();
    }

    snake.insert(0, QRectF(leftTop, rightBottom));
}

void SnakeGame::addDown()
{
    QPointF leftTop;
    QPointF rightBottom;

    if(snake[0].y() + nodeHeight*2 > this->height())
    {
        leftTop = QPointF(snake[0].x(), 0);
        rightBottom = QPointF(snake[0].x()+nodeWidth, nodeHeight);
    }else
    {
        leftTop = snake[0].bottomLeft();
        rightBottom = snake[0].bottomRight() + QPointF(0, nodeHeight);
    }

    snake.insert(0, QRectF(leftTop, rightBottom));
}

void SnakeGame::addRight()
{
    QPointF leftTop;
    QPointF rightBottom;
    if(snake[0].x() + nodeWidth*2 > this->width())
    {
        leftTop = QPointF(0, snake[0].y());
    }else
    {
        leftTop = snake[0].topRight();
    }

    rightBottom = leftTop + QPointF(nodeWidth, nodeHeight);
    snake.insert(0, QRectF(leftTop, rightBottom));
}

void SnakeGame::addLeft()
{
    QPointF leftTop;
    QPointF rightBottom;
    if(snake[0].x() - nodeWidth < 0)
    {
        leftTop = QPointF(this->width()-nodeWidth, snake[0].y());
    }else
    {
        leftTop = snake[0].topLeft() - QPointF(nodeWidth, 0);
    }

    rightBottom = leftTop + QPointF(nodeWidth, nodeHeight);
    snake.insert(0, QRectF(leftTop, rightBottom));
}

void SnakeGame::on_buttonStart_clicked()
{
    if(ui->buttonStart->text() == "开始")
    {
        ui->buttonStart->setText("暂停");
        isover = false;
        timer->start(SNAKE_READ_TIME/2);
    }else{
        ui->buttonStart->setText("开始");
        timer->stop();
    }
    //ismove = false;
}

void SnakeGame::on_returnButton_clicked()
{
    ui->buttonStart->setText("开始");
    timer->stop();
    dealClose();
    emit closeSnake();
}
