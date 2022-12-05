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

static int s = -1;
static unsigned seq;
static unsigned count = 0;

const char file_request[9] = "FILE_REQ";
const char conn_request[9] = "CONN_REQ";
const char endoffile[10] = "ENDOFFILE";
const char ack[4][6] = {{"ACKFQ"}, {"ACKFN"}, {"ACKCQ"}, {"ACKEF"}};
const char ACK[4] = "ACK";
int window_size = 10;
struct timeval timeout = {1, 0};
int max_timeout_try = 1;

string folderpath = "";
string addr = "";
int kfile = 0;
int port = 0;

int sockfd = -1;
struct sockaddr_in servaddr;
socklen_t servlen;
void Start_UDP_Client()
{
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_port = htons(port);
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, addr.c_str(), &servaddr.sin_addr);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_quit("socket");
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}
void sendmsg(void *sendline, int size)
{
	servlen = sizeof(servaddr);
	sendto(sockfd, sendline, size, 0, (sockaddr *)&servaddr, servlen);
}
int getresponse(void *buf)
{
	int n;
	n = recvfrom(sockfd, buf, MAXLINE - 1, 0, (sockaddr *)&servaddr, &servlen);
	if (n < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			fprintf(stderr, "C: timeout\n");
			return -1;
		}
		else
			err_quit("recvfrom error");
	}
	((char *)buf)[n] = 0;
	fprintf(stdout, "C: serv resp: %s\n", (char *)buf);
	return 0;
}
int sendandwaitACK(void *sendline, int size, void *waitline, int conn)
{
	int n;
	servlen = sizeof(servaddr);
	char buf[MAXLINE];
	bzero(&buf, sizeof(buf));
	for (int i = 0; i < max_timeout_try; i++)
	{
		if (conn == 1)
			i--;
		sendto(sockfd, sendline, size, 0, (sockaddr *)&servaddr, servlen);
		n = getresponse(buf);
		if (n < 0 || strcmp(buf, (char *)waitline))
			continue;
		return 0;
	}
	return -1;
}

void dg_cli()
{
	size_t n;
	char filename[7] = {"000"};
	for (int i = 0; i < kfile; i++)
	{
		// clear the buffer
		char dump[MAXLINE];
		while (getresponse(dump) == 0);
		fprintf(stdout, "C: initializing file transer..\n");
		sendandwaitACK((void *)file_request, 9, (void *)ack[0], 1);
		fprintf(stdout, "C: file transer connection established!\n");

		fprintf(stdout, "C: trying to send the file name...\n");
		filename[3] = i / 100 + '0';
		filename[4] = ((i / 10) % 10) + '0';
		filename[5] = i % 10 + '0';
		filename[6] = 0;
		string path = folderpath + "/" + filename;
		sendandwaitACK((char *)filename, 6, (void *)ack[1], 1);

		fprintf(stdout, "C: file name received by server: %s\n", path.c_str());

		ifstream f_in(path.c_str());
		if (!f_in)
			err_quit("fopen error");
		long filesize = f_in.tellg();

		int howmanyfragment = 0;
		int fragment_num = 0;
		int seq_num = 0;
		vector<string> send_buffer;
		vector<int> send_idx_buffer;
		fprintf(stdout, "C: buffering file content...\n");
		for (;;)
		{
			string buf;
			if (f_in >> buf)
			{
				int l = buf.length();
				int k = l / 500;
				for (int i = 0; i < k; i++)
				{
					send_buffer.push_back(buf.substr(500 * i, 500));
					send_idx_buffer.push_back(fragment_num);
					fragment_num++;
				}
				if ((k = l % 500) > 0)
				{
					send_buffer.push_back(buf.substr(500 * fragment_num));
					send_idx_buffer.push_back(fragment_num++);
				}
			}
			else
				break;
		}
		howmanyfragment = send_buffer.size();
		fprintf(stdout, "C: start sending file:...\n");
		int ack_num = 0;
		while (ack_num < howmanyfragment)
		{
			string seq;
			char temp[MAXLINE];
			char buf[MAXLINE];
			fragment_num = 0;
			while (fragment_num < send_idx_buffer.size())
			{
				int i = fragment_num;
				for (; i < send_idx_buffer.size() && i < fragment_num+25; i++)
				{
					if (send_idx_buffer[i] < 0){
						fragment_num++;
						continue;
					}
					sprintf(temp, "SEQNUM%3dSIZE%3ld", send_idx_buffer[i], send_buffer[i].length());
					seq = temp;
					seq += send_buffer[i];
					sendmsg((void *)seq.c_str(), seq.length());
					if ((n = getresponse(buf)) == 0)
					{
						string tmp = buf;
						int num = strtol(tmp.substr(3).c_str(), NULL, 10);
						if(send_idx_buffer[num] != -1) ack_num++;
						send_idx_buffer[num] = -1;
					}
				}
				sleep(0.1);
				fragment_num = i;
			}
			// get all the buffered response
			while ((n = getresponse(buf)) == 0)
			{
				string tmp = buf;
				int num = strtol(tmp.substr(3).c_str(), NULL, 10);
				if(send_idx_buffer[num] != -1) ack_num++;
				send_idx_buffer[num] = -1;
			}
			fprintf(stdout, "C: latest acked fragment: %s, total: %d\n", (char *)buf, ack_num);
			cout << "unakced: {";
			for (int i = 0; i < send_idx_buffer.size(); i++)
			{
				if (send_idx_buffer[i] < 0)
					continue;
				cout << i << ", ";
			}
			cout << "}" << endl;
		}
		sendandwaitACK((void *)endoffile, strlen(endoffile), (void *)ack[3], 0);
		sleep(1);
	}
}
int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		return -fprintf(stderr, "C: usage: %s ... <port> <ip>\n", argv[0]);
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
	cout << "Waiting for server response...\n";
	char buf[MAXLINE];
	sendandwaitACK((void *)conn_request, sizeof(conn_request), (void *)ack[2], 1);

	fprintf(stdout, "C: connection established, start sending file...\n");

	dg_cli();

	close(s);
}
