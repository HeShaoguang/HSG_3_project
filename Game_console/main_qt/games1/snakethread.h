#ifndef SNAKETHREAD_H
#define SNAKETHREAD_H

#include <QObject>

class SnakeThread : public QObject
{
    Q_OBJECT
public:
    explicit SnakeThread(QObject *parent = nullptr);


    void SnakeTimeout();
    void setFlag(bool flag = true);
    int MPU6050_Open();
    static float Kalman_Filter(float angle_m, float gyro_m);
    //void sigio_signal_func(int num);
    //static void emitSignal(float x, float y);

signals:
    void SnakeSignal(float x, float y);
public slots:

private:
    //static SnakeThread *mySnakeThread;
    bool isStop;
    int my_fd;
};

#endif // SNAKETHREAD_H
