#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

int main()
{
	int sock,cli;
	struct sockaddr_in server, client;
	unsigned int len;
	char mesg[] = "Hello";	
	int sent;
	int data_len;
	char data[1024];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket: ");
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(4949);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 8);
	
	len = sizeof(struct sockaddr_in);

	if ((bind(sock, (struct sockaddr *)&server, len)) == -1)
	{
		perror("bind");
		exit(-1);
	}

	if ((listen(sock, 5)) == -1)
	{
		perror("listen");
		exit(-1);
	} 

	while (1)
	{
		if((cli = accept(sock, (struct sockaddr *)&client, &len)) ==-1)
		{
			perror("accept");
			exit(-1);
		}
			sent = send(cli, mesg, strlen(mesg), 0);
			
		data_len = 1;
		while(data_len) {
			data_len = recv(cli, data, 1024,0);
			if (data_len) {
				send(cli, data, data_len-1,0);
				data[data_len] = '\0';
				printf(data);
			}
				printf(data_len);
			}
			if (!strcmp(data,mesg)) {
				printf("My name is rudi \n");
			}
			
		}
		

		printf("send %d bytes to client : %s\n", sent, inet_ntoa(client.sin_addr));
		


	}
	close(cli);
	return 0;
}

