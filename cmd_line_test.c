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
    changeKeyParam* param;

    int sub_fd;
    char* devName;
    char* buffer;

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
        if(!argv[2] || !argv[3]){
            printf("please enter a key and device ID as argument\n");
            return -1;
        } 
        id = atoi(argv[3]);
        if(id == 0){
            printf("please enter a valid ID argument\n");
            return -1;
        }
        
        printf("change_key for device %d to %s\n", id, argv[2]);
        //create a struct for parameters
        param = (changeKeyParam*)malloc(sizeof(changeKeyParam));
        param -> id = id;
        param -> key = argv[2]; //might have to malloc this?

        ioctl(fd, IOCTL_CHANGE_KEY, param);
    }else if(!strcmp(argv[1], "encrypt")){
        if(!argv[2] || !argv[3]){
            printf("please enter a device ID and string as argument\n");
            return -1;
        }
        id = atoi(argv[2]);
        if(id == 0){
            printf("please enter a valid ID argument\n");
            return -1;
        }

        //create string name
        devName = (char*)malloc(sizeof(char)* 15);
        sprintf(devName, "/dev/fuck%d", id);        
        sub_fd = open(devName, O_RDWR);
        if(sub_fd == -1){
            printf("invalid device id\n");
            return -1;
        }
        if(write(sub_fd, argv[3], strlen(argv[3])) == -1){
            printf("failed to write to encrypt\n");
        }else{
            printf("wrote successfully to encrypt\n");
        }
        
        buffer = (char*)malloc(sizeof(char) * 1000);

        if(read(sub_fd,buffer,999, 0) == 0){
            printf("failed to read from encrypt\n");
        }else{
            printf("read successfully from encrypt\n");
            printf("read %s\n", buffer);    
        }
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

