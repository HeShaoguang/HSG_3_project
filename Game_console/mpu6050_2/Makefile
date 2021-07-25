KERNEL_DIR=/home/pi/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := i2c_mpu6050_2.o 

all:app
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

app:
	arm-linux-gnueabihf-gcc app3.c -lm -o App3

.PHONE:clean copy 

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	sudo rm  /home/guang01/workdir/*.ko	App

copy:
	sudo  cp  *.ko App3  /home/guang01/workdir