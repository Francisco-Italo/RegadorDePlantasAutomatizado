#include<iostream>
#include<cstdlib>
#include<unistd.h>
#include<cstring>
#include <fcntl.h>

using namespace std;

const char exec[] = "./export_moisture_gpio";
string buff;
char str[2];

void export_gpio() { popen(exec, "w"); }

string get_value_sensor() {
	FILE *fp;
	fp = popen("cat /sys/class/gpio/gpio66/value", "r");
	
	while(fgets(str,2,fp) != NULL) buff.append(str);
	
	pclose(fp);
	return buff;
}

int main(){
	export_gpio();
	usleep(1000);
    
    string sensorUmi = get_value_sensor();
    
    
    int fd = open("umidade", O_RDWR);
    
    write(fd,sensorUmi.c_str(),2);
    
    close(fd);
}
