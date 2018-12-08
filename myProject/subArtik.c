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

int main(int argc, FAR char *argv[]) {
    mfrc522_init();

    u_int published = 0;
    u_char status;
    int cnt = 0;
    u_char buf[64];
    int fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY);

    while(1){
        up_mdelay(20);
        char str[20]={0,};
        read(fd, str, 8);
        if(strcmp(str, "readRFID"))continue;
        status = MFRC522_Request(PICC_REQIDL, str);
        u_int A = 0;
        if(status == MI_OK){
            status = MFRC522_Anticoll(str);
              if(status == MI_OK){
                 A = ((u_int)str[0] << (8*3)) | ((u_int)str[1] << (8*2)) | ((u_int)str[2] << (8*1)) | (u_int)str[3];
              }
        }
       char ddd[20] = {0,};
       printf("%x\n", A);
        if(A){
           sprintf(ddd, "%x", A);
        }
       write(fd, ddd, 8);
    }
    return 0;
}
