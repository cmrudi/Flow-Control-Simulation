#include <stdio.h>	//printf
#include <string.h> //memset
#include <string>
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "dcomm.h"
#include <queue>
#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#define BUFLEN 9	//Max length of buffer
#define MINUPPERBUF 10
#define MAXLOWERBUF 2

using namespace std;



void consumeByte();
vector<Byte> q_get();
void rcvchar(int sockfd);
bool validate();
void sendACK(int numFrame);
void sendNAK(int numFrame);
Byte makeCheckSum(Byte* ACKData);
bool isRecieved(int numFrame);

void die(char *s) {
	perror(s);
	exit(1);
}

struct sockaddr_in si_me, si_other;
int s, slen = sizeof(si_other) , recv_len;
Byte buf[BUFLEN];
//queue<Byte> map_dataBuffer;

map<int, vector<Byte> > map_dataBuffer;
vector<Byte> currentData;

vector<int> frameRecieved;
int consumedCounter = 1;
int consumedFrame = 0;
char xChar = XON;
char send_xonxoff[1];
int sourcePort;

bool isBinded = false;
bool isEndFile = false;

int main(int argc, char* args[]) {

	

	if (argc!= 2) {
		die((char*)"please input: ./receiver <PORT>\n");
	}

	sourcePort = atoi(args[1]);
        
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die((char*)"socket");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(sourcePort);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
		die((char*)"bind");	
	} else {
		isBinded = true;
	}

	thread t1(consumeByte);
	
	//keep listening for data
	while(1)
	{

		//try to receive some data, this is a blocking call
		rcvchar(s);
		if (isEndFile) {
			exit(0);
		}

		sleep(1);

	}

	isBinded = false;
	
	t1.join();

	close(s);
	return 0;
}

void consumeByte() {

	while (isBinded) {

		if(map_dataBuffer.size() > 0){
			vector<Byte> temp = q_get();
			for (int i=0;i<3;i++) {
				if (temp[i] == Endfile) {
					isEndFile = true;
					exit(0);
				} else {
					printf("Mengkonsumsi byte ke-%d\n",consumedCounter);
					consumedCounter++;
				}
			}
			sleep(1);
		}
		
	}
}


void rcvchar(int sockfd) {
	if (xChar==XON) {
		if ((recv_len = recvfrom(s, buf, BUFLEN, 4, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1) {
			die((char*)"recvfrom()");
		}

		if (recv_len != -1 && !isBinded) {
			printf("Binding pada %s:%d ...\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			isBinded = true;
		}

		if (recv_len!=0) {
			if(validate()) {
;				int numFrame = (buf[1]*256) + buf[2];
				if(!isRecieved(numFrame)){
					vector<Byte> currentData;
					currentData.push_back(buf[4]);
					currentData.push_back(buf[5]);
					currentData.push_back(buf[6]);
					map_dataBuffer.insert(make_pair(numFrame,currentData));


					frameRecieved.push_back(numFrame);
					sort(frameRecieved.begin(),frameRecieved.end());	
				}
				sendACK(numFrame);
			} else {
				int numFrame = (buf[1]*256) + buf[2];
				sendNAK(numFrame);
			}
					
		}

		if (map_dataBuffer.size()> MINUPPERBUF) {
			printf("Buffer > minimum upperlimit. Mengirim XOFF.\n");
			xChar = XOFF;
			send_xonxoff[0] = xChar;
			ssize_t sent_len = sendto(s, send_xonxoff, sizeof(send_xonxoff), 4, (struct sockaddr*) &si_other, slen);
			if(sent_len < 0) {
				perror("sendto() failed)\n");
      		}	
		}
	}
}

vector<Byte> q_get() {

	if (map_dataBuffer.size() < MAXLOWERBUF && xChar == XOFF) {
		printf("Buffer < maximum lowerlimit. Mengirim XON.\n");
		xChar = XON;
		send_xonxoff[0] = xChar;
		ssize_t sent_len = sendto(s, send_xonxoff, sizeof(send_xonxoff), 4, (struct sockaddr*) &si_other, slen);
		if(sent_len < 0) {
			perror("sendto() failed)\n");
  		}	
	}
	bool isMapped = false;
	map<int,vector<Byte> >::iterator it;
	vector<Byte> data;
	do {
		it = map_dataBuffer.find(consumedFrame);
		if (it!=map_dataBuffer.end()) {
			data = map_dataBuffer[consumedFrame];
			map_dataBuffer.erase(consumedFrame);
			consumedFrame++;
			isMapped = true;
		}
		else {
			sendNAK(consumedFrame);
			sleep(1);
		}
	} while (!isMapped);


	return data;
}

bool validate(){
	int i;
	unsigned char checksum = 0;
	for (i=0;i<8;i++) {
		checksum = checksum * 2;
		if(buf[i]%2 == 1){
			checksum = checksum + 1;
		}
	}

	int numFrame = (buf[1]*256) + buf[2];
	printf("Menerima frame ke-%d\n" , numFrame);


	if(buf[8] == checksum) {
				return true;
	} else {
		return false;
	}
}

void sendACK(int numFrame){
	Byte ACKData[4];
	ACKData[0] = ACK;
	ACKData[1] = (unsigned char) numFrame / 256;
	ACKData[2] = (unsigned char) numFrame % 256;
	ACKData[3] = makeCheckSum(ACKData);
	printf("aaa\n");
	ssize_t sent_len = sendto(s, ACKData, sizeof(ACKData), 4, (struct sockaddr*) &si_other, slen);
	if(sent_len < 0) {
		perror("sendto() failed)\n");
  	}
}

void sendNAK(int numFrame){
	Byte ACKData[4];
	ACKData[0] = NAK;
	ACKData[1] = (unsigned char) numFrame / 256;
	ACKData[2] = (unsigned char) numFrame % 256;
	ACKData[3] = makeCheckSum(ACKData);
	printf("aaa\n");
	
	ssize_t sent_len = sendto(s, ACKData, sizeof(ACKData), 0, (struct sockaddr*) &si_other, slen);
	if(sent_len < 0) {
		perror("sendto() failed)\n");
  	}
}

Byte makeCheckSum(Byte* ACKData){
	int i;
	unsigned char checksum = 0;
	for (i=0;i<3;i++) {
		checksum = checksum * 2;
		if(ACKData[i]%2 == 1){
			checksum = checksum + 1;
		}
	}

	return checksum;
}

bool isRecieved(int numFrame){
	bool found = false;
	for (std::vector<int>::iterator it = frameRecieved.begin() ; it != frameRecieved.end(); ++it) {
		if(numFrame == (*it))
			found = true;
	}

	return found;
    	
}

