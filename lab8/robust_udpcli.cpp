/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 * /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>
 */
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

#include <string>
#include <fstream>
#include <iostream>

#define MAXLINE 10000

#define err_quit(m) \
	{               \
		perror(m);  \
		exit(-1);   \
	}

#define NIPQUAD(s) ((unsigned char *)&s)[0], \
				   ((unsigned char *)&s)[1], \
				   ((unsigned char *)&s)[2], \
				   ((unsigned char *)&s)[3]

struct packet{
	unsigned seq;
	char data[2000];
};

using namespace std;

static int s = -1;
static unsigned seq;
static unsigned count = 0;

int window_size = 10;
int timeout = 4;

string folderpath = "";
string addr = "";
int kfile = 0;
int port = 0;

int sockfd = -1;
struct sockaddr_in servaddr;
void Start_UDP_Client()
{
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_port = htons(port);
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, addr.c_str(), &servaddr.sin_addr);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_quit("socket");
	struct timeval tv = {1, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void dg_cli(sockaddr *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];
	char filename[6] = {"000"};
	for (int i = 0; i < kfile; i++)
	{
		filename[3] = i / 100 + '0';
		filename[4] = ((i / 10) % 10) + '0';
		filename[5] = i % 10 + '0';
		string path = folderpath + "/" + filename;
		sendandwaitACK("FILE_REQ", 8);
		sendandwaitACK((char*)path.c_str(), path.length());
		while (fgets(sendline, MAXLINE, fopen(path.c_str(), "rb")) != nullptr)
		{
			
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
		sendandwaitACK("ENDF", 4);
		printf("file %d done\n");
	}
}
void sendandwaitACK(char *sendline, int size)
{
	int connected = 0;
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
		connected = 1;
	} while (!connected);
}
void waitACK(){
	int connected = 0;
	do
	{
		int n;
		socklen_t len = sizeof(servaddr);
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
		connected = 1;
	} while (!connected);
}
int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		return -fprintf(stderr, "usage: %s ... <port> <ip>\n", argv[0]);
	}
	srand(time(0) ^ getpid());

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	folderpath = argv[1];
	kfile = strtol(argv[2], NULL, 10);
	port = strtol(argv[3], NULL, 10);
	addr = argv[4];
	Start_UDP_Client();

	cout << "Connecting to UDP server..." << addr.c_str() << ":" << port << endl;
	sendandwaitACK("CONN_REQ", 8);
	sendandwaitACK("ACK", 3);

	dg_cli((sockaddr *)&servaddr, sizeof(servaddr));

	close(s);
}
