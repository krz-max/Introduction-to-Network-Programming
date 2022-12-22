#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/wait.h>

#define err_quit(s) \
    {               \
        perror(s);  \
        exit(-1);   \
    }
#define NIPQUAD(a) ((unsigned char *)&(a))[0], \
                   ((unsigned char *)&(a))[1], \
                   ((unsigned char *)&(a))[2], \
                   ((unsigned char *)&(a))[3]
void hexdump(void *in, int sz)
{
    int i, j;
    unsigned char *buf = (unsigned char *)in;
    for (j = 0; j < sz; j += 16)
    {
        printf("%06x | ", j);
        for (i = 0; i < 16 && i + j < sz; i++)
            printf("%2.2x ", buf[i + j]);
        for (; i < 16; i++)
            printf(" ");
        printf("| ");
        for (i = 0; i < 16 && i + j < sz; i++)
            printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
        for (; i < 16; i++)
            printf(" ");
        printf(" |\n");
    }
}
int main(int argc, char *argv[])
{
    int s;
    struct timeval tv;
    int count = 0;
    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        err_quit("socket");
    char buf[2048];
    size_t sz;
    while ((sz = recv(s, buf, sizeof(buf), MSG_TRUNC)) > 0)
    {
        struct ip *iph = (struct ip *)buf;
        gettimeofday(&tv, NULL);
        printf("# %d %lu.%06lu\n", ++count, tv.tv_sec, tv.tv_usec);
        printf("IPv%d hdrlen %u: %u.%u.%u.%u -> %u.%u.%u.%u "
               "len %u ttl %u proto %u\n",
               iph->ip_v, iph->ip_hl << 2,
               NIPQUAD(iph->ip_src), NIPQUAD(iph->ip_dst),
               ntohs(iph->ip_len), iph->ip_ttl, iph->ip_p);
        hexdump(buf, sz);
    }
}