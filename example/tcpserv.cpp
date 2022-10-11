#include "../Header/MySocket.h"

#define PORT 10000

using namespace std;


int main(){
    int listenfd, connfd;
    pid_t pid;
    socklen_t CLen;
    struct sockaddr_in CAddr;
    Start_TCP_Server(&listenfd, 9877); // listenfd set to listenfd

    while(1) {
        CLen = sizeof(CAddr);
        connfd = Accept(listenfd, (sockaddr*)&CAddr, CLen);

        if ( (pid = Fork()) == 0) {
            close(listenfd);
            // do something
            // str_echo(connfd);
            exit(0);
        }
        else {
            close(connfd);
        }
    }
    return 0;
}