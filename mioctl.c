#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_CHANGEKEY _IOW('n' ,1,int)
#define MAJOR_NUM 509

int main(int argc,char *argv[]) {
    if(argc<2){
        printf("Usage: %s <new_key>\n", argv[0]);
        return -1;
    }
    int fd = open("/dev/chardev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    int newKey = atoi(argv[1]);

    if (ioctl(fd,  IOCTL_CHANGEKEY, &newKey) < 0) {
        perror("ioctl failed");
        close(fd);
        return -1;
    }

    printf("IOCTL command executed successfully\n");

    close(fd);
    return 0;
}
