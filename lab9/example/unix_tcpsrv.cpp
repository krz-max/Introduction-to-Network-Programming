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

void str_echo(int sockfd){
    int n;
    char buf[MAXLINE];
again:
    while ((n=read(sockfd, buf, MAXLINE)) > 0){
        write(sockfd, buf, n);
    }
    if(n < 0 && errno == EINTR)
        goto again;
    else if(n < 0)
        err_quit("str_echo: read error");
}
void sig_chld(int){}
// stream socket : server
int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_un cliaddr, servaddr;
    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(argv[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, 1024);
    signal(SIGCHLD, sig_chld);
    // ...
    for (;;)
    {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0)
        {
            if (errno == EINTR)
                continue; /* back to for() */
            else
                perror("accept error");
        }
        if ((childpid = fork()) == 0)
        {                     /* child process */
            close(listenfd);  /* close listening socket */
            str_echo(connfd); /* process request */
            exit(0);
        }
        close(connfd); /* parent closes connected socket */
    }
}