#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    
    int integer = atoi(argv[1]);    
    switch(integer){
        case 1:
            printf("create\n");

            //create 1010
            break;
        case 2:
            printf("delete\n");
            break;
        case 3: 
            printf("change_key\n");
            break;
        case 4: 
            printf("encrypt\n");
            break;
        case 5:
            printf("decrypt\n");
            break;
    }

}

