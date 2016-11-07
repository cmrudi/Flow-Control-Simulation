#include <stdio.h>	//printf
#include <string.h> //memset
#include <string>
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "dcomm.h"
#include <queue>
#include <pthread.h>

#define BUFLEN 512	//Max length of buffer
#define MINUPPERBUF 5
#define MAXLOWERBUF 2

using namespace std;


pthread_t threadId;
pthread_mutex_t myLock;

void* mainThr(void* tArg);
static Byte *q_get(queue<Byte> Q, Byte *data);
static Byte *rcvchar(int sockfd, queue<Byte> Q);
int isChar(char c);


void die(char *s)
{
	perror(s);
	exit(1);
}

struct sockaddr_in si_me, si_other;
int s, i, slen = sizeof(si_other) , recv_len;
Byte buf[BUFLEN];
queue<Byte> dataBuffer;
int dataCounter = 1;
int consumedCounter = 1;
char xChar = XON;
Byte sent_xonxoff = XON;
bool send_xon = false;
bool send_xoff = false;
char send_xonxoff[1];
int sourcePort;
Byte currentData;
bool isBinded = false;

int main(int argc, char* args[])
{
	if (argc!= 2) {
		die((char*)"please input: ./receiver <PORT>\n");
	}

	sourcePort = atoi(args[1]);
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die((char*)"socket");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(sourcePort);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		
	}
	else {
		//printf("Binding pada %s:%d ...\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
	}
	
	//initialize mutex
	if (pthread_mutex_init(&myLock, NULL) != 0) {
	    die((char*)"mutex init failed\n");
	}
	//create main thread/*
	
	if(pthread_create(&threadId, NULL, mainThr, NULL)) {
		die((char*)"Error creating thread\n");
	}
	
	
	//keep listening for data
	while(1)
	{
		
		fflush(stdout);
		//try to receive some data, this is a blocking call
		currentData = *(rcvchar(s,dataBuffer));
		if (currentData==Endfile) {
			exit(0);
		}

		/*
		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			die((char*)"sendto()");
		}
		*/

		

	}
	
	//join the process thread
	pthread_join(threadId,NULL);
	//destroy mutex
	pthread_mutex_destroy(&myLock);

	close(s);
	return 0;
}

void* mainThr(void* tArg) {
	//pthread_mutex_lock(&myLock); //lock the thread process

	while (1) {
		
		Byte* temp = q_get(dataBuffer,&currentData); 
		if(isChar(currentData) ){
			if (currentData == Endfile) {
				exit(0);
			}
			else {
				printf("Mengkonsumsi byte ke-%d: '%c'\n",consumedCounter,currentData);
				consumedCounter++;
			}
			sleep(2);
		}
	}
	//pthread_mutex_unlock(&myLock); //release the thread lock

	return NULL;
}


static Byte *rcvchar(int sockfd, queue<Byte> Q) {
	if (xChar==XON) {
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1)
		{
			die((char*)"recvfrom()");
		}
		if (recv_len != 0 && !isBinded) {
			printf("Binding pada %s:%d ...\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			isBinded = true;
		}

		if (recv_len!=0) {
			printf("Menerima byte ke-%d\n" , dataCounter);
			Q.push(buf[0]);
			dataCounter++;		
		}

		if (Q.size()> MINUPPERBUF) {
			printf("Buffer > minimum upperlimit.\nMengirim XOFF.\n");
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

static Byte *q_get(queue<Byte> Q, Byte *data) {
	if (Q.size()==0) {
		return NULL;
	}
	else {
		while (Q.size()>0) {
			Byte temp;
			temp = Q.front();
			Q.pop();
			data[consumedCounter] = temp;
			//consumedCounter++;
		}
		
	}
	if (Q.size()< MAXLOWERBUF) {
		printf("Buffer < maximum lowerlimit. \nMengirim XON.\n");
		xChar = XON;
		send_xonxoff[0] = xChar;
		ssize_t sent_len = sendto(s, send_xonxoff, sizeof(send_xonxoff), 4, (struct sockaddr*) &si_other, slen);
		if(sent_len < 0) {
			perror("sendto() failed)\n");
  		}	
	}

	return data;
}

int isChar(char c){
  return (c >= 32 || c == CR || c == Endfile || c == LF);
}