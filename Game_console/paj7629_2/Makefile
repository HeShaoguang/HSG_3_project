KERNEL_DIR=/home/pi/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := paj7620_2.o 

all:app
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

app:
	arm-linux-gnueabihf-gcc app2.c -lm -o App2

.PHONE:clean copy 

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	sudo rm  /home/guang01/workdir/*.ko	App2

copy:
	sudo  cp  *.ko App2  /home/guang01/workdir