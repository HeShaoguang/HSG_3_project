#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//卡尔曼滤波参数与函数

 
//卡尔曼滤波
float Kalman_Filter(float angle_m, float gyro_m)//angleAx 和 gyroGy
{
    float dt=0.001;//注意：dt的取值为kalman滤波器采样时间
    float angle = 0, angle_dot;//角度和角速度
    float P[2][2] = {{ 1, 0 },
                    { 0, 1 }};
    float Pdot[4] ={ 0,0,0,0};
    float Q_angle=0.001, Q_gyro=0.005; //角度数据置信度,角速度数据置信度
    float R_angle=0.5 ,C_0 = 1;
    float q_bias=0, angle_err, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;

        angle+=(gyro_m-q_bias) * dt;
        angle_err = angle_m - angle;
        Pdot[0]=Q_angle - P[0][1] - P[1][0];
        Pdot[1]=- P[1][1];
        Pdot[2]=- P[1][1];
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

        printf("angle : %f !\n", angle);
        return angle;
}

// //一阶互补滤波
// float K1 =0.1; // 对加速度计取值的权重
// float dt=0.001;//注意：dt的取值为滤波器采样时间

// float yijiehubu_P(float angle_m, float gyro_m)//采集后计算的角度和角加速度
// {
//     float angle_P;
//     angle_P = K1 * angle_m + (1-K1) * (angle_P + gyro_m * dt);
//          return angle_P;
// }
 
// float yijiehubu_R(float angle_m, float gyro_m)//采集后计算的角度和角加速度
// {
//     float angle_R;
//      angle_R = K1 * angle_m + (1-K1) * (angle_R + gyro_m * dt);
//          return angle_R;
// }

int main(int argc, char *argv[])
{

    short resive_data[6];  //保存收到的 mpu6050转换结果数据，依次为 AX(x轴角度), AY, AZ 。GX(x轴加速度), GY ,GZ
    int error;
    float angle_1, angle_2;
    /*打开文件*/
    int fd;
        fd = open("/dev/I2C1_mpu6050", O_RDWR);
        if(fd < 0)
        {
            printf("open file : %s failed !\n", argv[0]);
            return -1;
        }

    /*读取数据*/
    while(1)
    {

        error = read(fd,resive_data,sizeof(resive_data));
        if(error < 0)
        {
            printf("write file error! \n");
            close(fd);
            /*判断是否关闭成功*/
        }
        /*打印数据*/
        printf("AX=%d, AY=%d, AZ=%d ",(int)resive_data[0],(int)resive_data[1],(int)resive_data[2]);
        printf("    GX=%d, GY=%d, GZ=%d \n \n",(int)resive_data[3],(int)resive_data[4],(int)resive_data[5]);
        Kalman_Filter((int)resive_data[0], (int)resive_data[4]);
        Kalman_Filter((int)resive_data[1], (int)resive_data[3]);
        //printf("angle1 : %f !\n", angle_1);
       // printf("angle : %f !\n", angle_R);
        
        usleep(1000*100);
    }
    
            /*关闭文件*/
        error = close(fd);
        if(error < 0)
        {
            printf("close file error! \n");
        }
    
    return 0;
}