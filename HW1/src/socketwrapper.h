#include "stdlib.h"

#define SA sockaddr
#define INFTIM -1
#define BACKLOGSIZE 1024

int Accept(int fd, struct SA *sa, socklen_t *salenptr);
int Socket(int family, int type, int protocol);
int Connect(int sockfd, struct SA *serv, socklen_t addrlen);
int Bind(int sockfd, struct SA *myaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
void Send(int fd, const char *ptr, size_t nbytes, int flags);
const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len);
int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout);
int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
int Start_TCP_Server(int* sockfd, uint16_t Port, uint64_t INADDR);
