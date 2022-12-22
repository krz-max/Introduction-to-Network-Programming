#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

#define err_quit(m) { perror(m); exit(-1); }
// // // pass the length of the structure as (sizeof(sa_family_t) + strlen(sun_path) + 1)
// struct sockaddr_un{
//     sa_family_t sun_family; /* AF_UNIX | AF_LOCAL */
//     char sun_path[104]; /* null-terminated pathname */
// };

int main(int argc, char **argv){
    int sockfd;
    socklen_t len;
    struct sockaddr_un addr1, addr2;
    if(argc != 2) err_quit("usage: unixbind <pathname>");
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(argv[1]);
    bzero(&addr1, sizeof(addr1));
    addr1.sun_family = AF_LOCAL;
    strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path)-1);
    bind(sockfd, (sockaddr *)&addr1, SUN_LEN(&addr1));
    len = sizeof(addr2);
    getsockname(sockfd, (sockaddr *)&addr2, &len);
    printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);
    return 0;
}