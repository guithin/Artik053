#include<stdio.h>
#include<tinyara/gpio.h>
#include<tinyara/config.h>
#include<time.h>
#include<string.h>

#define GPIO_FUNC_SHIFT 13
#define GPIO_INPUT (0x0 << GPIO_FUNC_SHIFT)

#define GPIO_PORT_SHIFT 3
#define GPIO_PORTG2 (0x6 << GPIO_PORT_SHIFT)

#define GPIO_PIN_SHIFT 0
#define GPIO_PIN6 (0x6 << GPIO_PIN_SHIFT)

#define GPIO_PUPD_SHIFT 11
#define GPIO_PULLDOWN (0x1 << GPIO_PUPD_SHIFT)

uint32_t pir_signal;

int main(int argc, FAR char *argv[]){
	struct timespec currentTime;
	struct tm tm_now;
	char buf[32];

	size_t sz_buf = sizeof(buf);
	int pirState=0;
	pir_signal = GPIO_INPUT | GPIO_PORTG2 | GPIO_PIN6 | GPIO_PULLDOWN;

	s5j_configgpio(pir_signal);
	printf("Waiting for Motion Sensor Initialized.\n");
	up_mdelay(2000);

	printf("Detecting Motion...\n");

	while(1){
		if(s5j_gpioread(pir_signal)==1){
			if(pirState==0){
				clock_gettime(CLOCK_REALTIME, &currentTime);
				localtime_r((time_t*)&currentTime.tv_sec, &tm_now);
				strftime(buf, sz_buf, "%Y/%m/%d %H:%M:%S", &tm_now);
				printf("Motion Started: %s\n", buf);
				pirState=1;
			}
		}
		else{
			if(pirState==1){
				printf("Motion Ended!!\n");
				pirState=0;
			}
		}
	}
	return 0;
}