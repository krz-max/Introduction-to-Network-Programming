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

#include <vector>
#include <iostream>
#include <string>
#include <sys/wait.h>
#define err_quit(x) \
    {               \
        perror(x);  \
        exit(-1);   \
    }
#define NIPQUAD(a) ((unsigned char *)&(a))[0], \
                   ((unsigned char *)&(a))[1], \
                   ((unsigned char *)&(a))[2], \
                   ((unsigned char *)&(a))[3]

unsigned short cksum(void *in, int sz)
{
    long sum = 0;
    unsigned short *ptr = (unsigned short *)in;
    for (; sz > 1; sz -= 2)
        sum += *ptr++;
    if (sz > 0)
        sum += *((unsigned char *)ptr);
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}
int echorsp1(int s, unsigned short id, struct sockaddr_in src)
{
    int sz;
    char buf[2048];
    struct ip *iph = (struct ip *)buf;
    struct icmp *icmph = NULL;
    struct timeval tv, *ptv = NULL;
    if ((sz = recv(s, buf, sizeof(buf), MSG_TRUNC)) <= 0)
        return sz;
    gettimeofday(&tv, NULL);
    if (iph->ip_v != 4)
        return -1;
    if (iph->ip_src.s_addr != src.sin_addr.s_addr)
        return -1;
    if (htons(iph->ip_len) < (iph->ip_hl << 1) + 8)
        return -1;
    icmph = (struct icmp *)(buf + (iph->ip_hl << 2));
    if (icmph->icmp_type != 0)
        return -1;
    if (icmph->icmp_code != 0)
        return -1;
    if (icmph->icmp_id != htons(id))
        return -1;
    ptv = (struct timeval *)(buf + (iph->ip_hl << 2) + 8);
    printf("%lu.%06lu: %d bytes from %u.%u.%u.%u to %u.%u.%u.%u: "
           "icmp_seq=%u ttl=%u time=%.4f ms\n",
           tv.tv_sec, tv.tv_usec,
           sz,
           NIPQUAD(iph->ip_src),
           NIPQUAD(iph->ip_dst),
           ntohs(icmph->icmp_seq),
           iph->ip_ttl,
           tv2ms(tv) - tv2ms(*ptv));
    return sz;
}
double tv2ms(struct timeval tv)
{
    return 1000.0 * tv.tv_sec + 0.001 * tv.tv_usec;
}
int echoreq1(int s, unsigned short id, unsigned short seq)
{
    char buf[64];
    struct icmp *icmph = (struct icmp *)buf;
    struct timeval *ptv = (struct timeval *)(buf + 8);
    bcopy("FEDCBA9876543210", buf, 16);
    icmph = (struct icmp *)buf;
    icmph->icmp_type = 8;
    icmph->icmp_code = 0;
    icmph->icmp_id = htons(id);
    icmph->icmp_seq = htons(seq);
    icmph->icmp_cksum = 0;
    gettimeofday(ptv, NULL);
    icmph->icmp_cksum = cksum(buf, 8 + sizeof(struct timeval));
    return write(s, buf, 8 + sizeof(struct timeval));
}
static int s = -1;                     /* socket */
static unsigned short seq = 0, id = 0; /* ICMP seq and id */
int main(int argc, char *argv[])
{
    struct sockaddr_in dst;
    if (argc < 2)
        return usage();
    srand(time(0) ^ getpid());
    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        err_quit("s");
    bzero(&dst, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr(argv[1]);
    if (connect(s, (struct sockaddr *)&dst, sizeof(dst)) < 0)
        err_quit("c");
    printf("PING %s\n", argv[1]);
    if (echoreq1(s, id = random() & 0xffff, 1) < 0)
        err_quit("echoreq1");
    alarm(3);
    while (1)
    {
        if (echorsp1(s, id, dst) > 0)
            break;
    }
    return 0;
}