#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int error;
    unsigned char resive_data[4];  

    /*打开文件*/
    int fd = open("/dev/rfid", O_RDWR);
    if(fd < 0)
    {
		printf("open file : %s failed !\n", argv[0]);
		return -1;
	}

    // while(1)
    // {
        /*读取数据*/
        error = read(fd,resive_data,4);
        if(error < 0)
        {
            printf("write file error! \n");
            close(fd);
            /*判断是否关闭成功*/
        }
        /*打印数据*/
        printf ( "The Card ID is: %02X%02X%02X%02X\r\n",resive_data [0], resive_data [1], resive_data [2],resive_data [3] );
        
    //     usleep(1000*10);
    // }


    /*关闭文件*/
    error = close(fd);
    if(error < 0)
    {
        printf("close file error! \n");
    }
    
    return 0;
}