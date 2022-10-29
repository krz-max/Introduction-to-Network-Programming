#ifndef STRING_HEADER
#define STRING_HEADER

#include <cstring>
#include <string>
#endif

#ifndef STD_HEADER
#define STD_HEADER
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> // strtol(string, endpointer, base)
#include <sys/wait.h>
#include <sys/time.h>
#endif

#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define SA struct sockaddr

#define INFTIM -1

#define BACKLOGSIZE 1024

void err_sys(const char* x)
{
    perror(x);
    exit(1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int		n;
	if ( (n = accept(fd, sa, salenptr)) < 0) {
		err_sys("accept error");
	}
	return(n);
}
// Create a socket
int Socket(int family, int type, int protocol){
    int n;
    if( (n = socket(family, type, protocol)) < 0)
		fprintf(stderr, "socket error\n");
    return n;
}
// for TCP, initiate 3-way handshaking
int Connect(int sockfd, struct sockaddr *serv, socklen_t addrlen){
    int n;
    if( (n = connect(sockfd, serv, addrlen)) < 0)
		fprintf(stderr, "connect error\n");
    return n;
}
// for Server, bind the server to the IP address and port
/*
SAddr.sin_family      = AF_INET
SAddr.sin_addr.s_addr = htonl(INADDR_ANY);
SAddr.sin_port        = htons(SERV_PORT)
*/
int Bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen){
    int n;
    if( (n = bind(sockfd, myaddr, addrlen)) < 0)
		fprintf(stderr, "bind error\n");
    return n;
}
// for Server, Listen to the sockfd that was bind to a specific port
// Called after bind
int Listen(int sockfd, int backlog){
    int n;
    if( (n = listen(sockfd, backlog)) < 0)
		fprintf(stderr, "listen error\n");
    return n;
}
const char* Inet_ntop(int family, const void *addrptr, char *str, size_t len){
	const char* result;
    if ( (result = inet_ntop(family, addrptr, str, len)) == NULL ) {
		fprintf(stderr, "inet_ntop error for %ld\n", (size_t)addrptr);
        return NULL;
    }
	return result;
}

void Close(int fd)
{
	if (close(fd) == -1)
		err_sys("close error");
}

void Dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
		err_sys("dup2 error");
}

int Start_TCP_Server(int* sockfd, uint16_t Port, uint64_t INADDR=INADDR_ANY)
{
	struct   sockaddr_in  SAddr;
	bzero(&SAddr, sizeof(SAddr));
	*sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	SAddr.sin_family 	  = AF_INET;
	SAddr.sin_port 		  = htons(Port);
	SAddr.sin_addr.s_addr = htonl(INADDR);
	Bind(*sockfd, (struct sockaddr*)&SAddr, sizeof(SAddr));
	Listen(*sockfd, BACKLOGSIZE);
	return 0;
}
int Start_TCP_Client(int* sockfd, uint16_t Port, const char* IP)
{
	struct sockaddr_in  CAddr;
	bzero(&CAddr, sizeof(CAddr));
	*sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	CAddr.sin_family      = AF_INET;
	CAddr.sin_port        = htons(Port);
	// vvv 32bit network byte IPv4 address vvv
	CAddr.sin_addr.s_addr = inet_addr(IP);
	Connect(*sockfd, (sockaddr*)&CAddr, sizeof(CAddr));
	return 0;
}