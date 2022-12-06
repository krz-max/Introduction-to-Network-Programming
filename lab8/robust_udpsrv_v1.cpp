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

#define INIT 0
#define WAIT_FILE_REQ 1
#define WAIT_NAME 4
#define RECV 2
#define WRIT 3
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

int state = INIT;
struct timeval timeout = {1, 0};
int max_timeout_try = 5;
int b = 100000;
const char ack[4][6] = {{"ACKFQ"}, {"ACKFN"}, {"ACKCQ"}, {"ACKEF"}};
const char ACK[4] = "ACK";
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
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &b, sizeof(b));
}
void sendmsg(void *sendline, int size)
{
	clilen = sizeof(cliaddr);
	sendto(sockfd, sendline, size, 0, (sockaddr *)&cliaddr, clilen);
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
	string temp = (char *)buf;
	temp = temp.substr(0, 9);
	fprintf(stdout, "S: cli resp: %s\n", temp.c_str());
	return 0;
}
void dg_echo()
{
	int rcvwait;
	size_t n;
	char buf[MAXLINE];
	string path;
	ofstream f_out;
	map<int, string> output_buf;
	for (;;)
	{
		bzero(&buf, sizeof(buf));
		if (state == INIT)
		{
			// ignore other message and wait conn_req
			if (getresponse(buf) < 0 || strcmp(buf, "CONN_REQ"))
				continue;
			if(cliaddr.sin_port == 0) continue;
			fprintf(stderr, "S: PING %u.%u.%u.%u/%u, init seq = %d\n",
					NIPQUAD(cliaddr.sin_addr), ntohs(cliaddr.sin_port), 0);
			sendmsg((void *)ack[2], sizeof(ack[2]));
			sendmsg((void *)ack[2], sizeof(ack[2]));
			sendmsg((void *)ack[2], sizeof(ack[2]));
			state = WAIT_FILE_REQ;
			fprintf(stdout, "S: wait file request\n");
		}
		else if (state == WAIT_FILE_REQ)
		{
			if (getresponse(buf) < 0 || strcmp(buf, "FILE_REQ")){
				if(!strncmp(buf, "000", 3)){
					sendmsg((void *)ack[1], sizeof(ack[1]));
					sendmsg((void *)ack[1], sizeof(ack[1]));
					sendmsg((void *)ack[1], sizeof(ack[1]));
					continue;
				}
				sendmsg((void *)ack[2], sizeof(ack[2]));
				sendmsg((void *)ack[2], sizeof(ack[2]));
				sendmsg((void *)ack[2], sizeof(ack[2]));
				continue;
			}
			state = WAIT_NAME;
			sendmsg((void *)ack[0], sizeof(ack[0]));
			sendmsg((void *)ack[0], sizeof(ack[0]));
			sendmsg((void *)ack[0], sizeof(ack[0]));
			fprintf(stdout, "S: waite for file name...\n");
		}
		else if (state == WAIT_NAME)
		{
			if (getresponse(buf) < 0 || !strcmp(buf, "FILE_REQ")){
				sendmsg((void *)ack[0], sizeof(ack[0]));
				sendmsg((void *)ack[0], sizeof(ack[0]));
				sendmsg((void *)ack[0], sizeof(ack[0]));
				continue;
			}
			sendmsg((void *)ack[1], sizeof(ack[1]));
			sendmsg((void *)ack[1], sizeof(ack[1]));
			sendmsg((void *)ack[1], sizeof(ack[1]));
			path = folderpath + "/" + buf;
			fprintf(stdout, "S: storing file in \"%s\"\n", path.c_str());
			f_out.open(path.c_str());
			if (!f_out)
				err_quit("fopen failed");
			state = RECV;
			rcvwait = 8;
		}
		else if (state == RECV)
		{
			if (getresponse(buf) < 0)
				continue;
			if (!strcmp(buf, "ENDOFFILE") || rcvwait == 0)
			{
				sendmsg((void *)ack[3], strlen(ack[3]));
				sendmsg((void *)ack[3], strlen(ack[3]));
				sendmsg((void *)ack[3], strlen(ack[3]));
				printf("file %s done sending, start writng\n", path.c_str());
				state = WRIT;
			}
			if (!strncmp(buf, "SEQNUM", 6))
			{
				string temp = buf;
				string size_str = temp.substr(13, 3);
				rcvwait = 10;
				if(strtol(size_str.c_str(), NULL, 10) != temp.length()-16){
					continue;
				}
				// parse number
				string num_str = temp.substr(6, 3);
				string ack_seq = ACK + num_str;
				sendmsg((void *)ack_seq.c_str(), ack_seq.length());
				sendmsg((void *)ack_seq.c_str(), ack_seq.length());
				sendmsg((void *)ack_seq.c_str(), ack_seq.length());

				// store data
				string data = temp.substr(16);
				int num = strtol(num_str.c_str(), NULL, 10);
				output_buf[num] = data;
				fprintf(stdout, "S: seq num: %d\n", num);
			}
			else{
				sendmsg((void *)ack[1], sizeof(ack[1]));
				sendmsg((void *)ack[1], sizeof(ack[1]));
				sendmsg((void *)ack[1], sizeof(ack[1]));
				rcvwait--;
			}
		}
		else if (state == WRIT)
		{
			for (auto it : output_buf)
				f_out << it.second;
			f_out.close();
			sendmsg((void *)ack[3], strlen(ack[3]));
			sendmsg((void *)ack[3], strlen(ack[3]));
			sendmsg((void *)ack[3], strlen(ack[3]));
			cout << "writing file done, ready for next transfer" << endl;
			output_buf.clear();
			state = WAIT_FILE_REQ;
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
	state = INIT;

	dg_echo();
	close(sockfd);
}
