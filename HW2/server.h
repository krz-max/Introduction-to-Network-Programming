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
    bool has_subzone;

    R_soa _SOA; // 1 SOA per zone
    R_cname CNAME;
    std::list<R_mx> _MX;
    std::list<R_str> str_type; // A, AAAA, NS, TXT
    std::vector<short> with_type; // a bit vector to indicate the node has what types of answers
    Zone(){ with_type = vector<short>(7, 0); }
    void print_info()          // only print RR records in this node and whether it has subzones
    {
        printf("Zone_name: %s\n", zone_name.c_str());
        if(! str_type.empty()){
            for(auto it: str_type){
                it.print_ans_info();
            }
        }
        printf("\n");
        if(! _MX.empty()){
            for(auto it: _MX){
                it.print_ans_info();
            }
        }
        printf("\n");
        if(_SOA._length != 0) this->_SOA.print_ans_info();
        if(CNAME._length != 0) this->CNAME.print_ans_info(); // ???
        if (has_subzone)
            printf("There are subzone(s) in this zone\n");
        else
            printf("This is the leaf node in current zone\n");
    }
    void update_info(int ttl, uint16_t _class, uint16_t Qtype, std::string rdata)
    {
        // assume class always 1
        switch (Qtype)
        {
        case TypeID::TYPE_A:
            this->str_type.push_back(R_str(zone_name, Qtype, _class, ttl, TypeID::TYPE_A, rdata));
            this->str_type.back()._length += 4;
            with_type[0] = 1;
            break;
        case TypeID::TYPE_AAAA:
            this->str_type.push_back(R_str(zone_name, Qtype, _class, ttl, TypeID::TYPE_AAAA, rdata));
            this->str_type.back()._length += 16;
            with_type[1] = 1;
            break;
        case TypeID::TYPE_NS:
            this->str_type.push_back(R_str(zone_name, Qtype, _class, ttl, TypeID::TYPE_NS, rdata));
            this->str_type.back()._length += rdata.length();
            with_type[2] = 1;
            break;
        case TypeID::TYPE_TXT:
            this->str_type.push_back(R_str(zone_name, Qtype, _class, ttl, TypeID::TYPE_TXT, rdata));
            this->str_type.back()._length += rdata.length();
            with_type[3] = 1;
            break;
        case TypeID::TYPE_CNAME:
            this->_SOA.d_name = zone_name;
            this->_SOA.Qtype = Qtype;
            this->_SOA._class = _class;
            this->_SOA._ttl = ttl;
            this->_SOA._length += (rdata.length());
            this->CNAME.name_list.push_back(rdata);
            with_type[4] = 1;
            break;
        case TypeID::TYPE_MX:
            this->_MX.push_back(R_mx(zone_name, Qtype, _class, ttl,\
                                strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10),\
                                rdata.substr(rdata.find(' ') + 1)));
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_MX.back()._length += (rdata.length()) + 2;
            with_type[5] = 1;
            break;
        case TypeID::TYPE_SOA:
            this->_SOA.d_name = zone_name;
            this->_SOA.Qtype = Qtype;
            this->_SOA._class = _class;
            this->_SOA._ttl = ttl;
            // 2 for label length and 10 for other uint32_t
            this->_SOA._length = rdata.rfind('.')+2+10;
            this->_SOA.m_name = rdata.substr(0, rdata.find(' '));
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.r_name = rdata.substr(0, rdata.find(' '));
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.serial = strtoul(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.refresh = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.retry = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.expire = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
            rdata = rdata.substr(rdata.find(' ')+1);
            this->_SOA.minimum = strtoul(rdata.c_str(), nullptr, 10);
            with_type[6] = 1;
            break;
        default:
            printf("Wrong format\n");
            break;
        }
    }
    void has_type(const string &name, const uint16_t &Qtype, const uint16_t &_class, std::vector<int> &res){
        switch (Qtype)
        {
        case TypeID::TYPE_A:
            if(name == zone_name && with_type[0] == 1){
                res[0] = 1;
                return ;
            }
            if(name == zone_name && with_type[6] == 1){
                res[6] = 1;
                return ;
            }
            break;
        case TypeID::TYPE_AAAA:
            break;
        case TypeID::TYPE_NS:
            break;
        case TypeID::TYPE_TXT:
            break;
        case TypeID::TYPE_CNAME:
            break;
        case TypeID::TYPE_MX:
            break;
        case TypeID::TYPE_SOA:
            // return whether this node has SOA RR
            res[6] = with_type[6];
            break;
        default:
            printf("Wrong format\n");
            break;
        }
    }
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
    void start_query(const string&, const uint16_t&, const uint16_t&, uint16_t&, uint16_t&, uint16_t&);
    struct sockaddr_in servaddr;
    socklen_t servlen;
    int sockfd;

private:
    std::string forward_ip;
    std::unordered_map<std::string, Zone> zones;
};