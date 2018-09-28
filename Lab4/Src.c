#include<stdio.h>
#include<tinyara/gpio.h>
#include<apps/shell/tash.h>
#include<fcntl.h>

unsigned int gun_countStep=0;
unsigned int gun_direction=1;

int motorspeed=5;

static void gpio_write(int port, int value){
	char str[4];
	static char devpath[16];
	snprintf(devpath, 16, "/dev/gpio%d", port);
	int fd=open(devpath, O_RDWR);
	if(fd<0){
		printf("fd open fail\n");
		return;
	}
	ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);
	if(write(fd, str, snprintf(str, 4, "%d", value!=0)+1)<0){
		printf("write error\n");
		return;
	}
	close(fd);
}

void clockwise_2PhaseExcitation(){
	gpio_write(37, 1);
	gpio_write(30, 1);
	gpio_write(31, 0);
	gpio_write(32, 0);
	up_mdelay(motorspeed);

	gpio_write(37, 0);
	gpio_write(30, 1);
	gpio_write(31, 1);
	gpio_write(32, 0);
	up_mdelay(motorspeed);

	
	gpio_write(37, 0);
	gpio_write(30, 0);
	gpio_write(31, 1);
	gpio_write(32, 1);
	up_mdelay(motorspeed);

	
	gpio_write(37, 1);
	gpio_write(30, 0);
	gpio_write(31, 0);
	gpio_write(32, 1);
	up_mdelay(motorspeed);
}

void stepMotor(){
	int i=0;
	for(i=0;i<512;i++){
		clockwise_2PhaseExcitation();
	}
}

int main(int argc, FAR char *argv[]){
	tash_cmd_install("step", stepMotor, TASH_EXECMD_SYNC);
	return 0;
}