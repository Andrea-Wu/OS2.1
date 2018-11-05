#define MAJOR_NUM 169
#define IOCTL_CREATE _IOR(MAJOR_NUM, 0, char*)
#define IOCTL_DESTROY _IOW(MAJOR_NUM, 1, int)


//struct for passing in parameters to IOCTL_CHANGE KEY

typedef struct changeKeyParam{
    int id;
    char* key;
} changeKeyParam;

#define IOCTL_GET_KEY _IOW(MAJOR_NUM, 3, int)
#define IOCTL_CHANGE_KEY _IOW(MAJOR_NUM, 2, changeKeyParam*)
