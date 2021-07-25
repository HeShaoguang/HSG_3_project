#include "gesturethread.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

GestureThread::GestureThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}


int GestureThread::PAJ7620_Open1()
{
    int fd_paj7620_1 = open("/dev/paj7620_1", O_RDWR);
    if(fd_paj7620_1 < 0)
    {
        printf("open file : /dev/paj7620_1 failed !\n");
        return - 1;
    }

     printf("open /dev/paj7620_1 success !\n");

     return fd_paj7620_1;
}

int GestureThread::PAJ7620_Open2()
{
    int fd_paj7620_2 = open("/dev/paj7620_2", O_RDWR);
    if(fd_paj7620_2 < 0)
    {
        printf("open file : /dev/paj7620_2 failed !\n");
        return - 1;
    }

     printf("open /dev/paj7620_2 success !\n");

     return fd_paj7620_2;
}

void GestureThread::PAJ1Read()
{
    int error = 0;
    unsigned char data1 = 0x01, data2 = 0x01;

    my_fd1 = PAJ7620_Open1();
    my_fd2 = PAJ7620_Open2();

    while(isStop == false)
    {
        if(-1 == my_fd1)
        {
            data1 = data1<<1;
            if(!(data1|0x00)) data1 = 0x01;
        }else
        {
            error = read(my_fd1,&data1,sizeof(data1));
            if(error < 0)
            {
                printf("read file1 error! \n");
                close(my_fd1);
            }
        }
       // usleep(1000 * 500);

        if(-1 == my_fd2)
        {
            data2 = data2<<1;
            if(!(data2 | 0x00)) data2 = 0x01;
        }else
        {
            error = read(my_fd2,&data2,sizeof(data2));
            if(error < 0)
            {
                printf("read file2 error! \n");
                close(my_fd2);
            }
        }

        emit PAJ1Signal(data1, data2);
        usleep(1000 * 500);
    }

    close(my_fd1);
    if(error < 0)
    {
        printf("close file error! \n");
    }
}

void GestureThread::setFlag(bool flag)
{
    isStop = flag;
}



