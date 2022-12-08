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

const char endoffile[10] = "ENDOFFILE";
const char ack[4][6] = {{"ACKFQ"}, {"ACKFN"}, {"ACKCQ"}, {"ACKEF"}};
const char ACK[4] = "ACK";
struct timeval timeout = {0, 200000};

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
			// fprintf(stderr, "C: timeout\n");
			return -1;
		}
		else
			err_quit("recvfrom error");
	}
	((char *)buf)[n] = 0;
	// fprintf(stdout, "C: serv resp: %s\n", (char *)buf);
	return 0;
}
void dg_cli(int start, int k_file)
{
	size_t n;
	char filename[7] = {"000"};
	vector<int> fin_file(k_file, 0);
	for (int i = 0; i < k_file; i++)
	{
		// clear the buffer
		char dump[MAXLINE];
		while (getresponse(dump) == 0)
		{
			if (!strncmp(dump, "ACKEF", 5))
			{
				string fname = dump;
				fname = fname.substr(5);
				int fID = strtol(fname.substr(3).c_str(), NULL, 10);
				fin_file[fID % 60] = 1;
				continue;
			}
		}
		filename[3] = (start + i) / 100 + '0';
		filename[4] = (((start + i) / 10) % 10) + '0';
		filename[5] = (start + i) % 10 + '0';
		filename[6] = 0;
		string path = folderpath + "/" + filename;
		ifstream f_in(path.c_str());
		if (!f_in)
			continue;
		long filesize = f_in.tellg();

		int howmanyfragment = 0;
		int fragment_num = 0;
		int seq_num = 0;
		vector<string> send_buffer;
		vector<int> send_idx_buffer;
		for (;;)
		{
			string buf;
			if (f_in >> buf)
			{
				int l = buf.length();
				int k = l / 900;
				for (int i = 0; i < k; i++)
				{
					send_buffer.push_back(buf.substr(900 * i, 900));
					send_idx_buffer.push_back(fragment_num);
					fragment_num++;
				}
				if ((k = l % 900) > 0)
				{
					send_buffer.push_back(buf.substr(900 * fragment_num));
					send_idx_buffer.push_back(fragment_num++);
				}
			}
			else
				break;
		}
		howmanyfragment = send_buffer.size();
		int ack_num = 0;
		while (ack_num < howmanyfragment)
		{
			string seq;
			fragment_num = 0;
			for (int i = 0; i < send_idx_buffer.size(); i++)
			{
				if (send_idx_buffer[i] < 0)
					continue;
				char sendline[MAXLINE], recvline[MAXLINE];
				sprintf(sendline, "FILENAME%sSEQNUM%3dSIZE%3ld%s", filename, i, send_buffer[i].length(), send_buffer[i].c_str());
				sendmsg((void *)sendline, 30 + send_buffer[i].length());
				sleep(0.01);
				if (getresponse(recvline) == 0)
				{
					if (!strncmp(recvline, "ACKEF", 5))
					{
						string fname = recvline;
						fname = fname.substr(5);
						int fID = strtol(fname.substr(3).c_str(), NULL, 10);
						fin_file[fID % 60] = 1;
						continue;
					}
					string resp = recvline;
					int num = strtol(resp.substr(3).c_str(), NULL, 10);
					if (send_idx_buffer[num] >= 0)
						ack_num++;
					send_idx_buffer[num] = -1;
				}
			}
			// get all the buffered response
			char recvline[MAXLINE];
			while (getresponse(recvline) == 0)
			{
				if (!strncmp(recvline, "ACKEF", 5))
				{
					string fname = recvline;
					fname = fname.substr(5);
					int fID = strtol(fname.substr(3).c_str(), NULL, 10);
					fin_file[fID % 60] = 1;
					continue;
				}
				string resp = recvline;
				int num = strtol(resp.substr(3).c_str(), NULL, 10);
				if (send_idx_buffer[num] != -1)
					ack_num++;
				send_idx_buffer[num] = -1;
			}
		}
		for (int j = 0; j < k_file; j++)
		{
			filename[3] = (start + i) / 100 + '0';
			filename[4] = (((start + i) / 10) % 10) + '0';
			filename[5] = (start + i) % 10 + '0';
			filename[6] = 0;
			char end_req[MAXLINE];
			sprintf(end_req, "%s%s", endoffile, filename);
			if (!fin_file[j])
			{
				sendmsg((void *)end_req, 15);
			}
		}
		// cout << "ENDOFFILE" << filename << endl;
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
	int start = 0;
	int k_file;
	for (int i = 0; i < 15; i++)
	{
		k_file = (i < 10) ? 67 : 66;
		pid_t p = fork();
		if (p == 0)
		{
			Start_UDP_Client();
			dg_cli(start, k_file);
		}
		start += k_file;
		close(sockfd);
		continue;
	}
	for (int i = 0; i < 15; i++)
		wait(nullptr);
	return 0;
}
