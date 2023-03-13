/**
 * Based on BMP180 pressure & temperature sensor on Beaglebone black Rev C using I2C bus
 *
 * OS: Linux beaglebone 3.8.13-bone70 #1 SMP Fri Jan 23 02:15:42 UTC 2015 armv7l GNU/Linux
 *
 * The i2c bus is on pins P9_19 as SCL, P9_20 as SDA
 *
 * The code is partly converted from https://arduinodiy.wordpress.com/2015/05/26/reading-the-bmp180-pressure-sensor-with-an-attiny85/
 * 
 * Original algorithm:
 * Author: Yingfeng Shen, yfshen@gmail.com
 * Date: 1.2016
 *
 * Based algorithm:
 * Author: Francisco Ítalo de A. Moraes
 * Bate: 12.2022
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_BUS   	2
#define DEV_ADDR   	0b1110111	// 0x77
#define RET(msg,r)	{fprintf(stderr,msg); return r;}

// define calibration data for temperature:
int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
uint16_t ac4, ac5, ac6;
long x1, x2, b5;

int i2c_open(int bus, uint8_t dev_addr, int timeout){
	char path[64];
	int	fd;

	sprintf(path, "/dev/i2c-%d", bus);
	fd = open(path, O_SYNC | O_RDWR);
	if (fd<0) RET("failed to open i2c device\n",-1);
	ioctl(fd,I2C_SLAVE, dev_addr);
	ioctl(fd,I2C_TIMEOUT,timeout);	// unit 10 ms

	return fd;
}

void i2c_close(int fd){
	if (fd>0) close(fd);

	fd=0;
}

int bmp180ReadInt(int fd, unsigned char address) {
	uint8_t data[2];

	data[0] = address;
	write(fd, data, 1); // write address

	if (read(fd, data, 2)!=2) RET("error in bmp180ReadInt\n",0);

	return (int) (data[0] << 8 | data[1]);
}

void readBmp180Calibration(int fd) {
	// Read calibration data from EEPROM
	ac1 = bmp180ReadInt(fd,0xAA);
	ac2 = bmp180ReadInt(fd,0xAC);
	ac3 = bmp180ReadInt(fd,0xAE);
	ac4 = bmp180ReadInt(fd,0xB0);
	ac5 = bmp180ReadInt(fd,0xB2);
	ac6 = bmp180ReadInt(fd,0xB4);
	b1 = bmp180ReadInt(fd,0xB6);
	b2 = bmp180ReadInt(fd,0xB8);
	mb = bmp180ReadInt(fd,0xBA);
	mc = bmp180ReadInt(fd,0xBC);
	md = bmp180ReadInt(fd,0xBE);
}

unsigned int bmp180ReadUT(int fd) {
	uint8_t data[8];
	int ret;

	data[0] = 0xf4;
	data[1] = 0x2e;
	ret = write(fd, data, 2);
	usleep(5000);

	data[0] = 0xf6; // read MSB first
	ret = write(fd, data, 1);

	// read temperature
	ret = read(fd, data, 2);

	if (ret != 2) {
		RET("Read temperature failed\n",0);
	}

	return (int) (data[0] << 8 | data[1]);
}

double bmp180CorrectTemperature(unsigned int ut) {
	x1 = (((long) ut - (long) ac6) * (long) ac5) >> 15;
	x2 = ((long) mc << 11) / (x1 + md);
	b5 = x1 + x2;

	return (((b5 + 8) >> 4)) / 10.0;
}

int main(int argc, char* argv[]) {
	int bmp180=1;			// device file descriptor
	double temperature;
	FILE *fp=NULL;

	// open i2c device, timeout 10ms
	bmp180 = i2c_open(I2C_BUS, DEV_ADDR,1);

	if (bmp180 < 0) {
		RET("Failed to get I2C device BMP180!\n",0);
	}

	readBmp180Calibration(bmp180);
	
	fp = fopen("temp_data", "w");
	
	if(!fp) {
		RET("Failed to create log.\n",0);
	}

	temperature = bmp180CorrectTemperature(bmp180ReadUT(bmp180));
	
	//printf("%.1lfC\n",temperature);
	fprintf(fp,"%.1f",temperature);
	
	fclose(fp);
	i2c_close(bmp180);	// close device
	return 1;
}
