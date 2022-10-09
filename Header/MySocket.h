#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h> // strtol(string, endpointer, base)

#define BACKLOGSIZE 128

// Create a socket
int Socket(int family, int type, int protocol){
    int n;
    if( (n = socket(family, type, protocol)) < 0)
        std::cerr << "socket error" << std::endl;
    return n;
}
// for TCP, initiate 3-way handshaking
int Connect(int sockfd, struct sockaddr *serv, socklen_t addrlen){
    int n;
    if( (n = connect(sockfd, serv, addrlen)) < 0)
        std::cerr << "connect error" << std::endl;
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
        std::cerr << "bind error" << std::endl;
    return n;
}
// for Server, Listen to the sockfd that was bind to a specific port
// Called after bind
int Listen(int sockfd, int backlog){
    int n;
    if( (n = listen(sockfd, backlog)) < 0)
        std::cerr << "listen error" << std::endl;
    return n;
}
// Convert a domain name to IP
// return format "xxx.xxx.xxx.xxx"
const char* HostToIp(const std::string& host) {
    hostent* hostname = gethostbyname(host.c_str());
    if(hostname)
        return inet_ntoa(**(in_addr**)hostname->h_addr_list);
    return NULL;
}
// Read a line from fd
int Readline(int fd, std::string& recvline){
    char buf;
    int n = 0;
    while((read(fd, &buf, 1) > 0) && (buf != '\n') && (buf != '\0')){
        recvline += buf;
        n++;
    }
    return n;        
}
int Inet_pton(int family, const char *str, void *addrptr){
	int n;
    if ( (n = inet_pton(family, str, addrptr) <= 0) ){
        std::cerr << "inet_pton error for " << str << std::endl;
        return n;
    }
	return 0;
}
const char* Inet_ntop(int family, const void *addrptr, char *str, size_t len){
	const char* result;
    if ( (result = inet_ntop(family, addrptr, str, len)) == NULL ) {
        std::cerr << "inet_ntop error for " << (size_t)addrptr << std::endl;
        return NULL;
    }
	return result;
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