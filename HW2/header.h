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


static std::unordered_map<uint16_t, std::string> QTYPE_TABLE = \
{{1, "A"}, {2, "NS"}, {5, "CNAME"}, {6, "SOA"}, {15, "MX"}, {16, "TXT"}, {7, "AAAA"}};
static std::unordered_map<uint16_t, std::string> QCLASS_TABLE = \
{{1, "IN"}, {2, "CS"}, {3, "CH"}, {4, "HS"}};


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
struct zone
{
    std::string zone_name;
    std::string A;
    std::string AAAA;
    std::string NS;
    std::string CNAME;
    std::string SOA;
    std::string MX;
    std::string TXT;
    void print_info(){
        std::cout << "zone_name: " << zone_name << std::endl;
        std::cout << "A: " << A << std::endl;
        std::cout << "AAAA: " << AAAA << std::endl;
        std::cout << "NS: " << NS << std::endl;
        std::cout << "CNAME: " << CNAME << std::endl;
        std::cout << "SOA: " << SOA << std::endl;
        std::cout << "MX: " << MX << std::endl;
        std::cout << "TXT: " << TXT << std::endl;
    }
};