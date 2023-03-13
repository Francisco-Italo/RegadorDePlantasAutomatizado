#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MOISTURE_DATA "umidade"
#define TEMP_DATA "temp_data"

const char cmd_temp_sensor[] = "./bmp180_temp";
const char cmd_umi_sensor[] = "./umidade";
char bomb[16];

void exec_sensor() { 
	popen(cmd_temp_sensor, "r");
	popen(cmd_umi_sensor, "r");
}
void bomb_func(char param) {
	sprintf(bomb, "./bomb_turnon %c", param);
	popen(bomb, "r");
	//pclose(bomb);
}

int main(void){
	char input_umidity[2];
	char input_temp[5];
	int umidity; float temp;
	
	
	while(1){
		exec_sensor();
		usleep(1000);
		
		int moisture_fd = open(MOISTURE_DATA, O_RDWR);
		
		if(!moisture_fd) {
			printf("Ocorreu um erro ao criar/acessa o arquivo moisture_data.\n");
		} else {
			read(moisture_fd, &input_umidity, 2);
			umidity = atoi(input_umidity);
		}
		
		int temp_fd = open(TEMP_DATA, O_RDWR);
		if(!temp_fd) {
			printf("Ocorreu um erro ao criar/acessar o arquivo temp_data.\n");
		} else {
			read(temp_fd, &input_temp, 5);
			temp = atof(input_temp);
			
			if(!umidity){			
				if(temp >= 28.0){
					bomb_func('1');
					usleep(8000000);
					bomb_func('0');
				}else{
					bomb_func('1');
					usleep(3000000);
					bomb_func('0');
				} 
			}
		}

		close(moisture_fd);
		close(temp_fd);
		
		usleep(5000000);
	}
		
	return 0;
}
