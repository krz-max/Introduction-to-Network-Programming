#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <string>
#include <list>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
#define NIPQUAD(s) ((unsigned char *)&s)[0], \
                   ((unsigned char *)&s)[1], \
                   ((unsigned char *)&s)[2], \
                   ((unsigned char *)&s)[3]
using namespace std;

static unordered_map<uint16_t, string> QTYPE_TABLE =
    {{1, "A"}, {2, "NS"}, {5, "CNAME"}, {6, "SOA"}, {15, "MX"}, {16, "TXT"}, {7, "AAAA"}};
static unordered_map<uint16_t, string> QCLASS_TABLE =
    {{1, "IN"}, {2, "CS"}, {3, "CH"}, {4, "HS"}};
static string forward_ip;

typedef struct header
{
    uint16_t ID;
    uint16_t flags;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
} header_t;

typedef struct Question
{
    char *QNAME;
    uint16_t QTYPE;
    uint16_t QCLASS;
} question_t;
typedef struct Response
{
    uint16_t _compression;
    uint16_t _type;
    uint16_t _class;
    uint32_t _ttl;
    uint16_t _length;
    struct in_addr _addr;
} __attribute__((packed)) response_t;
struct pkt_info
{
    string _name;
    uint16_t _type;
    uint16_t _class;
    uint32_t _ttl;
    uint16_t _rdlength;
    string _rdata;
    pkt_info(string name, string type, string Class, uint32_t ttl, string rdata)
    {
        this->_name = name;
        this->_ttl = htonl(ttl);
        for (auto it : QTYPE_TABLE)
        {
            if (it.second == type)
            {
                this->_type = htons(it.first);
                break;
            }
        }
        for (auto it : QCLASS_TABLE)
        {
            if (it.second == Class)
            {
                this->_class = htons(it.first);
                break;
            }
        }
        if (type == "SOA")
        {
            rdata[rdata.find(' ')] = ',';
            rdata[rdata.find(' ')] = ',';
            // dns.example1.org.,admin.example1.org.,2022121301 3600 300 3600000 3600
            cout << rdata.rfind(',') +12+1<< endl;
            this->_rdlength = htons(rdata.rfind(',') + 12 + 1);
        }
        this->_rdata = rdata;
    }
    void print_node_info()
    {
        printf("name: %s\ntype: %u, class: %u, ttl: %u, rdlength: %u, rdata: %s\n",
               _name.c_str(), _type, _class, _ttl, _rdlength, _rdata.c_str());
    }
};
struct zone
{
    string zone_name;
    vector<pkt_info> nodes;
    void print_info()
    {
        for (auto it : nodes)
            it.print_node_info();
    }
    void update_info(string line)
    {
        string name = line.substr(0, line.find(','));
        line = line.substr(name.length() + 1); // skip ','
        name = (name == "@") ? zone_name : name + "." + zone_name;
        uint32_t ttl = strtoul(line.substr(0, line.find(',')).c_str(), nullptr, 10);
        line = line.substr(line.find(',') + 1);
        string Class = line.substr(0, line.find(','));
        line = line.substr(line.find(',') + 1);
        string type = line.substr(0, line.find(','));
        string Rdata = line.substr(line.find(',') + 1);
        nodes.push_back(pkt_info(name, type, Class, ttl, Rdata));
    }
    vector<pkt_info> find_answer(string Name, uint16_t Type, uint16_t Class)
    {
        vector<pkt_info> res;
        if (Type == 1 || Type == 6)
        { // A, SOA response
            for (auto it : nodes)
            {
                if (Name == it._name && ntohs(it._type) == 6)
                { // exactly same name
                    res.push_back(it);
                }
            }
        }
        return res;
    }
};

static unordered_map<string, zone> zones;

