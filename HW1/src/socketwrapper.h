#ifndef STRING_HEADER
#define STRING_HEADER

#include <cstring>
#include <string>
#include <sstream>
#endif

#ifndef STD_HEADER
#define STD_HEADER
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> // strtol(string, endpointer, base)
#include <limits.h>
#include <sys/wait.h>
#endif

#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define SA sockaddr
#define INFTIM -1
#define BACKLOGSIZE 1024

void err_sys(const char *x);
int Accept(int fd, struct SA *sa, socklen_t *salenptr);
int Socket(int family, int type, int protocol);
int Connect(int sockfd, struct SA *serv, socklen_t addrlen);
int Bind(int sockfd, struct SA *myaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
const char *HostToIp(const std::string &host);
void Send(int fd, const char *ptr, size_t nbytes, int flags);
const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len);
int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout);
int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout = NULL);
int Start_TCP_Server(int* sockfd, uint16_t Port, uint64_t INADDR = INADDR_ANY);
