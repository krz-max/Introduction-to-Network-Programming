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
#include <sys/wait.h>
#include <fstream>
#define err_quit(x) \
    {               \
        perror(x);  \
        exit(-1);   \
    }
#define NIPQUAD(a) ((unsigned char *)&(a))[0], \
                   ((unsigned char *)&(a))[1], \
                   ((unsigned char *)&(a))[2], \
                   ((unsigned char *)&(a))[3]
#define MAXLINE 1500

using namespace std;

static char addr[] = "10.113.113.255";
static int port = 1053;
string folderpath = "";
int kfile = 0;
const char endoffile[10] = "ENDOFFILE";

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
struct timeval wt = {0, 200000};
int sockfd = -1;
struct sockaddr_in servaddr;
socklen_t servlen;
void Start_Client()
{
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_port = htons(1053);
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &servaddr.sin_addr);
    if ((sockfd = socket(AF_INET, SOCK_RAW, 255)) < 0)
        err_quit("socket");
    const int on = 1;
    printf("%d\n", setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)));
    printf("%d\n", setsockopt(sockfd, SOL_SOCKET, IP_HDRINCL, &on, sizeof(on)));
}
void make_ip_hdr(char *buf, uint16_t total_len)
{
    struct ip *iph = (struct ip *)buf;
    iph->ip_v = 4;
    iph->ip_hl = 5;
    iph->ip_tos = 0;
    iph->ip_len = (((total_len + 31) / 32) * 32);
    iph->ip_id = rand();
    iph->ip_off = 0;
    iph->ip_ttl = 128;
    iph->ip_p = 255;
    iph->ip_src = servaddr.sin_addr;
    iph->ip_dst = servaddr.sin_addr;
    // modify later
    iph->ip_sum = cksum(iph, total_len);
}
void make_prot_hdr(char *buf, uint16_t len)
{
    struct prot *pthdr = (struct prot *)(buf + sizeof(ip));
    pthdr->type = 161;
    pthdr->payload_len = len;
}
void append_msg(char *buf, uint16_t len, std::string msg)
{
    // printf("%s\n", msg.c_str());
    char *start_of_data = (char *)(buf + sizeof(ip));
    strcpy(start_of_data, msg.c_str());
    start_of_data += msg.length();
    // padding
    for (int i = msg.length(); i < len - 20; i++)
    {
        *start_of_data = 0;
    }
}
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
void sendmsg(void *sendline, int size)
{
    // process message
    char buffer[1500];
    // make_prot_hdr(buffer, size);
    size = (((20 + size + 31) / 32) * 32);
    append_msg(buffer, size, (char *)sendline);
    // init ip
    make_ip_hdr(buffer, size);
    // hexdump(buffer, size);
    servlen = sizeof(servaddr);
    sendto(sockfd, buffer, size, 0, (struct sockaddr *)&servaddr, servlen);
}
int getresponse(void *buf)
{
    int n;
    n = recvfrom(sockfd, buf, MAXLINE - 1, 0, (sockaddr *)&servaddr, &servlen);
    if (n < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            fprintf(stderr, "C: timeout\n");
            return -1;
        }
        else
            err_quit("recvfrom error");
    }
    ((char *)buf)[n] = 0;
    // skip IP and prot
    buf = ((char *)buf + 24);
    // fprintf(stdout, "C: serv resp: %s\n", (char *)buf);
    return 0;
}
void dg_cli(int start, int k_file)
{
    size_t n;
    char filename[7] = {"000"};
    vector<int> fin_file(k_file, 0);
    for (int i = 0; i < k_file; i++)
    {
        filename[3] = (start + i) / 100 + '0';
        filename[4] = (((start + i) / 10) % 10) + '0';
        filename[5] = (start + i) % 10 + '0';
        filename[6] = 0;
        string path = folderpath + "/" + filename;
        ifstream f_in(path.c_str());
        if (!f_in)
            continue;
        long filesize = f_in.tellg();

        int howmanyfragment = 0;
        int fragment_num = 0;
        int seq_num = 0;
        vector<string> send_buffer;
        vector<int> send_idx_buffer;
        for (;;)
        {
            string buf;
            if (f_in >> buf)
            {
                int l = buf.length();
                int k = l / 900;
                for (int i = 0; i < k; i++)
                {
                    send_buffer.push_back(buf.substr(900 * i, 900));
                    send_idx_buffer.push_back(fragment_num);
                    fragment_num++;
                }
                if ((k = l % 900) > 0)
                {
                    send_buffer.push_back(buf.substr(900 * fragment_num));
                    send_idx_buffer.push_back(fragment_num++);
                }
            }
            else
                break;
        }
        howmanyfragment = send_buffer.size();
        string seq;
        fragment_num = 0;
        for (int i = 0; i < send_idx_buffer.size(); i++)
        {
            char sendline[MAXLINE], recvline[MAXLINE];
            sprintf(sendline, "FILENAME%sSEQNUM%3dSIZE%3ld%s", filename, i, send_buffer[i].length(), send_buffer[i].c_str());
            sendmsg((void *)sendline, 30 + send_buffer[i].length());
            usleep(3000);
        }
        filename[3] = (start + i) / 100 + '0';
        filename[4] = (((start + i) / 10) % 10) + '0';
        filename[5] = (start + i) % 10 + '0';
        filename[6] = 0;
        char end_req[MAXLINE];
        sprintf(end_req, "%s%s", endoffile, filename);
        sendmsg((void *)end_req, 15);
        // cout << "ENDOFFILE" << filename << endl;
        usleep(50000);
    }
    return ;
}
int main(int argc, char **argv)
{
    strcpy(addr, argv[3]);
    srand(0);
    char buf[1500];
    // sockfd is set
    Start_Client();

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    folderpath = argv[1];
    kfile = strtol(argv[2], nullptr, 10);
    dg_cli(0, kfile);
    return 0;
}