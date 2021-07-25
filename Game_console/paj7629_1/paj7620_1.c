
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
#include "paj7620_1.h"
/*------------------字符设备内容----------------------*/
#define DEV_NAME "paj7620_1"
#define DEV_CNT (1)

/*定义 led 资源结构体，保存获取得到的节点信息以及转换后的虚拟寄存器地址*/
static dev_t paj7620_devno;				 //定义字符设备的设备号
static struct cdev paj7620_chr_dev;		 //定义字符设备结构体chr_dev
struct class *class_paj7620;			 //保存创建的类
struct device *device_paj7620;			 // 保存创建的设备

/*------------------IIC设备内容----------------------*/
struct i2c_client *paj7620_client = NULL; //保存paj7620设备对应的i2c_client结构体，匹配成功后由.prob函数带回！。
static bool isok = false;
/*通过i2c 向paj7620写入数据
*paj7620_client：paj7620的i2c_client结构体。
*address, 数据要写入的地址，
*data, 要写入的数据
*返回值，错误，-1。成功，0  
*/
static int i2c_write_paj7620(u8 address, u8 data)
{
	int error = 0;
	u8 write_data[2];
	struct i2c_msg send_msg; //要发送的数据结构体

	/*设置要发送的数据*/
	write_data[0] = address;
	write_data[1] = data;

	/*发送 iic要写入的地址 reg*/
	send_msg.addr = paj7620_client->addr; //paj7620在 iic 总线上的地址，也就是设备树的reg<0x68>
	send_msg.flags = 0;					  //标记为发送数据
	send_msg.buf = write_data;			  //写入的首地址
	send_msg.len = 2;					  //reg长度?write_data长度

	/*执行发送*/
	error = i2c_transfer(paj7620_client->adapter, &send_msg, 1);
	if (error != 1)
	{
		printk(KERN_DEBUG "\n i2c_transfer error \n");
		return -1;
	}
	return 0;
}

/*6.通过i2c 向paj7620写入数据
*address, 要读取的地址，
*data，保存读取得到的数据
*length，读长度
*返回值，错误，-1。成功，0
*/
static int i2c_read_paj7620(u8 address, u32 length, void *data)
{
	int error = 0;
	u8 address_data = address;
	struct i2c_msg paj7620_msg[2];
	/*设置读取位置msg*/
	paj7620_msg[0].addr = paj7620_client->addr; //paj7620在 iic 总线上的地址
	paj7620_msg[0].flags = 0;					//标记为发送数据
	paj7620_msg[0].buf = &address_data;			//写入的首地址
	paj7620_msg[0].len = 1;						//写入长度

	/*设置读取位置msg*/
	paj7620_msg[1].addr = paj7620_client->addr; //paj7620在 iic 总线上的地址
	paj7620_msg[1].flags = I2C_M_RD;			//标记为读取数据
	paj7620_msg[1].buf = data;					//读取得到的数据保存位置
	paj7620_msg[1].len = length;				//读取长度

	//第一个meg是要读取的地址，第二个是读取的值
	error = i2c_transfer(paj7620_client->adapter, paj7620_msg, 2);
	 

	if (error != 2)
	{
		printk(KERN_DEBUG "\n i2c_read_paj7620 error \n");
		return -1;
	}
	return 0;
}

static int GS_WakeUp(void)
{
	int error = 0;
	u8 write_data;
	struct i2c_msg send_msg; //要发送的数据结构体

	/*设置要发送的数据*/
	write_data= 0x73;

	/*发送 iic要写入的地址 reg*/
	send_msg.addr = paj7620_client->addr; //paj7620在 iic 总线上的地址，也就是设备树的reg<0x68>
	send_msg.flags = 0;					  //标记为发送数据
	send_msg.buf = &write_data;			  //写入的首地址
	send_msg.len = 1;					  //reg长度?write_data长度

	/*执行发送*/
	error = i2c_transfer(paj7620_client->adapter, &send_msg, 1);
	if (error != 1)
	{
		printk(KERN_DEBUG "\n i2c_transfer error \n");
		return -1;
	}
	return 0;
}

//PAJ7620U2唤醒
static u8 paj7620u2_wakeup(void)
{ 
	u8 data=0x0a;
	GS_WakeUp();//唤醒PAJ7620U2
	mdelay(5);;//唤醒时间>400us
	GS_WakeUp();//唤醒PAJ7620U2
	mdelay(5);//唤醒时间>400us
	i2c_write_paj7620(PAJ_REGITER_BANK_SEL, PAJ_BANK0); //进入BANK0寄存器区域
	i2c_read_paj7620(0x00 , 1,  &data);
	if(data!=0x20) return 0; //唤醒失败
	printk("paj7620u2_wakeup successed \n");	
	return 1;
}

/*5.初始化i2c
*返回值，成功，返回0。失败，返回 -1
*/
static int paj7620_init(void)
{
	int error = 0;
	int i;
	
	if(paj7620u2_wakeup())
	{
			error += i2c_write_paj7620(PAJ_REGITER_BANK_SEL, PAJ_BANK0); //进入BANK0寄存器区域
			for(i=0;i<INIT_SIZE;i++)
			{
				error += i2c_write_paj7620(init_Array[i][0],init_Array[i][1]);//初始化PAJ7620U2
			}
			for(i=0;i<GESTURE_SIZE;i++)
			{
				error += i2c_write_paj7620(gesture_arry[i][0],gesture_arry[i][1]);//手势识别模式初始化
			}
			error += i2c_write_paj7620(PAJ_REGITER_BANK_SEL, PAJ_BANK0);;//切换回BANK0寄存器区域
	}else
	{
		/*初始化错误*/
		printk(KERN_DEBUG "\n paj7620_init paj7620u2_wakeup error \n");
		return -1;
	}
\
	if (error < 0)
	{
		/*初始化错误*/
		printk(KERN_DEBUG "\n paj7620_init error \n");
		return -1;
	}
	return 0;
}

