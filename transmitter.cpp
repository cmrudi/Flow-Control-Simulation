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

#define BUFLEN 512
#define NPACK 10
//#define PORT 8888

void diep(char *s) {
	perror(s);
	exit(1);
}

void recieveRespond();

bool isFinish = false;
int state = 0; //0 == XON, 1 == XOFF
struct sockaddr_in si_other;
int s, i, slen=sizeof(si_other);

int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("please input: ./transmitter <IP Address> <Port> <Text File>\n");
	}	
	else {
		int PORT = atoi(argv[2]);
		char* SRV_IP = argv[1];
		char* FILE_NAME = argv[3];
		
		
		
		
		char buf[BUFLEN];
	 
		if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
			diep((char*)"socket");

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
			fprintf(stderr, "inet_aton() failed\n");
			exit(1);
		}
		
		
		
		ifstream is(FILE_NAME);     // open file
		int count = 1;
		char c;
		int recvlen;
		
		thread t1(recieveRespond);

		while (is.get(c)) {          // loop getting single characters
			
			while(state) {
				printf("Menunggu XON ...\n");
				sleep(1);
			}

			sprintf(buf, "%c", c);
			if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*) &si_other, slen)==-1) {
				diep((char*)"sendto()");
			}

			printf("Mengirim byte ke-%d: '%s'\n",count,buf);
			count++;
			sleep(1);
			
		}
		printf("%d",(int)c);
		isFinish = true;

		t1.join();
		is.close();                // close file
		
		close(s);
	}

 	return 0;
}

void recieveRespond() {
	char *message;
	while(!isFinish) {
		if (recvfrom(s, message, 2, MSG_DONTWAIT, (struct sockaddr *) &si_other, (socklen_t*)&slen) == 1) {
			if (message[0] == XON) {
				printf("XON diterima.\n");
				state = 0;
			} else if (message[0] == XOFF) {
				printf("XOFF diterima.\n");
				state = 1;
			}
		}

		
	}
}

