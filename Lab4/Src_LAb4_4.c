#include<stdio.h>
#include<tinyara/gpio.h>
#include<tinyara/config.h>
#include<apps/shell/tash.h>
// other Path #include</**/arch/arm/src/s5j/s5j_gpio.h>

int motorSpeed = 1000;
int pins[4];

void motorStep(){
	for(int i=0;i<4;i++){
		s5j_gpiowrite(pins[i ? i - 1 : 3], 0);
		s5j_gpiowrite(pins[i], 1);
		up_udelay(motorSpeed);
	}
}

int getint(char *str){
	int idx = 0, value = 0;
	if(str[0]=='-') idx = 1;
	while(str[idx] >= '0' && str[idx] <= '9'){
		value *= 10; value += str[idx++] - '0';
	}
	return str[0] == '-' ? -value : value;
}

int temp(int argc, FAR char *argv[]){
	printf("%d\n", argc);
	for(int i=0;i<argc;i++){
		printf("%s\n",argv[i]);
	}
	if(argc > 1)motorSpeed = getint(argv[1]);
	printf("motor speed: %d\n", motorSpeed);
	while(1){
		motorStep();
	}
	return 0;
}

int main(int argc, FAR char *argv[]){
	pins[0]= GPIO_OUTPUT | GPIO_PULLUP | GPIO_PORTG0 | GPIO_PIN0;
	pins[1]= GPIO_OUTPUT | GPIO_PULLUP | GPIO_PORTG0 | GPIO_PIN1;
	pins[2]= GPIO_OUTPUT | GPIO_PULLUP | GPIO_PORTG0 | GPIO_PIN2;
	pins[3]= GPIO_OUTPUT | GPIO_PULLUP | GPIO_PORTG0 | GPIO_PIN3;
	for(int i=0;i<4;i++) s5j_configgpio(pins[i]);
	for(int i=0;i<4;i++) s5j_gpiowrite(pins[i], 0);
	tash_cmd_install("step", temp, TASH_EXECMD_SYNC);
	return 0;
}