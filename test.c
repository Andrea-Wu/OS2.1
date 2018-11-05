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

    ioctl(fd, IOCTL_CREATE, 100);
    ioctl(fd, IOCTL_CREATE, 50);
    ioctl(fd, IOCTL_CREATE, 101);
    ioctl(fd, IOCTL_CREATE, 51);
    ioctl(fd, IOCTL_CREATE, 2);
    ioctl(fd, IOCTL_CREATE, 0);

    ioctl(fd, IOCTL_DESTROY, 50);
    ioctl(fd, IOCTL_DESTROY, 101);
    ioctl(fd, IOCTL_DESTROY, 51);
    ioctl(fd, IOCTL_DESTROY, 100);


}
