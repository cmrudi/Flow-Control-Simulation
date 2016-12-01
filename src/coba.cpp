#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcomm.h"



void insertData(int frameNum, unsigned char* data);
void makeCheckSum();
bool validateData(unsigned char* buffer);

unsigned char buffer[9];

int main() {
	int frameNum = 2;
	unsigned char data[3];
	data[1] = 'a';
	data[0] = 's';
	data[2] = 'y';
	insertData(frameNum,data);
	makeCheckSum();

	int i;
	for (i=0;i<9;i++) {
		printf("%d \n",buffer[i]);
	}

	return 0;
}

void makeCheckSum(){
	int i;
	unsigned char checksum = 0;
	for (i=0;i<8;i++) {
		checksum = checksum * 2;
		if(buffer[i]%2 == 1){
			checksum = checksum + 1;
		}
	}

	buffer[8] = checksum;
}


void insertData(int frameNum, unsigned char* data) {

	
	buffer[0] = (unsigned char) SOH;
	buffer[3] = (unsigned char) STX;
	buffer[7] = (unsigned char) ETX;

	//frameNum
	buffer[1] = (unsigned char) frameNum / 256;
	buffer[2] = (unsigned char) frameNum % 256;

	//data
	buffer[4] = data[0];
	buffer[5] = data[1];
	buffer[6] = data[2];

}
