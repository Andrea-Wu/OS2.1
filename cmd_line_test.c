#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ioctl.h"
#include <string.h>
int main(int argc, char* argv[]){
    
    int integer;
    int fd;
    int id;

    fd = open("/dev/cryptctl", O_RDWR);
    if(fd == -1){
        printf("file could not be opened, try again\n");
        return -1;
    }
    
    integer = atoi(argv[1]);    
     
    if(!strcmp(argv[1], "create")){ 
        printf("create\n");
        if(!argv[2]){
            printf("please enter a key as argument\n");
            return -1;
        }

        //TODO: check if argv[2] is a valid key
       //try and ensure key is not more than 20 characters 
        ioctl(fd, IOCTL_CREATE, argv[2]);
    }else if(!strcmp(argv[1], "delete")){
        if(!argv[2]){
            printf("please enter a device ID as argument\n");
            return -1;
        }            
        id = atoi(argv[2]);
        if(id == 0){
            printf("please enter a valid ID argument\n");
            return -1;
        }
        printf("delete id %d\n", id);
        ioctl(fd, IOCTL_DESTROY, id);
    }else if(!strcmp(argv[1], "change_key")){
        printf("change_key\n");
    }else if(!strcmp(argv[1], "encrypt")){
        printf("encrypt\n");
    }else if(!strcmp(argv[1], "decrypt")){
        printf("decrypt\n");
    }else if(!strcmp(argv[1], "get_key")){
        if(!argv[2]){
            printf("please enter a device ID as argument\n");
            return -1;
        }
        ioctl(fd, IOCTL_GET_KEY, atoi(argv[2]));   
        printf("get key (output to /var/log/kern.log)\n");
    }else{
        printf("not a valid command\n");
    }

}

