
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <linux/timer.h> 
#include <linux/fcntl.h>

#define SMPLRT_DIV                                     0x19
#define CONFIG                                      		 0x1A
#define GYRO_CONFIG                                 0x1B
#define ACCEL_CONFIG                                0x1C
#define ACCEL_XOUT_H                               0x3B
#define ACCEL_XOUT_L                                0x3C
#define ACCEL_YOUT_H                               0x3D
#define ACCEL_YOUT_L                                0x3E
#define ACCEL_ZOUT_H                               0x3F
#define ACCEL_ZOUT_L                                0x40
#define TEMP_OUT_H                                    0x41
#define TEMP_OUT_L                                    0x42
#define GYRO_XOUT_H                                 0x43
#define GYRO_XOUT_L                                  0x44
#define GYRO_YOUT_H                                 0x45
#define GYRO_YOUT_L                                  0x46
#define GYRO_ZOUT_H                                 0x47
#define GYRO_ZOUT_L                                  0x48
#define PWR_MGMT_1                                   0x6B
#define WHO_AM_I                                          0x75

#define MPU_INT_EN_REG			               0X38	//中断使能寄存器
#define MPU_USER_CTRL_REG		         0X6A	//用户控制寄存器
#define MPU_FIFO_EN_REG			              0X23	//FIFO使能寄存器
#define MPU_PWR_MGMT2_REG		      0X6C	//电源管理寄存器2 
#define MPU_INTBP_CFG_REG		          0X37	//中断/旁路设置寄存器

/*------------------字符设备内容----------------------*/
#define DEV_NAME "I2C1_mpu6050_2"
#define DEV_CNT (1)
#define TIME_FOR_TIMER  300

/*定义 led 资源结构体，保存获取得到的节点信息以及转换后的虚拟寄存器地址*/
static dev_t mpu6050_devno;				 //定义字符设备的设备号
static struct cdev mpu6050_chr_dev;		 //定义字符设备结构体chr_dev
struct class *class_mpu6050;			 //保存创建的类
struct device *device_mpu6050;			 // 保存创建的设备
struct device_node *mpu6050_device_node; //rgb_led的设备树节点结构体

short mpu6050_result[6]; 
bool isbreak;
struct timer_list timer;   //定时器
struct fasync_struct *fasync_queue;
static atomic_t blcok_atomic = ATOMIC_INIT(0);

/*------------------IIC设备内容----------------------*/
struct i2c_client *mpu6050_client = NULL; //保存mpu6050设备对应的i2c_client结构体，匹配成功后由.prob函数带回！。

/*通过i2c 向mpu6050写入数据
*mpu6050_client：mpu6050的i2c_client结构体。
*address, 数据要写入的地址，
*data, 要写入的数据
*返回值，错误，-1。成功，0  
*/
static int i2c_write_mpu6050(struct i2c_client *mpu6050_client, u8 address, u8 data)
{
	int error = 0;
	u8 write_data[2];
	struct i2c_msg send_msg; //要发送的数据结构体

	/*设置要发送的数据*/
	write_data[0] = address;
	write_data[1] = data;

	/*发送 iic要写入的地址 reg*/
	send_msg.addr = mpu6050_client->addr; //mpu6050在 iic 总线上的地址，也就是设备树的reg<0x68>
	send_msg.flags = 0;					  //标记为发送数据
	send_msg.buf = write_data;			  //写入的首地址
	send_msg.len = 2;					  //reg长度?write_data长度

	/*执行发送*/
	error = i2c_transfer(mpu6050_client->adapter, &send_msg, 1);
	if (error != 1)
	{
		printk(KERN_DEBUG "\n i2c_transfer error \n");
		return -1;
	}
	return 0;
}

/*6.通过i2c 向mpu6050写入数据
*mpu6050_client：mpu6050的i2c_client结构体。
*address, 要读取的地址，
*data，保存读取得到的数据
*length，读长度
*返回值，错误，-1。成功，0
*/
static int i2c_read_mpu6050(struct i2c_client *mpu6050_client, u8 address, void *data, u32 length)
{
	int error = 0;
	u8 address_data = address;
	struct i2c_msg mpu6050_msg[2];
	/*设置读取位置msg*/
	mpu6050_msg[0].addr = mpu6050_client->addr; //mpu6050在 iic 总线上的地址
	mpu6050_msg[0].flags = 0;					//标记为发送数据
	mpu6050_msg[0].buf = &address_data;			//写入的首地址
	mpu6050_msg[0].len = 1;						//写入长度

	/*设置读取位置msg*/
	mpu6050_msg[1].addr = mpu6050_client->addr; //mpu6050在 iic 总线上的地址
	mpu6050_msg[1].flags = I2C_M_RD;			//标记为读取数据
	mpu6050_msg[1].buf = data;					//读取得到的数据保存位置
	mpu6050_msg[1].len = length;				//读取长度

	//第一个meg是要读取的地址，第二个是读取的值
	error = i2c_transfer(mpu6050_client->adapter, mpu6050_msg, 2);
	 

	if (error != 2)
	{
		printk(KERN_DEBUG "\n i2c_read_mpu6050 error \n");
		return -1;
	}
	return 0;
}

