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
#include <netinet/ip.h>

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
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
static char addr[20];
static int port = 1053;
const char ack[4][6] = {{"ACKFQ"}, {"ACKFN"}, {"ACKCQ"}, {"ACKEF"}};
const char ACK[4] = "ACK";
using namespace std;
string folderpath;
int kfile;
#define MAXLINE 1500
// struct ip
//   {
// #if __BYTE_ORDER == __LITTLE_ENDIAN
//     unsigned int ip_hl:4;		/* header length */
//     unsigned int ip_v:4;		/* version */
// #endif
// #if __BYTE_ORDER == __BIG_ENDIAN
//     unsigned int ip_v:4;		/* version */
//     unsigned int ip_hl:4;		/* header length */
// #endif
//     uint8_t ip_tos;			/* type of service */
//     unsigned short ip_len;		/* total length */
//     unsigned short ip_id;		/* identification */
//     unsigned short ip_off;		/* fragment offset field */
// #define	IP_RF 0x8000			/* reserved fragment flag */
// #define	IP_DF 0x4000			/* dont fragment flag */
// #define	IP_MF 0x2000			/* more fragments flag */
// #define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
//     uint8_t ip_ttl;			/* time to live */
//     uint8_t ip_p;			/* protocol */
//     unsigned short ip_sum;		/* checksum */
//     struct in_addr ip_src, ip_dst;	/* source and dest address */
// };
struct prot
{
    uint16_t type;
    uint16_t payload_len;
};
uint16_t cksum(void *in, int sz)
{
    long sum = 0;
    uint16_t *ptr = (unsigned short *)in;
    for (; sz > 1; sz -= 2)
        sum += *ptr++;
    if (sz > 0)
        sum += *((unsigned char *)ptr);
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

int sockfd = -1;
struct sockaddr_in servaddr, cliaddr;
socklen_t servlen, clilen;
void Start_Server()
{
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, addr, &servaddr.sin_addr);
    if ((sockfd = socket(AF_INET, SOCK_RAW, 255)) < 0)
        err_quit("socket");
    const int on = 1;
    const int buf_size = 20 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    setsockopt(sockfd, SOL_SOCKET, IP_HDRINCL, &on, sizeof(on));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
}
int main(int argc, char **argv)
{
    strcpy(addr, argv[1]);
    srand(0);
    char buf[1500];
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    // sockfd is set
    Start_Server();
    size_t sz;
    struct timeval tv;
    size_t count = 0;

    folderpath = argv[1];
    kfile = strtol(argv[2], nullptr, 10);
    printf("Serv start...\n");

    int rcvwait;
    size_t n;
    string path;
    ofstream f_out;
    map<int, string> output_buf;
    while ((sz = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen)))
    {
        // printf("Serv received\n");
        struct ip *iph = (struct ip *)(buf);
        gettimeofday(&tv, nullptr);
        // printf("# %ld %lu.%06lu\n", ++count, tv.tv_sec, tv.tv_usec);
        // printf("Ipv%u hdrlen %u: %u.%u.%u.%u -> %u.%u.%u.%u len %u ttl %u proto %u\n",
        //        iph->ip_v, (iph->ip_hl << 2), NIPQUAD(iph->ip_src), NIPQUAD(iph->ip_dst), ntohs(iph->ip_len), iph->ip_ttl, iph->ip_p);
        char *recvline = (char *)(buf + sizeof(ip));
        
        string proc = string(recvline);
        if (!strncmp(recvline, "ENDOFFILE", 9))
        {
            string fname = proc.substr(9, 6);
            int fID = strtol(fname.substr(3).c_str(), NULL, 10);
            char sendline[MAXLINE];
            f_out.open((folderpath + "/" + fname).c_str());
            cout << "write file: " << fname << ", ID: " << fID << endl;
            for (auto it : output_buf)
            {
                // cout << "write seq_num: " << it.first << endl;
                f_out << it.second;
            }
            f_out.close();
            output_buf.clear();
            continue;
        }
        else
        {
            string size = proc.substr(27, 3);
            int offset = 0;
            if (size[0] == ' ')
                offset++;
            if (size[1] == ' ')
                offset++;
            size = size.substr(offset);
            int len = strtol(size.c_str(), NULL, 10);
            string data = proc.substr(30);
            // fprintf(stdout, "len: %d, true len: %ld", len, data.length());
            if (len != data.length())
                continue;

            string sqnum = proc.substr(20, 3);
            int seq_num = strtol(sqnum.c_str(), NULL, 10);

            output_buf[seq_num] = data;
            // fprintf(stdout, "FILE: %s, seq num: %s, now size: %ld\n", fname.c_str(), sqnum.c_str(), output_buf[fID].size());
        }
    }
    return 0;
}