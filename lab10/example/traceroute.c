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
#include <errno.h>
#define err_quit(x) \
    {               \
        perror(x);  \
        exit(-1);   \
    }
#define NIPQUAD(a) ((unsigned char *)&(a))[0], \
                   ((unsigned char *)&(a))[1], \
                   ((unsigned char *)&(a))[2], \
                   ((unsigned char *)&(a))[3]
#define MAXHOP 10
int probe4(int u, int r, struct sockaddr_in *dst)
{
    int timeout = 1, done = 0, sz;
    char buf[2048];
    struct ip *iph = (struct ip *)buf, *iph2 = NULL;
    struct icmp *icmph = NULL;
    struct udphdr *udph = NULL;
    struct timeval ts, tr;
    gettimeofday(&ts, NULL);
    dst->sin_port = htons(30000 + random() % 30000);
    if (sendto(u, &ts, sizeof(ts), 0,
               (struct sockaddr *)dst, sizeof(*dst)) < 0)
        err_quit("sendto");
    do
    {
        gettimeofday(&tr, NULL);
        /* ... wait for reply until timed-out */
        if ((sz = recv(r, buf, sizeof(buf), MSG_TRUNC)) < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }
            err_quit("recv");
        }
        gettimeofday(&tr, NULL);
        if (iph->ip_v != 4)
            continue;
        if (iph->ip_p != IPPROTO_ICMP)
            continue;
        if (iph->ip_src.s_addr == dst->sin_addr.s_addr)
            done = 1;
        icmph = (struct icmp *)(buf + (iph->ip_hl << 2));
        if (icmph->icmp_type != 3 && icmph->icmp_type != 11)
            continue;
        if (icmph->icmp_type == 3 && icmph->icmp_code == 3)
            done = 1;
        iph2 = (struct ip *)(buf + (iph->ip_hl << 2) + 8);
        if (iph2->ip_v != 4)
            continue;
        if (iph2->ip_p != IPPROTO_UDP)
            continue;
        if (iph2->ip_dst.s_addr != dst->sin_addr.s_addr)
            continue;
        udph = (struct udphdr *)(buf + (iph->ip_hl << 2) + 8 + (iph2->ip_hl << 2));
        if (udph->uh_dport != dst->sin_port)
            continue;
        snprintf(buf, sizeof(buf), "%s (%.4f ms)",
                 inet_ntoa(*((struct in_addr *)&iph->ip_src)),
                 tv2ms(tr) - tv2ms(ts));
        printf(" %-28s", buf);
        timeout = 0;
        break;
    } while (tv2ms(tr) - tv2ms(ts) < 3 * 1000);
    if (timeout)
        printf(" %-28s", "*");
    return done;
}
int main(int argc, char *argv[])
{
    int i, j, r, u, done = 0;
    struct sockaddr_in dst;
    struct timeval rto = {3, 0};
    setvbuf(stdout, NULL, _IONBF, 0);
    srand(time(0) ^ getpid());
    if (argc < 2)
        return usage();
    if ((r = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        err_quit("rs");
    if ((u = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        err_quit("us");
    bzero(&dst, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr(argv[1]);
    if (dst.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *h = gethostbyname(argv[1]);
        if (h == NULL)
        {
            herror("traceroute");
            return -1;
        }
        dst.sin_addr.s_addr = *((unsigned int *)h->h_addr_list[0]);
    }
    printf("TRACEROUTE %s (%s) maxhop %d\n", argv[1],
           inet_ntoa(dst.sin_addr), MAXHOP);
    if (setsockopt(r, SOL_SOCKET, SO_RCVTIMEO, &rto, sizeof(rto)) < 0)
        err_quit("setsockopt");
    for (i = 1; i <= MAXHOP && !done; i++)
    {
        if (setsockopt(u, IPPROTO_IP, IP_TTL, &i, sizeof(i)) < 0)
            err_quit("setsockopt");
        printf("%3d:", i);
        for (j = 0; j < 3; j++)
        {
            done = probe(u, r, &dst);
        }
    }
    return 0;
}