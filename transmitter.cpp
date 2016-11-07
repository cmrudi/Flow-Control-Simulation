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

using namespace std;

#define BUFLEN 512
#define NPACK 10
//#define PORT 8888

void diep(char *s) {
	perror(s);
	exit(1);
}

//#define SRV_IP "127.0.0.1"

int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("please input: ./transmitter <IP Address> <Port> <Text File>\n");
	}	
	else {
		int PORT = atoi(argv[2]);
		char* SRV_IP = argv[1];
		char* FILE_NAME = argv[3];
		
		
		struct sockaddr_in si_other;
		int s, i, slen=sizeof(si_other);
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
		while (is.get(c)) {          // loop getting single characters
			
			sprintf(buf, "%c", c);
			if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*) &si_other, slen)==-1)
				diep((char*)"sendto()");
			printf("Mengirim byte ke-%d: '%s'\n",count,buf);
			count++;
		
		}
		is.close();                // close file
		
		
		
		/*
		for (i=0; i<NPACK; i++) {
			printf("Sending packet %d\n", i);
			sprintf(buf, "This is packet %d\n", i);
			if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*) &si_other, slen)==-1)
				diep((char*)"sendto()");

		}
		*/
		close(s);
	}
 	return 0;
 }

