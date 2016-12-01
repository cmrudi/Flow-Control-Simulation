#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "dcomm.h"
#include <thread>

using namespace std;

#define BUFLEN 9
#define WINDOWS_SIZE 5

void diep(char *s) {
	perror(s);
	exit(1);
}

void recieveRespond();
void slidingWindows();
void makeCheckSum();
void insertData(int frameNum, unsigned char* data);
bool validate();
void copyToWindows(int frameNum);
void sendAgain(int numFrame);

// Windows
Byte WindowsACK[WINDOWS_SIZE];
Byte WindowsData[WINDOWS_SIZE][9];
int sizeWindows = 0;
int counterACK = 0;
clock_t WindowsTimeSend[WINDOWS_SIZE];

bool isFinish = false;
bool isEndFile = false;
int state = 0; //0 == XON, 1 == XOFF
struct sockaddr_in si_other;
int s, i, slen=sizeof(si_other);
Byte buffer[9];
Byte message[5];

int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("please input: ./transmitter <IP Address> <Port> <Text File>\n");
	}	
	else {
		int PORT = atoi(argv[2]);
		char* SRV_IP = argv[1];
		char* FILE_NAME = argv[3];
		
		//char buf[BUFLEN];
	 
		if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
			diep((char*)"socket");

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
			fprintf(stderr, "inet_aton() failed\n");
			exit(1);
		}
		
		//inisialisasi
		for (int i = 0; i < 5; ++i)
		{
			WindowsACK[i] = 0;
			WindowsTimeSend[i] = clock();
		}
		
		ifstream is(FILE_NAME);     // open file
		int count = 0;
		int counterData;
		char c;
		
		thread t1(recieveRespond);
		thread t2(slidingWindows);

		while (!isEndFile) {          // loop getting single characters
			
			while(state) {
				printf("Menunggu XON ...\n");
				sleep(1);
			}

			while(sizeWindows>WINDOWS_SIZE){
				printf("Wait Window FULL\n");
				sleep(1);
			}

			unsigned char data[3];
			counterData = 0;
			while(counterData < 3){
				if(is.get(c)) {
					data[counterData] = c;
				} else {
					isEndFile = true;
					data[counterData] = Endfile;
				}
				
				counterData++;
			}

			insertData(count,data);
			makeCheckSum();
			copyToWindows(count);

			//sprintf(buf, "%c", c);
			if (sendto(s, buffer, BUFLEN, 0, (struct sockaddr*) &si_other, slen)==-1) {
				diep((char*)"sendto()");
			}
			sizeWindows++;
			WindowsTimeSend[count%WINDOWS_SIZE] = clock();
			WindowsACK[count%WINDOWS_SIZE] = NAK;
			
			printf("Mengirim frame ke-%d: '%c %c %c'\n",count,buffer[4],buffer[5],buffer[6]);
			count++;
			sleep(1);
			
		}
		printf("%d",(int)c);
		isFinish = true;

		t1.join();
		t2.join();
		is.close();                // close file
		
		close(s);
	}

 	return 0;
}

void recieveRespond() {
	
	while(!isFinish || (sizeWindows != 0)) {
		if (recvfrom(s, message, 6, MSG_DONTWAIT, (struct sockaddr *) &si_other, (socklen_t*)&slen) != -1) {
			if (message[0] == XON) {
				printf("XON diterima.\n");
				state = 0;
			} else if (message[0] == XOFF) {
				printf("XOFF diterima.\n");
				state = 1;
			} else if(message[0] == ACK) {
				if(validate()) {
					printf("validate berasil\n");
				}
				int numFrame = (message[1]*256) + message[2];
				WindowsACK[numFrame%5] = ACK;
				printf("ACK Recieve ke-%d\n",numFrame);
			} else {
				if(validate()) {
					printf("validate berasil\n");
				}
				int numFrame = (message[1]*256) + message[2];
				sendAgain(numFrame);
				printf("NAK Recieve ke-%d\n",numFrame);
			}
		}

		
	}
}

void slidingWindows(){
	double duration;
	while(!isFinish || (sizeWindows != 0)) {

		for (int i = 0; i < 5; ++i)
		{
			duration = ( clock() - WindowsTimeSend[i] ) / (double) CLOCKS_PER_SEC;
			if((WindowsACK[i]==NAK) && (duration>5)){
				if(!state) {
					sendAgain(i);
				}
				WindowsTimeSend[i] = clock();
			}
		}

		if(WindowsACK[counterACK%WINDOWS_SIZE] == ACK) {
			WindowsACK[counterACK%WINDOWS_SIZE] = 0;
			counterACK++;
			sizeWindows--;
		}
		

	}
}

void sendAgain(int numFrame) {
	Byte DataSendAgain[9];
	WindowsTimeSend[numFrame%WINDOWS_SIZE] = clock();
	for (int i = 0; i < 9; ++i)
	{
		DataSendAgain[i] = WindowsData[numFrame%5][i];
	}
	if (sendto(s, DataSendAgain, BUFLEN, 0, (struct sockaddr*) &si_other, slen)==-1) {
		diep((char*)"sendto()");
	}

	int frameNum = (DataSendAgain[1]*256) + DataSendAgain[2];
	printf("Data send again frame - %d\n",frameNum);
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

void copyToWindows(int frameNum) {
	for (int i = 0; i < 9; ++i)
	{
		WindowsData[frameNum%5][i] = buffer[i];
	}
}

bool validate(){
	int i;
	unsigned char checksum = 0;
	for (i=0;i<3;i++) {
		checksum = checksum * 2;
		if(message[i]%2 == 1){
			checksum = checksum + 1;
		}
	}

	if(message[3] == checksum) {
		return true;
	} else {
		return false;
	}
}