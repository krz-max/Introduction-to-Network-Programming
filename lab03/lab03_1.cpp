#include <MySocket.h>

#define MAXLINE 5000000
using namespace std;

int main(int argc, char** argv){
    int sockfd, n;
    char recvline[MAXLINE];
    Start_TCP_Client(&sockfd, strtol(argv[2], NULL, 10), HostToIp(argv[1]));

    n = Readline(sockfd, recvline, MAXLINE);
    cout << recvline << endl;
    n = Readline(sockfd, recvline, MAXLINE);
    cout << recvline << endl;
    n = send(sockfd, "GO\n", 3, 0);
    n = Readline(sockfd, recvline, MAXLINE); //begin data
    cout << recvline << endl;
    n = Readline(sockfd, recvline, MAXLINE);
    // cout << recvline << endl;
    cout << n << endl;
    Readline(sockfd, recvline, MAXLINE); // end of data
    Readline(sockfd, recvline, MAXLINE); // how many
    cout << recvline << endl;
    char ans[15];
    strcpy(ans, to_string(n).c_str());
    strcat(ans, "\n");
    cout << "Bytes of data received: " << n << endl;
    send(sockfd, ans, strlen(ans), 0);
    Readline(sockfd, recvline, MAXLINE);
    cout << recvline << endl;
    return 0;

}
