#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    unsigned char resive_data;  
    int error;
    /*打开文件*/
    int fd;
        fd = open("/dev/paj7620_2", O_RDWR);
        if(fd < 0)
        {
            printf("open file : /dev/paj7620_2 failed !\n");
            return -1;
        }

    /*读取数据*/
    while(1)
    {

        error = read(fd, &resive_data,sizeof(resive_data));
        if(error < 0)
        {
            printf("read file error! \n");
            close(fd);
            /*判断是否关闭成功*/
        }
        /*打印数据*/
        printf("data: %#X \n", resive_data);
        
        usleep(1000*1000);
    }
    
            /*关闭文件*/
        error = close(fd);
        if(error < 0)
        {
            printf("close file error! \n");
        }
    
    return 0;
}