#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

const int nanosec = 1e9;
char response[2];

void exportGpio(){
    FILE *io = NULL, *ioDir = NULL;
    int gpioPin = 66;
    char drct[] = "in";

    io = fopen("sys/class/gpio/export", "w");
    fseek(io, 0, SEEK_SET);
    fprintf(io, "%d", gpioPin);
    fflush(io);

    ioDir = fopen("/sys/class/gpio/gpio66/direction", "w");
    fseek(ioDir, 0, SEEK_SET);
    fprintf(ioDir, "%s", drct);
    fflush(ioDir);

    return;
}

void getValSensor() {
	FILE *catRes;

    catRes = popen("cat /sys/class/gpio/gpio66/value", "r");
	fgets(response,2,catRes);
	
	pclose(catRes);
	
    return;
}

int main(){
    while(1){
        if(!fork()){
            exportGpio();
        }else{
            wait(NULL);
            getValSensor();
            
            int fd = open("umiFile", O_RDWR);
            write(fd, response, 2);
            
            close(fd);
        }
        usleep(3*nanosec);
    }
}