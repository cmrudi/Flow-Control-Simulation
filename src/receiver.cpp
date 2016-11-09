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

#define BUFLEN 512	//Max length of buffer
#define MINUPPERBUF 5
#define MAXLOWERBUF 2

using namespace std;



void consumeByte();
Byte q_get();
Byte *rcvchar(int sockfd);

void die(char *s) {
	perror(s);
	exit(1);
}

struct sockaddr_in si_me, si_other;
int s, slen = sizeof(si_other) , recv_len;
Byte buf[BUFLEN];
queue<Byte> dataBuffer;
int dataCounter = 1;
int consumedCounter = 1;
char xChar = XON;
char send_xonxoff[1];
int sourcePort;
Byte currentData;
bool isBinded = false;

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
		currentData = *(rcvchar(s));
		if (currentData==Endfile) {
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

		if(dataBuffer.size() > 0){
			Byte temp = q_get();

			if (temp == Endfile) {
				exit(0);
			} else {
				printf("Mengkonsumsi byte ke-%d: '%c'\n",consumedCounter,temp);
				consumedCounter++;
			}
		}
		
		sleep(3);
	}
}


Byte *rcvchar(int sockfd) {
	if (xChar==XON) {
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1) {
			die((char*)"recvfrom()");
		}

		if (recv_len != -1 && !isBinded) {
			printf("Binding pada %s:%d ...\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			isBinded = true;
		}

		if (recv_len!=0) {
			printf("Menerima byte ke-%d\n" , dataCounter);
			dataBuffer.push(buf[0]);
			dataCounter++;		
		}

		if (dataBuffer.size()> MINUPPERBUF) {
			printf("Buffer > minimum upperlimit. Mengirim XOFF.\n");
			xChar = XOFF;
			send_xonxoff[0] = xChar;
			ssize_t sent_len = sendto(s, send_xonxoff, sizeof(send_xonxoff), 4, (struct sockaddr*) &si_other, slen);
			if(sent_len < 0) {
				perror("sendto() failed)\n");
      		}	
		}
	} else {
		buf[0] = 0;
	}


	return &buf[0];
}

Byte q_get() {

	if (dataBuffer.size() < MAXLOWERBUF && xChar == XOFF) {
		printf("Buffer < maximum lowerlimit. Mengirim XON.\n");
		xChar = XON;
		send_xonxoff[0] = xChar;
		ssize_t sent_len = sendto(s, send_xonxoff, sizeof(send_xonxoff), 4, (struct sockaddr*) &si_other, slen);
		if(sent_len < 0) {
			perror("sendto() failed)\n");
  		}	
	}

	Byte dataChar = dataBuffer.front();
	dataBuffer.pop();

	return dataChar;
}

