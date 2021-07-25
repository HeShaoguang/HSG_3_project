KERNEL_DIR=/home/pi/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := rfid.o 

all:app
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

app:
	arm-linux-gnueabihf-gcc app_spi.c -lm -o App_spi

.PHONE:clean copy 

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	sudo rm  /home/guang01/workdir/*.ko	App_spi

copy:
	sudo  cp  *.ko App_spi  /home/guang01/workdir