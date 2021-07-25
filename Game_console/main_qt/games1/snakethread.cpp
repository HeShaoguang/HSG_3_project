#include "snakethread.h"
#include <QThread>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <config.h>
#include <signal.h>
#include <fcntl.h>

//SnakeThread *SnakeThread::mySnakeThread = nullptr;

SnakeThread::SnakeThread(QObject *parent) : QObject(parent)
{
    isStop = false;
    //mySnakeThread = this;
}

//static int fd;

//static void sigio_signal_func(int num)
//{
//    float xDirection = 0, yDirection = 0;
//     //printf("sigio_signal_func! \n");
//    short resive_data[6];  //保存收到的 mpu6050转换结果数据，依次为 AX(x轴角度), AY, AZ 。GX(x轴加速度), GY ,GZ
//    int error = read(fd ,resive_data,sizeof(resive_data));
//        if(error < 0)
//        {
//            printf("write file error! \n");
//            close(fd);
//            /*判断是否关闭成功*/
//        }
//        xDirection = SnakeThread::Kalman_Filter(resive_data[0], resive_data[4]);
//        yDirection = SnakeThread::Kalman_Filter(resive_data[1], resive_data[3]);
//        SnakeThread::emitSignal(xDirection, yDirection);
//}

//void SnakeThread::emitSignal(float x, float y)
//{
//    emit mySnakeThread->SnakeSignal(x, y);
//}

int SnakeThread::MPU6050_Open()
{
    //int flags;
    int fd_mpu6050 = open("/dev/I2C1_mpu6050", O_RDWR);

    if(fd_mpu6050 < 0)
    {
        printf("open file : /dev/I2C1_mpu6050 failed !\n");
        return - 1;
    }

     printf("open /dev/I2C1_mpu6050 success !\n");

     //fd = fd_mpu6050;
//     signal(SIGIO, sigio_signal_func);

//     fcntl(fd_mpu6050, F_SETOWN, getpid());   //设置当前进程接受SIGIO信号，getpid是获取当前进程号
//     flags = fcntl(fd_mpu6050, F_GETFL);
//     fcntl(fd_mpu6050, F_SETFL, flags | FASYNC);  //开启异步通知
     return fd_mpu6050;
}


void SnakeThread::SnakeTimeout()
{
    float xDirection = 0, yDirection = 0;
    int error;
    short resive_data[6];

    my_fd = MPU6050_Open();

    while(isStop == false)
    {
        if(-1 == my_fd)
        {
             emit SnakeSignal((qrand() % 5), (qrand() % 5));
        }else
        {
            error = read(my_fd,resive_data,sizeof (resive_data));
            if(error < 0)
            {
                printf("read file error! \n");
                close(my_fd);
            }
            xDirection = Kalman_Filter(resive_data[0], resive_data[4]);
            yDirection = Kalman_Filter(resive_data[1], resive_data[3]);
            emit SnakeSignal(xDirection, yDirection);
        }
        usleep(1000 * SNAKE_READ_TIME);
        //线程处理函数内部不允许操作图像界面，应该是纯数据处理，否则会报错
        //QMessageBox::aboutQt(NULL);
    }

    close(my_fd);
    if(error < 0)
    {
        printf("close file error! \n");
    }
}


void SnakeThread::setFlag(bool flag)
{
    isStop = flag;
}



//卡尔曼滤波
float SnakeThread::Kalman_Filter(float angle_m, float gyro_m)//angleAx 和 gyroGy
{
    float dt=0.001;//注意：dt的取值为kalman滤波器采样时间
    float angle = 0, angle_dot;//角度和角速度
    float P[2][2] = {{ 1, 0 },
                    { 0, 1 }};
    float Pdot[4] ={ 0,0,0,0};
    float Q_angle=0.001, Q_gyro=0.005; //角度数据置信度,角速度数据置信度
    float R_angle=0.5 ,C_0 = 1;
    float q_bias = 0, angle_err, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;

        angle+=(gyro_m-q_bias) * dt;
        angle_err = angle_m - angle;
        Pdot[0]=Q_angle - P[0][1] - P[1][0];
        Pdot[1]= -P[1][1];
        Pdot[2]= -P[1][1];
        Pdot[3]=Q_gyro;
        P[0][0] += Pdot[0] * dt;
        P[0][1] += Pdot[1] * dt;
        P[1][0] += Pdot[2] * dt;
        P[1][1] += Pdot[3] * dt;
        PCt_0 = C_0 * P[0][0];
        PCt_1 = C_0 * P[1][0];
        E = R_angle + C_0 * PCt_0;
        K_0 = PCt_0 / E;
        K_1 = PCt_1 / E;
        t_0 = PCt_0;
        t_1 = C_0 * P[0][1];
        P[0][0] -= K_0 * t_0;
        P[0][1] -= K_0 * t_1;
        P[1][0] -= K_1 * t_0;
        P[1][1] -= K_1 * t_1;
        angle += K_0 * angle_err; //最优角度
        q_bias += K_1 * angle_err;
        angle_dot = gyro_m-q_bias;//最优角速度

        return angle;
}