//设置MPU6050陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return i2c_write_mpu6050(mpu6050_client,GYRO_CONFIG, fsr<<3);//设置陀螺仪满量程范围  
}
//设置MPU6050加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return i2c_write_mpu6050(mpu6050_client, ACCEL_CONFIG,fsr<<3);//设置加速度传感器满量程范围  
}
//设置MPU6050的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return i2c_write_mpu6050(mpu6050_client, CONFIG, data);//设置数字低通滤波器  
}
//设置MPU6050的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=i2c_write_mpu6050(mpu6050_client, SMPLRT_DIV, data);	//设置数字低通滤波器
 	return MPU_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}

/*5.初始化i2c
*返回值，成功，返回0。失败，返回 -1
*/
static int mpu6050_init(void)
{
	int error = 0;
	u8 res;

	//i2c_write_mpu6050这个函数主要将数据打包好然后通过i2c_transfer发送
	error += i2c_write_mpu6050(mpu6050_client, PWR_MGMT_1, 0X80); 
	mdelay(100);
	error += i2c_write_mpu6050(mpu6050_client,  PWR_MGMT_1,0X00);	//唤醒MPU6050 
	MPU_Set_Gyro_Fsr(3);					//陀螺仪传感器,±2000dps
	MPU_Set_Accel_Fsr(0);					//加速度传感器,±2g
	MPU_Set_Rate(50);						//设置采样率50Hz
	error += i2c_write_mpu6050(mpu6050_client,  MPU_INT_EN_REG,0X00);	//关闭所有中断
	error += i2c_write_mpu6050(mpu6050_client,  MPU_USER_CTRL_REG,0X00);	//I2C主模式关闭
	error += i2c_write_mpu6050(mpu6050_client,  MPU_FIFO_EN_REG,0X00);	//关闭FIFO
	error += i2c_write_mpu6050(mpu6050_client,  MPU_INTBP_CFG_REG,0X80);	//INT引脚低电平有效
	i2c_read_mpu6050(mpu6050_client, ACCEL_XOUT_H, &res, 1); 
	if(res==0X68)//器件ID正确
	{
		error += i2c_write_mpu6050(mpu6050_client,  PWR_MGMT_1,0X01);	//设置CLKSEL,PLL X轴为参考
		error += i2c_write_mpu6050(mpu6050_client,  MPU_PWR_MGMT2_REG,0X00);	//加速度与陀螺仪都工作
		MPU_Set_Rate(50);						//设置采样率为50Hz
		printk("okokokok \n");
	}

	// error += i2c_write_mpu6050(mpu6050_client, PWR_MGMT_1, 0X00);   //唤醒MPU6050 

	// /*设置MPU6050的采样频率*/
	// error += i2c_write_mpu6050(mpu6050_client, SMPLRT_DIV, 0X07);
	// /*设置数字低通滤波器和帧同步引脚采样*/
	// error += i2c_write_mpu6050(mpu6050_client, CONFIG, 0X06);
	// /*设置量程和 X、Y、Z 轴加速度自检*/
	// error += i2c_write_mpu6050(mpu6050_client, ACCEL_CONFIG, 0X01);

	if (error < 0)
	{
		/*初始化错误*/
		printk(KERN_DEBUG "\n mpu6050_init error \n");
		return -1;
	}
	return 0;
}

