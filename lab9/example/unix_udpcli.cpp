#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
#define MAXLINE 1024

void dg_cli(FILE *fp, int sockfd, const sockaddr *pservaddr, socklen_t servlen)
{
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    while (fgets(sendline, MAXLINE, fp) != NULL)
    {
        sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
        n = recvfrom(sockfd, recvline, MAXLINE, 0, nullptr, nullptr);
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}
int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un cliaddr, servaddr;
    sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    bzero(&cliaddr, sizeof(cliaddr)); /* bind an address for us */
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, tmpnam(NULL));
    bind(sockfd, (sockaddr *)&cliaddr, sizeof(cliaddr));
    bzero(&servaddr, sizeof(servaddr)); /* fill in server's address */
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);
    dg_cli(stdin, sockfd, (sockaddr *)&servaddr, sizeof(servaddr));
    exit(0);
}