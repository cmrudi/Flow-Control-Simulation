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
#define PORT 8888	//The port on which to listen for incoming data
#define MAXBUF 10

using namespace std;

void die(char *s)
{
	perror(s);
	exit(1);
}



int main(/*int argc, char* args[]*/void)
{
	struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other) , recv_len;
	char buf[BUFLEN];
	bool isBinded= false;
	bool isPrinted= false;
	queue<char> dataBuffer;
	int dataCounter = 0;
	int consumedCounter = 0;

	Byte sent_xonxoff = XON;
	bool send_xon = ​ false​;
	bool send_xoff = ​ false​ ;


	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die((char*)"socket");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		die((char*)"bind");
	}
		
	//keep listening for data
	while(1)
	{
		
		fflush(stdout);
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1)
		{
			die((char*)"recvfrom()");
		}
		if (recv_len != -1 && !isBinded) {
			printf("Binding pada %s:%d ...\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			isBinded = true;
		}
		dataBuffer.push(buf[0]);
		if (dataBuffer.size()>MAXBUF) {

			 
		}

		//print details of the client/peer and the data received
		printf("Data: %s\n" , buf);
		
		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			die((char*)"sendto()");
		}
		sleep(1);
	}

	close(s);
	return 0;
}