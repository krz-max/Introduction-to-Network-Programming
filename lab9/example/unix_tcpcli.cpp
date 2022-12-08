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

void str_cli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    while (fgets(sendline, MAXLINE, fp) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        if(read(sockfd, recvline, MAXLINE) == 0){
            err_quit("str_cli: server terminated prematurely");
        }
        fputs(recvline, stdout);
    }
}
int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un servaddr;
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);
    connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr));
    str_cli(stdin, sockfd); /* do it all */
    exit(0);
}