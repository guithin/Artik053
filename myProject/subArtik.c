#include <tinyara/analog/ioctl.h>
#include <string.h>
#include <tinyara/gpio.h>
#include <tinyara/config.h>
#include "mfrc522.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

typedef unsigned char u_char;
typedef unsigned int u_int;

int rfidread(char *buf, int flag){
	if(MFRC522_Request(PICC_REQIDL, buf) == MI_OK){
		if(MFRC522_Anticoll(buf) == MI_OK){
			return 0;
		}else if(flag == 0){
			rfidread(buf, 1);
		}else return -1;
	}else if(flag == 0){
		rfidread(buf, 1);
	}else return -1;
}

int main(int argc, FAR char *argv[]) {
    mfrc522_init();

    int fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY);

    while(1){
        up_mdelay(20);
        char str[20]={0,};
        read(fd, str, 8);
        if(strcmp(str, "readRFID"))continue;
        u_int A = 0;
       char ddd[20] = {0,};
       if(rfidread(ddd, 0) == 0)
          A = ((u_int)ddd[0] << (8*3)) | ((u_int)ddd[1] << (8*2)) | ((u_int)ddd[2] << (8*1)) | (u_int)ddd[3];
       printf("%x\n", A);
        if(A){
           sprintf(ddd, "%x", A);
        }
       write(fd, ddd, 8);
    }
    return 0;
}
