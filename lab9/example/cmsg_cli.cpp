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
#include <fcntl.h>

// struct cmsghdr *CMSG_FIRSTHDR(struct msghdr *msgh);
// struct cmsghdr *CMSG_NXTHDR(struct msghdr *msgh,
//                             struct cmsghdr *cmsg);
// size_t CMSG_ALIGN(size_t length);
// size_t CMSG_SPACE(size_t length);
// size_t CMSG_LEN(size_t length);
// unsigned char *CMSG_DATA(struct cmsghdr *cmsg);

// struct cmsghdr *h, cm;
// for(h = CMSG_FIRSTHDR(&cm); h != nullptr; h = CMSG_NXTHDR(&cm, h)){
/* process record in h */
/* retrieve data from CMSG_DATA(h) */
// }

// sending descriptor example
#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
#define SA sockaddr
#define MAXLINE 1024
ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    union
    { /* allocate spaces for the ancillary data */
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr *cmptr;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(cmptr)) = sendfd;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    return (sendmsg(fd, &msg, 0));
}
int main(int argc, char **argv)
{
    int fd;
    if (argc != 4)
        err_quit("openfile <sockfd#> <filename> <mode>");
    if ((fd = open(argv[2], atoi(argv[3]))) < 0)
        exit((errno > 0) ? errno : 255);
    if (write_fd(atoi(argv[1]), (void*)"", 1, fd) < 0)
        exit((errno > 0) ? errno : 255);
    exit(0);
}