void init(const char *filepath)
{
    ifstream f_in("config.txt");
    getline(f_in, forward_ip);
    string temp;
    while (getline(f_in, temp))
    {
        // include trailing '.'
        string cur_name = temp.substr(0, temp.find(','));
        zones[cur_name].zone_name = cur_name;
        ifstream f_in2(temp.substr(temp.find(',') + 1).c_str());
        getline(f_in2, temp);
        while (getline(f_in2, temp))
        {
            zones[cur_name].update_info(temp);
        }
        f_in2.close();
    }
    for (auto it : zones)
    {
        it.second.print_info();
    }
    f_in.close();
}
void to_format(string &name)
{
    name.insert(name.begin(), 1, '.');
    int cur = name.find('.'); // should be 0
    while (name.find('.', cur + 1) != string::npos)
    {
        name[cur] = name.find('.', cur + 1) - cur - 1;
        cur = name.find('.', cur + 1);
    }
    name.back() = 0;
}
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc < 2)
    {
        return -fprintf(stderr, "usage: %s ... <port>\n", argv[0]);
    }

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(strtol(argv[argc - 1], NULL, 0));

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        err_quit("socket");

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        err_quit("bind");

    init("config.txt");

    uint8_t response[512];
    bzero(&response, sizeof(response));
    while (1)
    {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        ssize_t rlen = recvfrom(sockfd, response, 512, 0, (struct sockaddr *)&cliaddr, &clilen);
        uint8_t len = 0;

        header_t *head = (header_t *)response;
        head->flags = htons(0x8580);

        uint8_t *walk_ptr = (uint8_t *)(response + sizeof(header_t));
        for (int i = 0; i < ntohs(head->QDCOUNT); i++)
        {
            uint8_t total = 0;
            uint8_t *field_length = walk_ptr;
            string res;
            while (*field_length != 0)
            {
                /* Restore the dot in the name and advance to next length */
                total += *field_length + 1;
                res.append((char *)(field_length + 1), *field_length);
                res += ".";
                // *field_length = '.';
                field_length = walk_ptr + total;
            }
            uint16_t qtype = *(uint16_t *)(field_length + 1);
            uint16_t qclass = *(uint16_t *)(field_length + 3);
            // exactly same name
            if (zones.find(res) != zones.end())
            { // zone contains in the master file
                vector<pkt_info> temp = zones[res].find_answer(res, ntohs(qtype), ntohs(qclass));
                head->NSCOUNT = htons(temp.size());
                head->ARCOUNT = 0;
                string cur_type = QTYPE_TABLE[ntohs(qtype)];
                if (cur_type == "A")
                { // return SOA
                    size_t packetlen = sizeof(header_t) + (res.length() + 1) + sizeof(qtype) + sizeof(qclass) + 10 + (res.length() + 1);
                    for (int j = 0; j < temp.size(); j++)
                    {
                        packetlen += ntohs(temp[j]._rdlength);
                        uint8_t *packet = new uint8_t[packetlen];
                        uint8_t *p = packet;
                        memcpy(p, head, sizeof(header_t));
                        p += sizeof(header_t);
                        to_format(res);
                        memcpy(p, res.c_str(), res.length());
                        p += res.length();
                        memcpy(p, &qtype, 2);
                        p += 2;
                        memcpy(p, &qclass, 2);
                        p += 2;
                        memcpy(p, res.c_str(), res.length());
                        p += res.length();
                        memcpy(p, &temp[j]._class, 2);
                        p += 2;
                        memcpy(p, &temp[j]._ttl, 4);
                        p += 4;
                        memcpy(p, &temp[j]._rdlength, 2);
                        p += 2;
                        string data = temp[j]._rdata.substr(0, temp[j]._rdata.find(','));
                        to_format(data);
                        memcpy(p, data.c_str(), data.length()); p += data.length();
                        data = temp[j]._rdata.substr(temp[j]._rdata.find(',')+1, temp[j]._rdata.rfind('.')-temp[j]._rdata.find(','));
                        to_format(data);
                        memcpy(p, data.c_str(), data.length()); p += data.length();
                        data = temp[j]._rdata.substr(temp[j]._rdata.rfind(',')+1);
                        uint32_t serial = strtoul(data.substr(0, data.find(' ')).c_str(), nullptr, 10);
                        data = data.substr(data.find(' '));
                        uint32_t refresh = strtoul(data.substr(0, data.find(' ')).c_str(), nullptr, 10);
                        data = data.substr(data.find(' '));
                        uint32_t retry = strtoul(data.substr(0, data.find(' ')).c_str(), nullptr, 10);
                        data = data.substr(data.find(' '));
                        uint32_t expire = strtoul(data.substr(0, data.find(' ')).c_str(), nullptr, 10);
                        data = data.substr(data.find(' '));
                        uint32_t minimum = strtoul(data.c_str(), nullptr, 10);
                        memcpy(p, &serial, 4); p += 4;
                        memcpy(p, &refresh, 4); p += 4;
                        memcpy(p, &retry, 4); p += 4;
                        memcpy(p, &expire, 4); p += 4;
                        memcpy(p, &minimum, 4); p += 4;
                        for (int k = 0; k < packetlen; k++)
                        {
                            printf("0x%02x ", packet[k]);
                        }
                        cout << p - packet << endl;
                        sendto(sockfd, packet, packetlen, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
                        delete packet;
                    }
                }
            }
            // postfix fit

            // outside name
        }
    }
    close(sockfd);
}