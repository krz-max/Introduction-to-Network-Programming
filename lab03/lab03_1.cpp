#include <MySocket.h>

using namespace std;

int main(int argc, char** argv){
    int sockfd, n;
    string recvline;
    Start_TCP_Client(&sockfd, strtol(argv[2], NULL, 10), HostToIp(argv[1]));

    n = Readline(sockfd, recvline);
    cout << recvline << endl;
    recvline = "";
    n = Readline(sockfd, recvline);
    cout << recvline << endl;
    
    recvline = "";
    n = send(sockfd, "GO\n", 3, 0);
    n = Readline(sockfd, recvline); //begin data
    cout << recvline << endl;
    recvline = "";
    n = Readline(sockfd, recvline);
    // cout << recvline << endl;
    cout << n << endl;
    recvline = "";
    Readline(sockfd, recvline); // end of data
    cout << recvline << endl;
    recvline = "";
    Readline(sockfd, recvline); // how many
    cout << recvline << endl;
    char ans[15];
    strcpy(ans, to_string(n).c_str());
    strcat(ans, "\n");
    n = send(sockfd, ans, strlen(ans), 0);
    recvline = "";
    Readline(sockfd, recvline);
    cout << recvline << endl;
    return 0;

}
