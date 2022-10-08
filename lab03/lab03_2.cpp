#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

using namespace std;

int Socket(int family, int type, int protocol){ // Create a socket
    int n;
    if( (n = socket(family, type, protocol)) < 0)
        cerr << "socket error" << endl;
    return n;
}
int Connect(int sockfd, struct sockaddr *serv, socklen_t addrlen){ // for TCP, initiate three-way handshaking
    int n;
    if( (n = connect(sockfd, serv, addrlen)) < 0)
        cerr << "connect error" << endl;
    return n;
}
int Bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen){
    int n;
    if( (n = bind(sockfd, myaddr, addrlen)) < 0)
        cerr << "bind error" << endl;
    return n;
}
int Listen(int sockfd, int backlog){
    int n;
    if( (n = listen(sockfd, backlog)) < 0)
        cerr << "listen error" << endl;
    return n;
}
string HostToIp(const string& host) {
    hostent* hostname = gethostbyname(host.c_str());
    if(hostname)
        return string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    return {};
}
int readline(int fd, string& recvline){
    char buf;
    int n = 0;
    while(1){
        if(read(fd, &buf, 1) > 0){
            if(buf != '\n' && buf != '\0'){
                recvline += buf;
                n++;
            }
            else{
                break;
            }
        }
        else{
            break;
        }
    }
    return n;        
}

static struct timeval _t0;
static unsigned long long bytesent = 0;

double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&_t0);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

const char garbage[101] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";

int main(int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in servaddr;
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10003);
    if (inet_pton(AF_INET, HostToIp("inp111.zoolab.org").c_str(), &servaddr.sin_addr) <= 0){
        cerr << "inet_pton error for " << "inp111.zoolab.org" << endl;
        return -1;
    }
    Connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));

	signal(SIGINT,  handler);
	signal(SIGTERM, handler);

	gettimeofday(&_t0, NULL);
	while(1) {
		// sleep for 1ns
		// timespec {sec, nanosecond}
		struct timespec t = { 0, 1 };
		if(!strcmp(argv[1], "1")){
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 45, 0);
		}
		else if(!strcmp(argv[1], "1.5")){
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 60, 0);
		}
		else if(!strcmp(argv[1], "2")){
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 70, 0);
		}
		else {
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 100, 0);
			bytesent += send(sockfd, garbage, 20, 0);
		}
		nanosleep(&t, NULL);
	}

	return 0;
}
