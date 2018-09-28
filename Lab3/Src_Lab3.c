#include<stdio.h>
#include<fcntl.h>
#include<tinyara/analog/adc.h>
#include<tinyara/analog/ioctl.h>
#include<apps/shell/tash.h>
#include<tinyara/gpio.h>
#include<tinyara/config.h>

#define GPIO_FUNC_SHIFT 13
#define GPIO_INPUT (0x0 << GPIO_FUNC_SHIFT)
#define GPIO_OUTPUT (0x1 << GPIO_FUNC_SHIFT)

#define GPIO_PORT_SHIFT 3
#define GPIO_PORTG1 (0x5 << GPIO_PORT_SHIFT)
#define GPIO_PORTG2 (0x6 << GPIO_PORT_SHIFT)

#define GPIO_PIN_SHIFT 0
#define GPIO_PIN2 (0x2 << GPIO_PIN_SHIFT)

#define GPIO_PUPD_SHIFT 11
#define GPIO_PULLDOWN (0x1 << GPIO_PUPD_SHIFT)
#define GPIO_PULLUP (0x3 << GPIO_PUPD_SHIFT)

#define ADC_MAX_SAMPLES 4

int gasValue(int fd, int idx){ // cds, gas

    int ret, retval;
    struct adc_msg_s samples[ADC_MAX_SAMPLES];
    ssize_t nbytes;

    ret=ioctl(fd, ANIOC_TRIGGER, 0);
    nbytes=read(fd, samples, sizeof(samples));
    int nsamples=nbytes/sizeof(struct adc_msg_s);
    int i;
	retval=-1;
    for(i=0;i<nsamples;i++){
       	if(samples[i].am_channel == idx){
    		retval = samples[i].am_data;break;
		}
    }
	return retval;
}

int distance(uint32_t cfgcon_trig, uint32_t cfgcon_echo){
	float dist;
	long cnt;
    cnt=0;
    s5j_gpiowrite(cfgcon_trig, 0);up_udelay(2);
    s5j_gpiowrite(cfgcon_trig, 1);up_udelay(10);
    s5j_gpiowrite(cfgcon_trig, 0);
    while(s5j_gpioread(cfgcon_echo)==0);
    while(s5j_gpioread(cfgcon_echo)==1){
        cnt++;up_udelay(1);
    }
    dist = cnt/29.0/2.0;
    return dist;
}


void init1(int *fd){
	(*fd) = open("/dev/adc0", O_RDONLY);
}

void init2(uint32_t *cfgcon_trig, uint32_t *cfgcon_echo){
	(*cfgcon_trig) = GPIO_OUTPUT | GPIO_PORTG1 | GPIO_PIN2;
	(*cfgcon_echo) = GPIO_INPUT | GPIO_PORTG2 | GPIO_PIN2;
    s5j_configgpio(*cfgcon_trig);
    s5j_configgpio(*cfgcon_echo);
}

static int digitalRead(int port){
	char buf[4];
	char devpath[16];
	snprintf(devpath, 16, "/dev/gpio%d", port);
	int fd=open(devpath, O_RDWR);

	ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_IN);
	read(fd, buf, sizeof(buf));
	close(fd);
	return buf[0]=='1';
}

int main(int argc, char *argv[]){
	int fd;
	uint32_t a1;
	uint32_t a2;
	init1(&fd);
	init2(&a1, &a2);
	while(1){
		if(digitalRead(45)==0){
			printf("Gas: %d\n", gasValue(fd, 0));
		}
		if(digitalRead(46)==0){
			printf("Cds: %d\n", gasValue(fd, 1));
		}
		if(digitalRead(48)==0){
			printf("dist: %d\n", distance(a1, a2));
		}
		up_mdelay(20);
	}
	close(fd);
	return 0;
}
