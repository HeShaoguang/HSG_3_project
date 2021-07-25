#include "databasethread.h"
#include <QThread>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

DatabaseThread::DatabaseThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}

int DatabaseThread::RFID_Open()
{
    int fd_rfid = open("/dev/rfid", O_RDWR);
    if(fd_rfid < 0)
    {
        printf("open file : /dev/rfid failed !\n");
        return - 1;
    }

     //printf("open /dev/rfid success !\n");

     return fd_rfid;
}
void DatabaseThread::RFIDRead()
{
    int error;
    unsigned char resive_data[4];
    char str[4] = {0};
    my_fd = RFID_Open();

    while(isStop == false)
    {
        if(-1 == my_fd)
        {
             emit RFIDSignal("00000000");
        }else
        {
            error = read(my_fd,resive_data,sizeof (resive_data));
            if(error < 0)
            {
                printf("read file error! \n");
                close(my_fd);
            }
            if(resive_data[0] != 0 || resive_data[1] != 0 || resive_data[2] != 0 ||resive_data[3] != 0 )
            {
                sprintf(str, "%X%X%X%X", resive_data[0], resive_data[1], resive_data[2], resive_data[3]);
                emit RFIDSignal(QString(str));
            }
        }
        usleep(1000 * 250);
    }

    close(my_fd);
    if(error < 0)
    {
        printf("close file error! \n");
    }
}

void DatabaseThread::setFlag(bool flag)
{
    isStop = flag;
}

