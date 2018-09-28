#include<stdio.h>
#include<fcntl.h>
#include<tinyara/gpio.h>
#include<tinyara/analog/adc.h>
#include<tinyara/analog/ioctl.h>

#define ADC_MAX_SAMPLES 4

int inputValue=0;
int samplingTime=280;
int delayTime=40;
int offtime=9680;

void main(int argc, char *argv[]){
	int fd, fd2, ret, ret2;
	char str[4];
	struct adc_msg_s samples[ADC_MAX_SAMPLES];
	ssize_t nbytes;

	fd=open("/dev/adc0", O_RDONLY);
	fd2=open("/dev/gpio29", O_RDWR);

	for(;;){
		ret=ioctl(fd, ANIOC_TRIGGER, 0);
		ret2=ioctl(fd2, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);
		
		write(fd2, str, snprintf(str, 4, "%d", 0)+1);
		up_udelay(samplingTime);

		nbytes=read(fd, samples, sizeof(samples));
		int nsamples=nbytes/sizeof(struct adc_msg_s);

		int i;
		inputValue=-1;
		int cnt=0;
		for(i=0;i<nsamples;i++){
			if(samples[i].am_channel==0){
				inputValue += samples[i].am_data/4;cnt++;
			}
		}
		inputValue/=cnt;
		up_udelay(delayTime);
		write(fd2, str, snprintf(str, 4, "%d", 1)+1);
		up_udelay(offtime);
		printf("%d\n", nsamples);
		printf("gas value : %d (on %d cnt)\n", inputValue, cnt);

		up_mdelay(200);
	}
	close(fd);close(fd2);
}