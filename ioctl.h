#define MAJOR_NUM 169
#define IOCTL_CREATE _IOW(MAJOR_NUM, 0, int)
#define IOCTL_DESTROY _IOW(MAJOR_NUM, 1, int)


//untested
#define IOCCHANGE_KEY _IOW(MAJOR_NUM, 2, const char*)
