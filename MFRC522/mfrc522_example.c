#include<stdio.h>
#include "mfrc522.h"

int main(){
    mfrc522_init();

    u_char i, temp;
    u_char status;
    u_char str[64] = {0,}, RC_size;

    while(1){
        up_mdelay(20);
        status = MFRC522_Request(PICC_REQIDL, str);
        if(status == MI_OK){
            printf("Card detected: %x %x\n", (int)str[0], (int)str[1]);
        }
        
        status = MFRC522_Anticoll(str);
        if(status == MI_OK){
            for(i = 0; i < 5; i++){
                printf("%x ", (int)str[i]);
            }
            printf(" \n");
            MFRC522_Halt();
        }
    }
    
    return 0;
}
