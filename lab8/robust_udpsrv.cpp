/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 * /server <path-to-store-files> <total-number-of-files> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

#define MAXLINE 2000
#define err_quit(m) \
	{               \
		perror(m);  \
		exit(-1);   \
	}

string folderpath;
int kfile;
int port;

int sockfd = -1;
struct sockaddr_in servaddr;
void Start_UDP_Server()
{
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_quit("socket");
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		err_quit("bind");
}
void dg_echo(sockaddr *pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char msg[MAXLINE];
	for (;;)
	{
		len = clilen;
		bzero(&msg, sizeof(msg));
		n = recvfrom(sockfd, msg, MAXLINE, 0, pcliaddr, &len);
		msg[n] = 0;
		if (!strcmp(msg, "FILE_REQ"))
		{
			// let client know serv has received filename
			sendto(sockfd, "ACK", 3, 0, (sockaddr *)&servaddr, len);
			printf("file req\n");
			// get file name
			n = recvfrom(sockfd, msg, MAXLINE, 0, pcliaddr, &len);
			msg[n] = 0;
			sendto(sockfd, "ACK", 3, 0, (sockaddr *)&servaddr, len);
			string path = folderpath + "/" + msg;
			FILE *fp = fopen(path.c_str(), "wb");
			
			while(n = recvfrom(sockfd, msg, MAXLINE, 0, pcliaddr, &len))
			{
				if(!strcmp(msg, "ENDF")) break;
				waitACK("ACK", 3);
				sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
				// should return ACK
				n = recvfrom(sockfd, recvline, MAXLINE, 0, pservaddr, &servlen);
				if (n < 0)
				{
					if (errno == EWOULDBLOCK)
					{
						fprintf(stderr, "timeout\n");
						continue;
					}
					else
						err_quit("recvfrom error");
				}
				sendto(sockfd, "ACK", 3, 0, pservaddr, servlen);
				recvline[n] = 0;
				fputs(recvline, stdout);
			}
			printf("file %d done\n");
		}
		else if (!strcmp(msg, "CONN_REQ"))
		{
			waitACK("ACK", 3);
		}
	}
}
void waitACK(char *sendline, int size)
{
	int attempt = 5;
	do
	{
		int n;
		socklen_t len = sizeof(servaddr);
		sendto(sockfd, sendline, size, 0, (sockaddr *)&servaddr, len);
		char buf[5];
		n = recvfrom(sockfd, buf, 5, 0, nullptr, nullptr);
		if (n < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				fprintf(stderr, "timeout\n");
				continue;
			}
			else
				err_quit("recvfrom error");
		}
		// ACK received
		attempt = 1;
	} while (attempt);
}
int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		return -fprintf(stderr, "usage: %s ... <port>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	folderpath = argv[1];
	kfile = strtol(argv[2], NULL, 10);
	port = strtol(argv[3], NULL, 10);
	Start_UDP_Server();

	struct sockaddr_in cliaddr;
	dg_echo((sockaddr *)&cliaddr, sizeof(cliaddr));
	close(sockfd);
}
// cout << "here" << endl;
