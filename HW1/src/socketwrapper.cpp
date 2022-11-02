#include "socketwrapper.h"

void err_sys(const char *x)
{
    perror(x);
    exit(1);
}

/*
Used to wait for a client connection
Prototype:
    int Accept(int fd, struct SA *sa, socklen_t *salenptr);
Example:
    socklen_t clilen;
    sockaddr_in cliaddr;
    int connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
*/
int Accept(int fd, struct SA *sa, socklen_t *salenptr)
{
    int n;
    if ((n = accept(fd, sa, salenptr)) < 0)
    {
        err_sys("accept error");
    }
    return (n);
}
/*
Create a Socket
Prototype:
    int Socket(int family, int type, int protocol);
Example:
    Socket(AF_INET, SOCK_STREAM, 0);
*/
int Socket(int family, int type, int protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0)
        fprintf(stderr, "socket error\n");
    return n;
}
/*
Initiate 3-way handshaking, a function for client
Prototype:
    int Connect(int sockfd, struct SA *serv, socklen_t addrlen);
Example:
    Connect(AF_INET, SOCK_STREAM, 0);
*/
int Connect(int sockfd, struct SA *serv, socklen_t addrlen)
{
    int n;
    if ((n = connect(sockfd, serv, addrlen)) < 0)
        fprintf(stderr, "connect error\n");
    return n;
}
/*
For Server, bind the server to the IP address and port
Prototype:
    int Bind(int sockfd, struct SA *myaddr, socklen_t addrlen);
Example:
    SAddr.sin_family      = AF_INET
    SAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    SAddr.sin_port        = htons(SERV_PORT);
    Bind(sockfd, (SA*)&SAddr, sizeof(SAddr));
*/
int Bind(int sockfd, struct SA *myaddr, socklen_t addrlen)
{
    int n;
    if ((n = bind(sockfd, myaddr, addrlen)) < 0)
        fprintf(stderr, "bind error\n");
    return n;
}
/*
For Server, Set the listenfd for the server
Prototype:
    int Listen(int sockfd, int backlog);
Example:
    SAddr.sin_family      = AF_INET
    SAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    SAddr.sin_port        = htons(SERV_PORT);
    Bind(sockfd, (SA*)&SAddr, sizeof(SAddr));
    Listen(sockfd, BACKLOGSIZE);
*/
int Listen(int sockfd, int backlog)
{
    int n;
    if ((n = listen(sockfd, backlog)) < 0)
        fprintf(stderr, "listen error\n");
    return n;
}

/*
Send message in ptr to fd
Prototype:
    void Send(int fd, const char *ptr, size_t nbytes, int flags)
Example:
    Send(sockfd, "Hello", 5, 0);
*/
void Send(int fd, const char *ptr, size_t nbytes, int flags)
{
    if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes)
        err_sys("send error");
}
/*
Convert Network Byte Order to "xxx.xxx.xxx.xxx" IP
Prototype:
    const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len)
Example:
    Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
*/
const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len)
{
    const char *result;
    if ((result = inet_ntop(family, addrptr, str, len)) == NULL)
    {
        fprintf(stderr, "inet_ntop error for %ld\n", (size_t)addrptr);
        return NULL;
    }
    return result;
}

/*
Used for single process handling several clients
Prototype:
    int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
*/
int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
{
    int n;

    if ((n = poll(fdarray, nfds, timeout)) < 0)
        err_sys("poll error");

    return (n);
}
/*
Prototype:
    int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout = NULL)
return positive count of ready descriptor, 0 on timeout, -1 on error
*/
int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout = NULL)
{
    int n;
    if ((n = select(maxfdp1, readset, writeset, exceptset, timeout) <= 0))
    {
        err_sys("select error");
        return -1;
    }
    return n;
}
/*
Prototype:
    int Start_TCP_Server(int *sockfd, uint16_t Port, uint64_t INADDR = INADDR_ANY)
Example:
    Start_TCP_Server(int *sockfd, uint16_t Port, uint64_t INADDR = INADDR_ANY)
*/
int Start_TCP_Server(int* sockfd, uint16_t Port, uint64_t INADDR = INADDR_ANY)
{
    struct sockaddr_in SAddr;
    bzero(&SAddr, sizeof(SAddr));
    *sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    SAddr.sin_family = AF_INET;
    SAddr.sin_port = htons(Port);
    SAddr.sin_addr.s_addr = htonl(INADDR);
    Bind(*sockfd, (struct SA *)&SAddr, sizeof(SAddr));
    Listen(*sockfd, BACKLOGSIZE);
    return 0;
}
