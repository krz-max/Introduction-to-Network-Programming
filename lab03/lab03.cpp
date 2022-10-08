#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>


using namespace std;

int Socket(int family, int type, int protocol){ // Create a socket
    int n;
    if( (n = socket(family, type, protocol)) < 0)
        cerr << "socket error" << endl;
    return n;
}
int Connect(int sockfd, struct sockaddr *serv, socklen_t addrlen){ // for TCP, initiate three-way handshaking
    int n;
    if( (n = connect(sockfd, serv, addrlen)) < 0)
        cerr << "connect error" << endl;
    return n;
}
int Bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen){
    int n;
    if( (n = bind(sockfd, myaddr, addrlen)) < 0)
        cerr << "bind error" << endl;
    return n;
}
int Listen(int sockfd, int backlog){
    int n;
    if( (n = listen(sockfd, backlog)) < 0)
        cerr << "listen error" << endl;
    return n;
}
string HostToIp(const string& host) {
    hostent* hostname = gethostbyname(host.c_str());
    if(hostname)
        return string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    return {};
}
int readline(int fd, string& recvline){
    char buf;
    int n = 0;
    while(1){
        if(read(fd, &buf, 1) > 0){
            if(buf != '\n' && buf != '\0'){
                recvline += buf;
                n++;
            }
            else{
                break;
            }
        }
        else{
            break;
        }
    }
    return n;        
}


int main(int argc, char** argv){
    int sockfd, n;
    string recvline;
    struct sockaddr_in servaddr;
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    // int sock = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10002);
    if (inet_pton(AF_INET, "140.113.213.213", &servaddr.sin_addr) <= 0){
        cerr << "inet_pton error for " << argv[1] << endl;
        return -1;
    }
    Connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));
    n = readline(sockfd, recvline);
    cout << recvline << endl;
    recvline = "";
    n = readline(sockfd, recvline);
    cout << recvline << endl;
    
    recvline = "";
    n = send(sockfd, "GO\n", 3, 0);
    n = readline(sockfd, recvline); //begin data
    cout << recvline << endl;
    recvline = "";
    n = readline(sockfd, recvline);
    // cout << recvline << endl;
    cout << n << endl;
    recvline = "";
    readline(sockfd, recvline); // end of data
    cout << recvline << endl;
    recvline = "";
    readline(sockfd, recvline); // how many
    cout << recvline << endl;
    char ans[15];
    strcpy(ans, to_string(n).c_str());
    strcat(ans, "\n");
    n = send(sockfd, ans, strlen(ans), 0);
    recvline = "";
    readline(sockfd, recvline);
    cout << recvline << endl;
    return 0;

}
