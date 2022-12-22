#include "server.h"
#include "socketwrapper.cpp"
#define NIPQUAD(s) ((unsigned char *)&s)[0], \
                   ((unsigned char *)&s)[1], \
                   ((unsigned char *)&s)[2], \
                   ((unsigned char *)&s)[3]
using namespace std;
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
dns::dns(const char *config_file, in_port_t port)
{
    ifstream f_in(config_file);
    getline(f_in, this->forward_ip);
    string temp;
    while (getline(f_in, temp))
    {
        if (temp.back() == 0x0d)
            temp.pop_back();
        // include trailing '.'
        string root = temp.substr(0, temp.find(','));
        zones[root].zone_name = root;
        // open the zone file
        temp = temp.substr(temp.find(',') + 1);
        ifstream f_in2(temp.c_str());
        // skip the first line (is always current zone name)
        getline(f_in2, temp);
        string cur_name = root;
        bool has_subzone = false;
        while (getline(f_in2, temp))
        {
            int dot = temp.find(',');
            // temp[0] == '@', stick to the current zone name and insert RR records
            cur_name = (dot == 1) ? cur_name : (temp.substr(0, dot) + "." + root);
            temp = temp.substr(dot + 1);
            int TTL = strtol(temp.substr(0, temp.find(',')).c_str(), nullptr, 10);
            temp = temp.substr(temp.find(',') + 1);
            int CLASS = R_QCLASS_TABLE[temp.substr(0, temp.find(','))]; // IN
            temp = temp.substr(temp.find(',') + 1);
            int TYPE = R_QTYPE_TABLE[temp.substr(0, temp.find(','))];
            temp = temp.substr(temp.find(',') + 1);
            if (zones.find(cur_name) == zones.end())
            {
                zones[cur_name].zone_name = cur_name;
                has_subzone = true;
            }
            zones[cur_name].update_info(TTL, CLASS, TYPE, temp);
        }
        zones[root].has_subzone = has_subzone;
        f_in2.close();
    }
    f_in.close();

    bzero(&this->servaddr, sizeof(this->servaddr));
    this->servaddr.sin_family = AF_INET;
    this->servaddr.sin_port = port;
    this->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    this->sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    Bind(this->sockfd, (struct SA *)&this->servaddr, sizeof(this->servaddr));
    printf("Server start...\n");
}
dns::~dns()
{
}
void dns::summary()
{
    for (auto it : zones)
    {
        it.second.print_info();
    }
}

uint8_t *to_format(uint8_t *start_of_name, string &query_name)
{
    uint8_t total = 0;
    uint8_t *field_length = start_of_name;
    while (*field_length != 0)
    {
        /* Restore the dot in the name and advance to next length */
        total += *field_length + 1;
        query_name.append((char *)(field_length + 1), *field_length);
        query_name += ".";
        // *field_length = '.';
        field_length = start_of_name + total;
    }
    return field_length;
}
void parse_header(uint8_t *recvline, string &query_name, uint16_t &qtype, uint16_t &qclass)
{
    // struct header *hdrptr = (struct header *)recvline;
    uint8_t *start_of_name = recvline + sizeof(header);
    uint8_t *end_of_name = to_format(start_of_name, query_name);
    qtype = ntohs(*(uint16_t *)(end_of_name + 1));
    qclass = ntohs(*(uint16_t *)(end_of_name + 3));
}
void make_header(uint8_t *recvline)
{
}
void dns::start_query(const string &name, const uint16_t &qtype, const uint16_t &qclass, uint16_t &ancount, uint16_t &nscount, uint16_t &arcount)
{
    // check all answers in this node
    auto node = zones.find(name);
    // has the answer that fits the query
    vector<int> reply_ans(7, 0);
    zones[name].has_type(name, qtype, qclass, reply_ans);
}
void dns::start()
{
    size_t sz;
    uint8_t recvline[512];
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    while ((sz = recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct SA *)&cliaddr, &clilen)) > 0)
    {
        string query_zone;
        uint16_t qtype, qclass, ancount = 0, nscount = 0, arcount = 0;
        parse_header(recvline, query_zone, qtype, qclass);
        // cout << query_zone << " " << qtype << " " << qclass << endl;
        if(zones.find(query_zone) != zones.end()){
            start_query(query_zone, qtype, qclass, ancount, nscount, arcount);
        }
        else{

        }
        hexdump(recvline, sz);
        sendto(sockfd, recvline, sz, 0, (struct SA *)&cliaddr, clilen);
    }
}