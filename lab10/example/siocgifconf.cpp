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
#define errquit(x) \
    {              \
        perror(x); \
        exit(-1);  \
    }

int main()
{
    int s;
    char *ptr, buf[8192];
    struct ifconf ifc = {sizeof(buf), {buf}};
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errquit("socket");
    if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
        errquit("ioctl");
    for (ptr = ifc.ifc_buf; ptr < ifc.ifc_buf + ifc.ifc_len;)
    {

        /* dump each record HERE */
        struct ifreq *ifr = (struct ifreq *)ptr;
        struct ifreq flags, mask, baddr;
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)&ifr->ifr_addr;
            struct sockaddr_in *sm = (struct sockaddr_in *)&mask.ifr_addr;
            struct sockaddr_in *sb = (struct sockaddr_in *)&baddr.ifr_addr;
            char s_addr[32], s_mask[32], s_baddr[32];
            bcopy(ifr, &flags, sizeof(flags)); /* get FLAGS */
            if (ioctl(s, SIOCGIFFLAGS, &flags) < 0)
                errquit("ioctl");
            bcopy(ifr, &baddr, sizeof(baddr)); /* get DST/BCAST address */
            if (flags.ifr_flags & IFF_POINTOPOINT)
            {
                if (ioctl(s, SIOCGIFDSTADDR, &baddr) < 0)
                    errquit("ioctl");
            }
            else if (flags.ifr_flags & IFF_BROADCAST)
            {
                if (ioctl(s, SIOCGIFBRDADDR, &baddr) < 0)
                    errquit("ioctl");
            }
            bcopy(ifr, &mask, sizeof(mask)); /* get NETMASK */
            if (ioctl(s, SIOCGIFNETMASK, &mask) < 0)
                errquit("ioctl");
            printf("%16s: %04x %s/%s %s\n", ifr->ifr_name,
                   flags.ifr_flags,
                   inet_ntop(AF_INET, &sin->sin_addr, s_addr, sizeof(s_addr)),
                   inet_ntop(AF_INET, &sm->sin_addr, s_mask, sizeof(s_mask)),
                   flags.ifr_flags & (IFF_POINTOPOINT | IFF_BROADCAST) ? inet_ntop(AF_INET, &sb->sin_addr, s_baddr, sizeof(s_baddr))
                                                                       : "(none)");
        }
#ifdef __linux__
        ptr += sizeof(struct ifreq);
#else
        ptr += sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len;
#endif
    }
    close(s);
    return 0;
}