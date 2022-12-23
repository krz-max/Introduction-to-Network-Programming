#include "header.h"

namespace TypeString
{
    const std::string _A_ = "A";
    const std::string _NS_ = "NS";
    const std::string _CNAME_ = "CNAME";
    const std::string _SOA_ = "SOA";
    const std::string _MX_ = "MX";
    const std::string _TXT_ = "TXT";
    const std::string _AAAA_ = "AAAA";
    const std::string _IN_ = "IN";
};
namespace TypeID
{
    const int TYPE_A = 1;
    const int TYPE_NS = 2;
    const int TYPE_CNAME = 5;
    const int TYPE_SOA = 6;
    const int TYPE_MX = 15;
    const int TYPE_TXT = 16;
    const int TYPE_AAAA = 28;
    const int CLASS_IN = 1;
};
static std::unordered_map<uint16_t, std::string> QTYPE_TABLE =
    {{1, "A"},
     {2, "NS"},
     {5, "CNAME"},
     {6, "SOA"},
     {15, "MX"},
     {16, "TXT"},
     {28, "AAAA"}};
static std::unordered_map<std::string, uint16_t> R_QTYPE_TABLE =
    {{"A", 1},
     {"NS", 2},
     {"CNAME", 5},
     {"SOA", 6},
     {"MX", 15},
     {"TXT", 16},
     {"AAAA", 28}};
static std::unordered_map<uint16_t, std::string> QCLASS_TABLE =
    {{1, "IN"}};
static std::unordered_map<std::string, uint16_t> R_QCLASS_TABLE =
    {{"IN", 1}};
struct header
{
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};
struct question
{
    std::string qname;
    uint16_t qtype;
    uint16_t qclass;
};
struct rr
{
    std::string d_name = "";
    uint16_t Qtype = 0;
    uint16_t _class = 0;
    uint32_t _ttl = 0;
    uint16_t _length = 0;
    rr(){}
    rr(std::string dname, uint16_t type, uint16_t _class, uint32_t ttl)\
        : d_name(dname), Qtype(type), _class(_class), _ttl(ttl) {} ;
    void print_info(){
        printf("RR field: \ndname: %s, Qtype: %s, Class: %s, TTL: %u, Len: %u\n",\
            d_name.c_str(), QTYPE_TABLE[Qtype].c_str(), QCLASS_TABLE[_class].c_str(), _ttl, _length);
    }
};

struct R_soa : public rr
{
    std::string m_name;
    std::string r_name;
    uint32_t serial;
    int refresh;
    int retry;
    int expire;
    uint32_t minimum;
    void print_ans_info(){
        print_info();
        printf("SOA ans field: \nm_name: %s, r_name: %s, ser: %u, ref: %d, ret: %d, exp: %d, min: %u\n",\
            m_name.c_str(), r_name.c_str(), serial, refresh, retry, expire, minimum);
    }
};
struct R_cname : public rr
{
    std::list<std::string> name_list;
    void print_ans_info(){
        print_info();
        printf("Cnames are: \n");
        for(auto it:name_list){
            printf("%s, ", it.c_str());
        }
        printf("\n");
    }
};
struct R_mx : public rr
{
    int16_t preference;
    std::string exchange;
    R_mx(std::string dname, uint16_t Qtype, uint16_t _class,\
        uint32_t ttl, int16_t pref, std::string str)\
        : rr(dname, Qtype, _class, ttl), preference(pref), exchange(str) { this->_length = 0; }
    void print_ans_info(){
        print_info();
        printf("MX ans field: \npref: %d, exg: %s\n",\
            preference, exchange.c_str());
    }
};
struct R_str : public rr
{
    uint16_t rr_type;
    std::string info;
    R_str(std::string dname, uint16_t Qtype, uint16_t _class,\
        uint32_t ttl, uint16_t rr_type, std::string str)\
        : rr(dname, Qtype, _class, ttl), rr_type(rr_type), info(str) { this->_length = 0; }
    void print_ans_info(){
        print_info();
        printf("Type %s ans field: \nstr: %s\n", QTYPE_TABLE[rr_type].c_str(), info.c_str());
    }
};
// struct R_ns : public rr
// {
//     std::string nsname;
// };
// struct R_a : public rr
// {
//     std::string addr;
// };
// struct R_aaaa : public rr
// {
//     std::string addr;
// };

struct Zone
{
    std::string zone_name;
    std::list<std::string> subzones;
    bool has_subzone;

    R_soa _SOA; // 1 SOA per zone
    R_cname CNAME;
    std::list<R_mx> _MX;
    std::list<R_str> str_type; // A, AAAA, NS, TXT
    std::vector<short> with_type; // a bit vector to indicate the node has what types of answers
    Zone(){ with_type = std::vector<short>(7, 0); }
    void print_info();          // only print RR records in this node and whether it has subzones
    void update_info(int, uint16_t, uint16_t, std::string);
    uint8_t* append_rr(uint8_t*, const uint16_t&, const uint16_t&, const uint32_t&, const uint16_t&);
    uint8_t* get_answer(uint8_t*, const uint16_t&, const uint16_t& , uint16_t&);
    uint8_t* get_additional(uint8_t*, const uint16_t&, const uint16_t& , uint16_t&);
    uint8_t* get_authority(uint8_t*, const uint16_t&, const uint16_t& , uint16_t&);
};
// udp server
class dns
{
public:
    dns(const char *config_file, in_port_t port);
    ~dns();
    void setup();
    void start();
    void summary();
    uint8_t* start_query(uint8_t* , const std::string&, const uint16_t&, const uint16_t&, uint16_t&, uint16_t&, uint16_t&);
    struct sockaddr_in servaddr;
    socklen_t servlen;
    int sockfd;

private:
    std::string forward_ip;
    std::unordered_map<std::string, Zone> zones;
};