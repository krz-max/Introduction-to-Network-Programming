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
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

#define MAXLINE 2500
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define err_quit(m) \
	{               \
		perror(m);  \
		exit(-1);   \
	}
#define NIPQUAD(s) ((unsigned char *)&s)[0], \
				   ((unsigned char *)&s)[1], \
				   ((unsigned char *)&s)[2], \
				   ((unsigned char *)&s)[3]

struct timeval timeout = {3, 0};
int max_timeout_try = 4;
const char ack[4] = {"ACK"};
int sim_loss = 0;
string folderpath;
int kfile;
int port;

int sockfd = -1;
struct sockaddr_in servaddr, cliaddr;
socklen_t servlen, clilen;
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
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}
void sendmsg(void *sendline, int size)
{
	clilen = sizeof(cliaddr);
	sendto(sockfd, sendline, size, 0, (sockaddr *)&cliaddr, clilen);
	fprintf(stderr, "S: PING %u.%u.%u.%u/%u, init seq = %d\n",
		NIPQUAD(cliaddr.sin_addr), ntohs(cliaddr.sin_port), 0);
}
int getresponse(void *buf)
{
	int n;
	n = recvfrom(sockfd, buf, MAXLINE - 1, 0, (sockaddr *)&cliaddr, &clilen);
	if (n < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			fprintf(stderr, "S: timeout\n");
			return -1;
		}
		else
			err_quit("recvfrom error");
	}
	((char *)buf)[n] = 0;
	fprintf(stdout, "S: cli resp: %s\n", (char *)buf);
	return 0;
}
void dg_echo()
{
	size_t n;
	char buf[MAXLINE];
	for (;;)
	{
		bzero(&buf, sizeof(buf));
		while( getresponse(buf) < 0 );
		if (!strcmp(buf, "FILE_REQ"))
		{
			// let client know serv has received filename
			fprintf(stdout, "S: file request\n");
			sendmsg((void *)ack, sizeof(ack));
			// get file name
			fprintf(stdout, "S: waiting for file name...\n");
			while( getresponse(buf) < 0 );
			sendmsg((void *)ack, sizeof(ack));

			string path = folderpath + "/" + buf;
			fprintf(stdout, "S: storing file in \"%s\"\n", path.c_str());
			ofstream f_out(path.c_str());
			if(!f_out) err_quit("fopen failed");
			map<int, string> output_buf;
			while(1)
			{
				if(getresponse(buf) < 0)
					continue;
				if(!strcmp(buf, "ENDOFFILE")){
					sendmsg((void *)ack, strlen(ack));
					break;
				}
				if(!strncmp(buf, "SEQNUM", 6) && !sim_loss){
					string temp = buf;
					// parse number
					string num_str = temp.substr(6, 3);
					fprintf(stdout, "S: pkt num: %s\n", num_str.c_str());
					string ack_seq = ack + num_str;
					sendmsg((void *)ack_seq.c_str(), ack_seq.length());
					
					// store data
					string data = temp.substr(9);
					int num = strtol(num_str.c_str(), NULL, 10);
					output_buf[num] = data;
					fprintf(stdout, "S: seq num: %d\n", num);
				}
				sim_loss = rand() % 2;
			}
			printf("file %s done\n", path.c_str());
			fprintf(stdout, "S: start writing file...\n");
			for(auto it:output_buf){
				f_out << it.second;
			}
			f_out.close();
		}
		else if (!strcmp(buf, "CONN_REQ"))
		{
			fprintf(stderr, "S: PING %u.%u.%u.%u/%u, init seq = %d\n",
				NIPQUAD(cliaddr.sin_addr), ntohs(cliaddr.sin_port), 0);
			sendmsg((void *)ack, strlen(ack));
		}
	}
}
int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		return -fprintf(stderr, "S: usage: %s ... <port>\n", argv[0]);
	}
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	folderpath = argv[1];
	kfile = strtol(argv[2], NULL, 10);
	port = strtol(argv[3], NULL, 10);
	Start_UDP_Server();

	dg_echo();
	close(sockfd);
}
