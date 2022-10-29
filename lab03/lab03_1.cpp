#include "header.h"

#define MAXLINE 5000000
using namespace std;

int main(int argc, char** argv){
    int sockfd, n;
    char recvline[MAXLINE];
    Start_TCP_Client(&sockfd, strtol(argv[2], NULL, 10), HostToIp(argv[1]));

    n = Readline(sockfd, recvline, MAXLINE);
    fprintf(stdout, "%s\n", recvline);
    n = Readline(sockfd, recvline, MAXLINE);
    fprintf(stdout, "%s\n", recvline);
    n = send(sockfd, "GO\n", 3, 0);
    n = Readline(sockfd, recvline, MAXLINE); //begin data
    fprintf(stdout, "%s\n", recvline);
    n = Readline(sockfd, recvline, MAXLINE);
    fprintf(stdout, "%d\n", n);
    Readline(sockfd, recvline, MAXLINE); // end of data
    Readline(sockfd, recvline, MAXLINE); // how many
    fprintf(stdout, "%s\n", recvline);
    char ans[15];
    strcpy(ans, to_string(n).c_str());
    strcat(ans, "\n");
    fprintf(stdout, "Bytes of data recerived: %d\n", n);
    send(sockfd, ans, strlen(ans), 0);
    Readline(sockfd, recvline, MAXLINE);
    fprintf(stdout, "%s\n", recvline);
    return 0;

}