static void timer_function(struct timer_list *unused)
{
	//printk("timer_function!\n");
// char data_H;
// 	char data_L;
// 	i2c_read_mpu6050(mpu6050_client, ACCEL_XOUT_H, &data_H, 1); 
// 	i2c_read_mpu6050(mpu6050_client, ACCEL_XOUT_L, &data_L, 1);
// 	mpu6050_result[0] = data_H << 8;
// 	mpu6050_result[0] += data_L;

// 	i2c_read_mpu6050(mpu6050_client, ACCEL_YOUT_H, &data_H, 1);
// 	i2c_read_mpu6050(mpu6050_client, ACCEL_YOUT_L, &data_L, 1);
// 	mpu6050_result[1] = data_H << 8;
//     mpu6050_result[1] += data_L;

// 	i2c_read_mpu6050(mpu6050_client, ACCEL_ZOUT_H, &data_H, 1);
// 	i2c_read_mpu6050(mpu6050_client, ACCEL_ZOUT_L, &data_L, 1);
// 	mpu6050_result[2] = data_H << 8;
// 	mpu6050_result[2] += data_L;

// 	i2c_read_mpu6050(mpu6050_client, GYRO_XOUT_H, &data_H, 1);
// 	i2c_read_mpu6050(mpu6050_client, GYRO_XOUT_L, &data_L, 1);
// 	mpu6050_result[3] = data_H << 8;
// 	mpu6050_result[3] += data_L;

// 	i2c_read_mpu6050(mpu6050_client, GYRO_YOUT_H, &data_H, 1);
// 	i2c_read_mpu6050(mpu6050_client, GYRO_YOUT_L, &data_L, 1);
// 	mpu6050_result[4] = data_H << 8;
// 	mpu6050_result[4] += data_L;

// 	i2c_read_mpu6050(mpu6050_client, GYRO_ZOUT_H, &data_H, 1);
// 	i2c_read_mpu6050(mpu6050_client, GYRO_ZOUT_L, &data_L, 1);
// 	mpu6050_result[5] = data_H << 8;
// 	mpu6050_result[5] += data_L;


	kill_fasync(&fasync_queue, SIGIO, POLL_IN);   //发送信号

	if(atomic_read(&blcok_atomic))
	{
		mod_timer(&timer ,jiffies+ msecs_to_jiffies(TIME_FOR_TIMER));
	}
}

/*5.字符设备操作函数集，open函数实现*/
static int mpu6050_open(struct inode *inode, struct file *filp)
{
	// printk("\n mpu6050_open \n");

	/*向 mpu6050 发送配置数据，让mpu6050处于正常工作状态*/
	mpu6050_init();

	atomic_set(&blcok_atomic, 1);
	mod_timer(&timer ,jiffies+ msecs_to_jiffies(TIME_FOR_TIMER));
	return 0;
}

/*6.字符设备操作函数集，.read函数实现*/
static ssize_t mpu6050_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	int error;	
	char data_H;
	char data_L;
	i2c_read_mpu6050(mpu6050_client, ACCEL_XOUT_H, &data_H, 1); 
	i2c_read_mpu6050(mpu6050_client, ACCEL_XOUT_L, &data_L, 1);
	mpu6050_result[0] = data_H << 8;
	mpu6050_result[0] += data_L;

	i2c_read_mpu6050(mpu6050_client, ACCEL_YOUT_H, &data_H, 1);
	i2c_read_mpu6050(mpu6050_client, ACCEL_YOUT_L, &data_L, 1);
	mpu6050_result[1] = data_H << 8;
    mpu6050_result[1] += data_L;

	i2c_read_mpu6050(mpu6050_client, ACCEL_ZOUT_H, &data_H, 1);
	i2c_read_mpu6050(mpu6050_client, ACCEL_ZOUT_L, &data_L, 1);
	mpu6050_result[2] = data_H << 8;
	mpu6050_result[2] += data_L;

	i2c_read_mpu6050(mpu6050_client, GYRO_XOUT_H, &data_H, 1);
	i2c_read_mpu6050(mpu6050_client, GYRO_XOUT_L, &data_L, 1);
	mpu6050_result[3] = data_H << 8;
	mpu6050_result[3] += data_L;

	i2c_read_mpu6050(mpu6050_client, GYRO_YOUT_H, &data_H, 1);
	i2c_read_mpu6050(mpu6050_client, GYRO_YOUT_L, &data_L, 1);
	mpu6050_result[4] = data_H << 8;
	mpu6050_result[4] += data_L;

	i2c_read_mpu6050(mpu6050_client, GYRO_ZOUT_H, &data_H, 1);
	i2c_read_mpu6050(mpu6050_client, GYRO_ZOUT_L, &data_L, 1);
	mpu6050_result[5] = data_H << 8;
	mpu6050_result[5] += data_L;

	/*将读取得到的数据拷贝到用户空间*/
	error = copy_to_user(buf, mpu6050_result, cnt);

	if(error != 0)
	{
		printk("copy_to_user error!");
		return -1;
	}
	return 0;
}