/*5.字符设备操作函数集，open函数实现*/
static int paj7620_open(struct inode *inode, struct file *filp)
{
	// printk("\n paj7620_open \n");
	int i;
	/*向 paj7620 发送配置数据，让paj7620处于正常工作状态*/
	for(i=0; i<=5; i++)
	{
		if(paj7620_init() == 0)
		{
			isok = true;
			printk("paj7620_init successed \n");
			break;
		}
	}
	
	return 0;
}

/*6.字符设备操作函数集，.read函数实现*/
static ssize_t paj7620_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	int error = 0;
	u8 data = 0x00;	

	if(isok)
	{
		i2c_read_paj7620(PAJ_GET_INT_FLAG1, 1 ,&data);  //读取手势状态
	}

	/*将读取得到的数据拷贝到用户空间*/
	error = copy_to_user(buf, &data, 1);

	if(error != 0)
	{
		printk("copy_to_user error!");
		return -1;
	}
	return 0;
}

/*字符设备操作函数集，.release函数实现*/
static int paj7620_release(struct inode *inode, struct file *filp)
{
	// printk("\n paj7620_release \n");
	if(isok)
	{
		i2c_write_paj7620(PAJ_SET_INT_FLAG1,0X00);//关闭手势识别中断输出
		i2c_write_paj7620(PAJ_SET_INT_FLAG2,0X00);
	}

	return 0;
}

/*4.字符设备操作函数集*/
static struct file_operations paj7620_chr_dev_fops =
	{
		.owner = THIS_MODULE,
		.open = paj7620_open,
		.read = paj7620_read,
		.release = paj7620_release,
};

/*----------------3.平台驱动函数集-----------------*/
//client是根据设备树创建的client，在这个probe函数中返回。
static int paj7620_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int ret = -1; //保存错误状态码

	printk(KERN_EMERG "\t  match successed  \n");
	/*---------------------注册 字符设备部分-----------------*/

	//采用动态分配的方式，获取设备编号，次设备号为0，
	//设备名称为rgb-leds，可通过命令cat  /proc/devices查看
	//DEV_CNT为1，当前只申请一个设备编号
	ret = alloc_chrdev_region(&paj7620_devno, 0, DEV_CNT, DEV_NAME);
	if (ret < 0)
	{
		printk("fail to alloc paj7620_devno\n");
		goto alloc_err;
	}

	//关联字符设备结构体cdev与文件操作结构体file_operations
	paj7620_chr_dev.owner = THIS_MODULE;
	cdev_init(&paj7620_chr_dev, &paj7620_chr_dev_fops);

	// 添加设备至cdev_map散列表中
	ret = cdev_add(&paj7620_chr_dev, paj7620_devno, DEV_CNT);
	if (ret < 0)
	{
		printk("fail to add cdev\n");
		goto add_err;
	}

	/*创建类 */
	class_paj7620 = class_create(THIS_MODULE, DEV_NAME);

	/*创建设备 DEV_NAME 指定设备名，*/
	device_paj7620 = device_create(class_paj7620, NULL, paj7620_devno, NULL, DEV_NAME);
	paj7620_client = client;
	return 0;

add_err:
	// 添加设备失败时，需要注销设备号
	unregister_chrdev_region(paj7620_devno, DEV_CNT);
	printk("\n error! \n");
alloc_err:

	return -1;
}


static int paj7620_remove(struct i2c_client *client)
{
	/*删除设备*/
	device_destroy(class_paj7620, paj7620_devno);	  //清除设备
	class_destroy(class_paj7620);					  //清除类
	cdev_del(&paj7620_chr_dev);						  //清除设备号
	unregister_chrdev_region(paj7620_devno, DEV_CNT); //取消注册字符设备
	return 0;
}



/*定义ID 匹配表*/
static const struct i2c_device_id gtp_device_id[] = {
	{"myself,paj7620u2_1", 0},
	{}};

/*定义设备树匹配表*/
static const struct of_device_id paj7620_of_match_table[] = {
	{.compatible = "myself,paj7620u2_1"},  //跟设备树里面的compatible一样
	{/* sentinel */}};

/*2.定义i2c总线设备结构体*/
struct i2c_driver paj7620_driver = {
	.probe = paj7620_probe,        //匹配成功之后，执行的函数
	.remove = paj7620_remove,
	.id_table = gtp_device_id,
	.driver = {
		.name = "myself,paj7620u2_1",    //跟设备树里面的compatible一样
		.owner = THIS_MODULE,
		.of_match_table = paj7620_of_match_table,
	},
};

/*
*驱动初始化函数
*/
static int __init paj7620_driver_init(void)
{
	int ret;
	pr_info("paj7620_driver_init\n");
	ret = i2c_add_driver(&paj7620_driver);  // 1.注册i2c设备驱动，关键看这个结构体
	return ret;
}

/*
*驱动注销函数
*/
static void __exit paj7620_driver_exit(void)
{
	pr_info("paj7620_driver_exit\n");
	i2c_del_driver(&paj7620_driver);
}

module_init(paj7620_driver_init);
module_exit(paj7620_driver_exit);

MODULE_LICENSE("GPL");
