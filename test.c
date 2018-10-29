#include "ioctl.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
int main(){
    int fd = open("/dev/cryptctl", O_RDWR);

    if(fd <-1 ){
        printf("failed to open\n");
    }else{
        printf("opened\n");
    }

    ioctl(fd, IOCTL_CREATE);
}