static int my_fasync(int fd, struct file *file, int on)
{
	return fasync_helper(fd, file, on, &fasync_queue);   //当应用程序开启异步通知后执行，初始化异步通知结构体
}

/*字符设备操作函数集，.release函数实现*/
static int mpu6050_release(struct inode *inode, struct file *filp)
{
	// printk("\n mpu6050_release \n");
	atomic_set(&blcok_atomic, 0);
	return my_fasync(-1, filp,  0);    //释放异步通知
}

/*4.字符设备操作函数集*/
static struct file_operations mpu6050_chr_dev_fops =
	{
		.owner = THIS_MODULE,
		.open = mpu6050_open,
		.read = mpu6050_read,
		.fasync = my_fasync,
		.release = mpu6050_release,
};

/*----------------3.平台驱动函数集-----------------*/
//client是根据设备树创建的client，在这个probe函数中返回。
static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int ret = -1; //保存错误状态码

	printk(KERN_EMERG "\t  match successed  \n");
	/*---------------------注册 字符设备部分-----------------*/

	//采用动态分配的方式，获取设备编号，次设备号为0，
	//设备名称为rgb-leds，可通过命令cat  /proc/devices查看
	//DEV_CNT为1，当前只申请一个设备编号
	ret = alloc_chrdev_region(&mpu6050_devno, 0, DEV_CNT, DEV_NAME);
	if (ret < 0)
	{
		printk("fail to alloc mpu6050_devno\n");
		goto alloc_err;
	}

	//关联字符设备结构体cdev与文件操作结构体file_operations
	mpu6050_chr_dev.owner = THIS_MODULE;
	cdev_init(&mpu6050_chr_dev, &mpu6050_chr_dev_fops);

	// 添加设备至cdev_map散列表中
	ret = cdev_add(&mpu6050_chr_dev, mpu6050_devno, DEV_CNT);
	if (ret < 0)
	{
		printk("fail to add cdev\n");
		goto add_err;
	}

	/*创建类 */
	class_mpu6050 = class_create(THIS_MODULE, DEV_NAME);

	/*创建设备 DEV_NAME 指定设备名，*/
	device_mpu6050 = device_create(class_mpu6050, NULL, mpu6050_devno, NULL, DEV_NAME);
	mpu6050_client = client;

	timer_setup(&timer, timer_function, 0);    //初始化定时器

	return 0;

add_err:
	// 添加设备失败时，需要注销设备号
	unregister_chrdev_region(mpu6050_devno, DEV_CNT);
	printk("\n error! \n");
alloc_err:

	return -1;
}


static int mpu6050_remove(struct i2c_client *client)
{
	del_timer(&timer);  //删定时器
	/*删除设备*/
	device_destroy(class_mpu6050, mpu6050_devno);	  //清除设备
	class_destroy(class_mpu6050);					  //清除类
	cdev_del(&mpu6050_chr_dev);						  //清除设备号
	unregister_chrdev_region(mpu6050_devno, DEV_CNT); //取消注册字符设备
	return 0;
}



/*定义ID 匹配表*/
static const struct i2c_device_id gtp_device_id[] = {
	{"fire,i2c_mpu6050", 0},
	{}};

/*定义设备树匹配表*/
static const struct of_device_id mpu6050_of_match_table[] = {
	{.compatible = "fire,i2c_mpu6050"},  //跟设备树里面的compatible一样
	{/* sentinel */}};

/*2.定义i2c总线设备结构体*/
struct i2c_driver mpu6050_driver = {
	.probe = mpu6050_probe,        //匹配成功之后，执行的函数
	.remove = mpu6050_remove,
	.id_table = gtp_device_id,
	.driver = {
		.name = "fire,i2c_mpu6050",    //跟设备树里面的compatible一样??
		.owner = THIS_MODULE,
		.of_match_table = mpu6050_of_match_table,
	},
};

/*
*驱动初始化函数
*/
static int __init mpu6050_driver_init(void)
{
	int ret;
	pr_info("mpu6050_driver_init\n");
	ret = i2c_add_driver(&mpu6050_driver);  // 1.注册i2c设备驱动，关键看这个结构体
	return ret;
}

/*
*驱动注销函数
*/
static void __exit mpu6050_driver_exit(void)
{
	pr_info("mpu6050_driver_exit\n");
	i2c_del_driver(&mpu6050_driver);
}

module_init(mpu6050_driver_init);
module_exit(mpu6050_driver_exit);

MODULE_LICENSE("GPL");
