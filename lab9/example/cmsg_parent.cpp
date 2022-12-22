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
#include <sys/wait.h>
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
ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;
    union
    { /* allocate spaces for the ancillary data */
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr *cmptr;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    if ((n = recvmsg(fd, &msg, 0)) <= 0)
        return (n);
    /* check received result */
    if ((cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
        cmptr->cmsg_len == CMSG_LEN(sizeof(int)))
    {
        if (cmptr->cmsg_level != SOL_SOCKET)
            err_quit("control level != SOL_SOCKET");
        if (cmptr->cmsg_type != SCM_RIGHTS)
            err_quit("control type != SCM_RIGHTS");
        *recvfd = *((int *)CMSG_DATA(cmptr));
    }
    else
        *recvfd = -1; /* descriptor was not passed */
    return (n);
}
int my_open(const char *pathname, int mode)
{
    int fd, sockfd[2], status;
    pid_t childpid;
    char c, argsockfd[10], argmode[10];
    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);
    if ((childpid = fork()) == 0)
    {
        /* child process */
        close(sockfd[0]);
        snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
        snprintf(argmode, sizeof(argmode), "%d", mode);
        execl("./openfile", "openfile", argsockfd, pathname, argmode,
              (char *)NULL);
        err_quit("execl error");
    }
    /* parent process - wait for the child to terminate */
    close(sockfd[1]); /* close the end we don't use */
    waitpid(childpid, &status, 0);
    if (WIFEXITED(status) == 0)
        err_quit("child did not terminate");
    if ((status = WEXITSTATUS(status)) == 0)
        read_fd(sockfd[0], &c, 1, &fd);
    else
    {
        errno = status; /* set errno value from child's status */
        fd = -1;
    }
    close(sockfd[0]);
    return (fd);
}
int main(int argc, char **argv)
{
    int fd, n;
    char buff[MAXLINE];
    if (argc != 2)
        err_quit("usage: mycat <pathname>");
    if ((fd = my_open(argv[1], O_RDONLY)) < 0)
        err_quit("cannot open file");
    while ((n = read(fd, buff, MAXLINE)) > 0)
        write(STDOUT_FILENO, buff, n);
    exit(0);
}