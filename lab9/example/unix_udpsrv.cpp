#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>
#include <cerrno>

#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
#define SA sockaddr
#define MAXLINE 1024
// int socketpair(int family, int type, int protocol, int sockfd[2]);
// family : AF_LOCAL / AF_UNIX
// protocol : 0
// call fork to let parent and child talk via the paired socket descriptor

void dg_echo(int sockfd, sockaddr *pcliaddr, socklen_t clilen)
{
    int n;
    socklen_t len;
    char buf[MAXLINE];
    for (;;)
    {
        len = clilen;
        n = recvfrom(sockfd, buf, MAXLINE, 0, pcliaddr, &len);
        sendto(sockfd, buf, n, 0, pcliaddr, len);
    }
}
void sig_chld(int) {}
// dgram socket : server
int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un servaddr, cliaddr;
    sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    unlink(argv[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);
    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
    dg_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
}