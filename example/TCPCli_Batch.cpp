#include "../Header/MySocket.h"

#define PORT 9877
#define IP "127.0.0.1"
#define MAXLINE 2000

using namespace std;

void str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
    int maxfdp1;
    int stdineof = 0;
    fd_set rset;

    FD_ZERO(&rset);

    while (1) {
        FD_SET(fileno(fp), &rset);
        if ( stdineof == 0 )
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, 0, 0, 0);

        if ( FD_ISSET(sockfd, &rset) ) {
            if ( Readline(sockfd, recvline, MAXLINE) == 0 ) {
                /* ORIGINAL: return here */
                if ( stdineof == 1 )
                    return ;
                else {
                    err_sys("str_cli: server terminate permanently");
                    return ;
                }
            }
            fputs(recvline, stdout);
        }

        if ( FD_ISSET(fileno(fp), &rset) ) {
            if ( fgets(sendline, MAXLINE, fp) == NULL ) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                // close(sockfd);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
    return ;
}


int main(int argc, char** argv){
    int i, sockfd[5];
    struct sockaddr_in SAddr;
	if (argc != 2) {
		err_sys("usage: tcpcli <IPaddress>");
        return -1;
    }
    Start_TCP_Client(&sockfd[0], PORT, IP);

    str_cli(stdin, sockfd[0]);
    return 0;
}