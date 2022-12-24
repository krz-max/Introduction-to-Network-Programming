#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>

#define MAXLINE 1024

#define err_quit(m) \
	{               \
		perror(m);  \
		exit(-1);   \
	}
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define NIPQUAD(s) ((unsigned char *)&s)[0], \
				   ((unsigned char *)&s)[1], \
				   ((unsigned char *)&s)[2], \
				   ((unsigned char *)&s)[3]

using namespace std;

int sockfd = -1;
struct sockaddr_in servaddr;
socklen_t servlen;
void Start_UDP_Client()
{
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_port = htons(53);
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "8.8.8.8",  &servaddr.sin_addr);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_quit("socket");
}

void dg_cli(){
	char sendline[MAXLINE];
	while(fgets(sendline, MAXLINE, stdin) != 0){
		servlen = sizeof(servaddr);
		printf("Send: %s", sendline);
		sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, servlen);
		char recvline[MAXLINE];
		size_t sz;
		struct sockaddr_in cliaddr;
		socklen_t clilen;

		sz = recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *)&servaddr, &servlen);
		printf("received: %s\n", recvline);
	}
}
int main(int argc, char *argv[])
{
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	Start_UDP_Client();
	dg_cli();
	return 0;
}
