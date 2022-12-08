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
#define MAXLINE 2048
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

struct timeval timeout = {1, 0};
const char ack[4][6] = {{"ACKFQ"}, {"ACKFN"}, {"ACKCQ"}, {"ACKEF"}};
const char ACK[4] = "ACK";
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
}
int getresponse(void *buf)
{
	int n;
	bzero(buf, sizeof(buf));
	n = recvfrom(sockfd, buf, MAXLINE - 1, 0, (sockaddr *)&cliaddr, &clilen);
	if (n < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			// fprintf(stderr, "S: timeout\n");
			return -1;
		}
		else
			err_quit("recvfrom error");
	}
	((char *)buf)[n] = 0;
	// string temp = (char *)buf;
	// temp = temp.substr(0, 30);
	// fprintf(stdout, "S: cli resp: %s\n", temp.c_str());
	return 0;
}
void dg_echo()
{
	int rcvwait;
	size_t n;
	string path;
	ofstream f_out;
	map<int, string> output_buf[1000];
	vector<int> written(1000, 0);
	for (;;)
	{
		char buf[MAXLINE];
		if (getresponse(buf) == 0)
		{
			string recvline = buf;
			if (!strncmp(buf, "ENDOFFILE", 9))
			{
				string fname = recvline.substr(9, 6);
				int fID = strtol(fname.substr(3).c_str(), NULL, 10);
				if (output_buf[fID].empty())
				{
					char sendline[MAXLINE];
					sprintf(sendline, "%s%s", ack[3], fname.c_str());
					sendmsg((void *)sendline, 11);
					sendmsg((void *)sendline, 11);
					sendmsg((void *)sendline, 11);
					sendmsg((void *)sendline, 11);
					sendmsg((void *)sendline, 11);
					continue;
				}
				f_out.open((folderpath + "/" + fname).c_str());
				cout << "write file: " << fname << ", ID: " << fID << endl;
				for (auto it : output_buf[fID])
				{
					// cout << "write seq_num: " << it.first << endl;
					f_out << it.second;
				}
				f_out.close();
				written[fID] = 1;
				char sendline[MAXLINE];
				sprintf(sendline, "%s%s", ack[3], fname.c_str());
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				output_buf[fID].clear();
				continue;
			}
			string size = recvline.substr(27, 3);
			int offset = 0;
			if (size[0] == ' ')
				offset++;
			if (size[1] == ' ')
				offset++;
			size = size.substr(offset);
			int len = strtol(size.c_str(), NULL, 10);
			string data = recvline.substr(30);
			// fprintf(stdout, "len: %d, true len: %ld", len, data.length());
			if (len != data.length())
				continue;

			string fname = recvline.substr(8, 6);
			int fID = strtol(fname.substr(3).c_str(), NULL, 10);
			if(written[fID] == 1){
				char sendline[MAXLINE];
				sprintf(sendline, "%s%s", ack[3], fname.c_str());
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				sendmsg((void *)sendline, 11);
				continue;
			}
			string sqnum = recvline.substr(20, 3);
			int seq_num = strtol(sqnum.c_str(), NULL, 10);
			string ack_seq = ACK + sqnum;
			sendmsg((void *)ack_seq.c_str(), ack_seq.length());
			sendmsg((void *)ack_seq.c_str(), ack_seq.length());
			sendmsg((void *)ack_seq.c_str(), ack_seq.length());
			sendmsg((void *)ack_seq.c_str(), ack_seq.length());
			sendmsg((void *)ack_seq.c_str(), ack_seq.length());

			output_buf[fID][seq_num] = data;
			// fprintf(stdout, "FILE: %s, seq num: %s, now size: %ld\n", fname.c_str(), sqnum.c_str(), output_buf[fID].size());
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
