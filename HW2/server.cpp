#include "server.h"
#include "socketwrapper.cpp"
#define NIPQUAD(s) ((unsigned char *)&s)[0], \
                   ((unsigned char *)&s)[1], \
                   ((unsigned char *)&s)[2], \
                   ((unsigned char *)&s)[3]
using namespace std;

string &rtrim(string &s)
{
    if (s.empty())
        return s;

    s.erase(s.find_last_not_of(" \n\r") + 1);
    return s;
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

dns::dns(const char *config_file, in_port_t port)
{
    ifstream f_in(config_file);
    getline(f_in, this->forward_ip);
    string temp;
    while (getline(f_in, temp))
    {
        temp = rtrim(temp);
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
            if (!isprint(temp.back()))
                temp.pop_back();
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
                zones[root].subzones.push_back(cur_name);
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

uint8_t *copy_string(uint8_t *walk_ptr, const string &char_string, const bool domain_name)
{
    if (domain_name) // format : FOO.ISMA.ARPA.OFD.ORG.
    {
        // add a '.' in front of walk_ptr
        *walk_ptr = char_string.find('.');
        walk_ptr++;
        size_t i = 0;
        for (i = 0; i < char_string.length() - 1; i++, walk_ptr++)
        {
            if (char_string[i] == '.')
            {
                *walk_ptr = char_string.find('.', i + 1) - i - 1;
            }
            else
            {
                *walk_ptr = (uint8_t)char_string[i];
            }
        }
        // trailing zero
        *walk_ptr = 0;
        walk_ptr++;
        return walk_ptr;
    }
    // simple character string
    // add a length octet in front of the message, only 8 bits, so length should be limited to 256 bytes
    // unlike <domain-name>, there are no trailing zeros at the end of the string
    *walk_ptr = char_string.length();
    walk_ptr++;
    size_t i = 0;
    for (i = 0; i < char_string.length(); i++, walk_ptr++)
        *walk_ptr = (uint8_t)char_string[i];
    return walk_ptr;
}

void parse_header(uint8_t *recvline, string &query_name, uint16_t &qtype, uint16_t &qclass)
{
    // struct header *hdrptr = (struct header *)recvline;
    uint8_t *start_of_name = recvline + sizeof(header);
    uint8_t *end_of_name = to_format(start_of_name, query_name);
    qtype = ntohs(*(uint16_t *)(end_of_name + 1));
    qclass = ntohs(*(uint16_t *)(end_of_name + 3));
}

void make_header(uint8_t *recvline, const uint16_t &ancount, const uint16_t &nscount, const uint16_t &arcount)
{
    struct header *hdptr = (struct header *)recvline;
    hdptr->id = hdptr->id;
    hdptr->flags = htons(0x8580);
    hdptr->qdcount = hdptr->qdcount;
    hdptr->ancount = htons(ancount);
    hdptr->arcount = htons(arcount);
    hdptr->nscount = htons(nscount);
}

// querying for this domain : SOA in authority section
// querying for subdomain : ns in authority section (dns.example1.org)
uint8_t *dns::start_query(uint8_t *recvline, const string &name, const uint16_t &qtype, const uint16_t &qclass, const string &root_zone, uint16_t &ancount, uint16_t &nscount, uint16_t &arcount)
{
    // check all answers in this node
    auto node = zones.find(root_zone);
    if (node == zones.end())
    {
        cout << "Query send to forward ip" << endl;
        // should send "name" not "root_zone" because root_zone is not necessarily equal to query name
        return recvline;
    }
    // Answer Section
    uint8_t *walk_ptr = (recvline + sizeof(header) + (name.length() + 1) + 2 + 2); // header + <domain-name> + QTYPE + QCLASS
    // just make sure (start_of_ans - recvline) < 512
    string additional_info;
    // use query name to find answer
    node = zones.find(name);
    if(node != zones.end()){
        walk_ptr = node->second.get_answer(walk_ptr, qtype, qclass, ancount, additional_info);
    }

    // authority
    node = zones.find(root_zone);
    // no authority reply on Type NS query
    if (qtype != TypeID::TYPE_NS)
    {
        string found_name = name;
        if(!additional_info.empty()){
            found_name = additional_info.substr(additional_info.find(':')+2);
        }
        if (found_name == root_zone)
        {
            walk_ptr = node->second.get_authority(walk_ptr, TypeID::TYPE_SOA, qclass, arcount);
        }
        else
        {
            walk_ptr = node->second.get_authority(walk_ptr, TypeID::TYPE_NS, qclass, arcount);
        }
    }
    // additional
    if (ancount > 0 && (qtype == TypeID::TYPE_NS || qtype == TypeID::TYPE_MX))
    {
        string additional_domain_name = additional_info.substr(additional_info.find(':') + 2);
        node = zones.find(additional_domain_name); // should exist
        walk_ptr = node->second.get_additional(walk_ptr, TypeID::TYPE_A, qclass, nscount);
    }
    return walk_ptr;
}

string dns::start_from_root(const string &query_name)
{
    string probe = query_name.substr(query_name.rfind('.'));
    while (zones.find(probe) == zones.end())
    {
        probe = query_name.substr(query_name.rfind('.', query_name.length() - probe.length() - 2) + 1);
        if (probe == "")
            break;
    }
    return probe;
}

uint8_t* dns::do_forward_query(uint8_t *recvline, size_t &sz, const string &query_name)
{
    int s = -1;
    struct sockaddr_in SAddr;
    socklen_t SLen;
	bzero(&SAddr, sizeof(SAddr));
	SAddr.sin_family = AF_INET;
	SAddr.sin_port = htons(53);
    SAddr.sin_addr.s_addr = inet_addr(forward_ip.c_str());
    s = Socket(AF_INET, SOCK_DGRAM, 0);
    struct header *hdptr = (struct header *)recvline;
    hdptr->flags = htons(0x0180);

    hexdump(recvline, sz);
    SLen = sizeof(SAddr);
    sendto(s, recvline, sz, 0, (struct SA *)&SAddr, SLen);
    sz = recvfrom(s, recvline, 512, 0, (struct SA *)&SAddr, &SLen);
    hexdump(recvline, sz);
    return recvline;
}

void dns::start()
{
    size_t sz;
    uint8_t recvline[512];
    bzero(recvline, sizeof(recvline));
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    while ((sz = recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct SA *)&cliaddr, &clilen)) > 0)
    {
        string query_zone;
        uint16_t qtype, qclass, ancount = 0, nscount = 0, arcount = 0;
        uint8_t *walk_ptr = recvline;
        parse_header(walk_ptr, query_zone, qtype, qclass);
        string root_zone = start_from_root(query_zone);
        cout << query_zone << " " << root_zone << endl;
        walk_ptr = start_query(walk_ptr, query_zone, qtype, qclass, root_zone, ancount, nscount, arcount);
        if (walk_ptr == recvline)
        {
            walk_ptr = do_forward_query(recvline, sz, query_zone);
            sendto(sockfd, walk_ptr, sz, 0, (struct SA *)&cliaddr, clilen);
            bzero(recvline, sizeof(recvline));
            continue;
            // forward ip, not implemented yet
        }
        make_header(recvline, ancount, nscount, arcount);
        sz = (walk_ptr - recvline);
        hexdump(recvline, sz);
        sendto(sockfd, recvline, sz, 0, (struct SA *)&cliaddr, clilen);
        bzero(recvline, sizeof(recvline));
    }
}

uint8_t *Zone::append_rr(uint8_t *walk_ptr, const uint16_t &TYPE, const uint16_t &CLASS, const uint32_t &TTL, const uint16_t &RDLENGTH)
{
    walk_ptr = copy_string(walk_ptr, zone_name, true);
    *(uint16_t *)walk_ptr = htons(TYPE);
    walk_ptr += 2;
    *(uint16_t *)walk_ptr = htons(CLASS);
    walk_ptr += 2;
    *(uint32_t *)walk_ptr = htonl(TTL);
    walk_ptr += 4;
    *(uint16_t *)walk_ptr = htons(RDLENGTH);
    walk_ptr += 2;
    return walk_ptr;
}

uint8_t *Zone::get_additional(uint8_t *walk_ptr, const uint16_t &qtype, const uint16_t &_class, uint16_t &nscount)
{
    if (this->with_type[0])
    {
        // add answer to recvline;
        auto it = str_type.begin();
        for (; it != str_type.end(); it++)
        {
            if (it->Qtype == TypeID::TYPE_A)
            {
                walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                uint32_t temp;
                Inet_pton(AF_INET, it->info.c_str(), &temp);
                *(uint32_t *)walk_ptr = temp; // not sure
                walk_ptr += 4;
                nscount++;
            }
        }
    }
    return walk_ptr;
}

uint8_t *Zone::get_authority(uint8_t *walk_ptr, const uint16_t &qtype, const uint16_t &_class, uint16_t &arcount)
{
    if (qtype == TypeID::TYPE_SOA)
    {
        if (this->with_type[6])
        {
            walk_ptr = this->append_rr(walk_ptr, qtype, _class, this->_SOA._ttl, this->_SOA._length);
            walk_ptr = copy_string(walk_ptr, this->_SOA.m_name, true);
            walk_ptr = copy_string(walk_ptr, this->_SOA.r_name, true);
            *(uint32_t *)walk_ptr = htonl(this->_SOA.serial);
            walk_ptr += 4;
            *(int *)walk_ptr = htonl(this->_SOA.refresh);
            walk_ptr += 4;
            *(int *)walk_ptr = htonl(this->_SOA.retry);
            walk_ptr += 4;
            *(int *)walk_ptr = htonl(this->_SOA.expire);
            walk_ptr += 4;
            *(uint32_t *)walk_ptr = htonl(this->_SOA.minimum);
            walk_ptr += 4;
            arcount++;
        }
    }
    else if (qtype == TypeID::TYPE_NS)
    {
        if (this->with_type[2])
        {
            // add answer to recvline;
            auto it = str_type.begin();
            for (; it != str_type.end(); it++)
            {
                if (it->Qtype == TypeID::TYPE_NS)
                {
                    walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                    walk_ptr = copy_string(walk_ptr, it->info, true);
                    arcount++;
                }
            }
        }
    }
    return walk_ptr;
}

uint8_t *Zone::get_answer(uint8_t *walk_ptr, const uint16_t &qtype, const uint16_t &_class, uint16_t &ancount, string &additional_info)
{
    if (qtype == TypeID::TYPE_A)
    {
        if (this->with_type[0])
        {
            // add answer to recvline;
            auto it = str_type.begin();
            for (; it != str_type.end(); it++)
            {
                if (it->Qtype == TypeID::TYPE_A)
                {
                    walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                    uint32_t temp;
                    Inet_pton(AF_INET, it->info.c_str(), &temp);
                    *(uint32_t *)walk_ptr = temp; // not sure
                    walk_ptr += 4;
                    ancount++;
                }
            }
        }
    }
    else if (qtype == TypeID::TYPE_AAAA)
    {
        if (this->with_type[1])
        {
            // add answer to recvline;
            auto it = str_type.begin();
            for (; it != str_type.end(); it++)
            {
                if (it->Qtype == TypeID::TYPE_AAAA)
                {
                    walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                    uint32_t temp;
                    Inet_pton(AF_INET6, it->info.c_str(), &temp);
                    *(uint32_t *)walk_ptr = temp; // not sure
                    walk_ptr += it->_length;
                    ancount++;
                }
            }
        }
    }
    else if (qtype == TypeID::TYPE_NS)
    {
        if (this->with_type[2])
        {
            // add answer to recvline;
            auto it = str_type.begin();
            for (; it != str_type.end(); it++)
            {
                if (it->Qtype == TypeID::TYPE_NS)
                {
                    walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                    walk_ptr = copy_string(walk_ptr, it->info, true);
                    additional_info += Prefix::TYPE_NS + ": " + it->info;
                    ancount++;
                }
            }
            // should get additional type A processing, not done yet
        }
    }
    else if (qtype == TypeID::TYPE_TXT)
    {
        if (this->with_type[3])
        {
            // add answer to recvline;
            auto it = this->str_type.begin();
            for (; it != this->str_type.end(); it++)
            {
                if (it->Qtype == TypeID::TYPE_TXT)
                {
                    walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                    walk_ptr = copy_string(walk_ptr, it->info, false);
                    ancount++;
                }
            }
        }
    }
    else if (qtype == TypeID::TYPE_CNAME)
    {
        if (this->with_type[4])
        {
            // "a" <domain-name>, so each answer should contributes to a rr
            // each <domain-name> should be zero terminated
            // maybe need compression ?
            auto it = this->CNAME.name_list.begin();
            for (; it != this->CNAME.name_list.end(); it++)
            {
                walk_ptr = this->append_rr(walk_ptr, qtype, _class, this->CNAME._ttl, this->CNAME._length);
                walk_ptr = copy_string(walk_ptr, (*it).c_str(), true);
                ancount++;
            }
        }
    }
    else if (qtype == TypeID::TYPE_MX)
    {
        if (this->with_type[5])
        {
            auto it = this->_MX.begin();
            // each MX record is an answer
            for (; it != this->_MX.end(); it++)
            {
                walk_ptr = this->append_rr(walk_ptr, qtype, _class, it->_ttl, it->_length);
                *(uint16_t *)walk_ptr = htons(it->preference);
                walk_ptr += 2;
                walk_ptr = copy_string(walk_ptr, it->exchange.c_str(), true);
                additional_info += Prefix::TYPE_MX + ": " + it->exchange;
                ancount++;
            }
        }
    }
    else if (qtype == TypeID::TYPE_SOA)
    {
        if (this->with_type[6])
        {
            ancount++;
            walk_ptr = this->append_rr(walk_ptr, qtype, _class, this->_SOA._ttl, this->_SOA._length);
        }
    }
    else
    {
        printf("Wrong format\n");
    }
    return walk_ptr;
}

string Zone::get_NS_name()
{
    auto it = this->str_type.begin();
    // each MX record is an answer
    for (; it != this->str_type.end(); it++)
        if (it->rr_type == TypeID::TYPE_NS)
            return it->info;
    return "";
}

void Zone::update_info(int ttl, uint16_t _class, uint16_t Qtype, std::string rdata)
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
        this->str_type.back()._length += rdata.length() + 1;
        with_type[2] = 1;
        break;
    case TypeID::TYPE_TXT:
        this->str_type.push_back(R_str(zone_name, Qtype, _class, ttl, TypeID::TYPE_TXT, rdata));
        this->str_type.back()._length += rdata.length() + 1;
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
        this->_MX.push_back(R_mx(zone_name, Qtype, _class, ttl,
                                 strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10),
                                 rdata.substr(rdata.find(' ') + 1)));
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_MX.back()._length += (rdata.length() + 1) + 2;
        with_type[5] = 1;
        break;
    case TypeID::TYPE_SOA:
        this->_SOA.d_name = zone_name;
        this->_SOA.Qtype = Qtype;
        this->_SOA._class = _class;
        this->_SOA._ttl = ttl;
        // 2 for label length and 10 for other uint32_t
        this->_SOA._length = rdata.rfind('.') + 2 + 20;
        this->_SOA.m_name = rdata.substr(0, rdata.find(' '));
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.r_name = rdata.substr(0, rdata.find(' '));
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.serial = strtoul(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.refresh = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.retry = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.expire = strtol(rdata.substr(0, rdata.find(' ')).c_str(), nullptr, 10);
        rdata = rdata.substr(rdata.find(' ') + 1);
        this->_SOA.minimum = strtoul(rdata.c_str(), nullptr, 10);
        with_type[6] = 1;
        break;
    default:
        printf("Wrong format\n");
        break;
    }
}

void Zone::print_info() // only print RR records in this node and whether it has subzones
{
    printf("Zone_name: %s\n", zone_name.c_str());
    if (!str_type.empty())
    {
        for (auto it : str_type)
        {
            it.print_ans_info();
        }
    }
    if (!_MX.empty())
    {
        for (auto it : _MX)
        {
            it.print_ans_info();
        }
    }
    if (_SOA._length != 0)
        this->_SOA.print_ans_info();
    if (CNAME._length != 0)
        this->CNAME.print_ans_info(); // ???
    if (has_subzone)
    {
        printf("There are subzone(s) in this zone: \n");
        for (auto it : subzones)
        {
            printf("%s, ", it.c_str());
        }
        printf("\n");
    }
    else
    {
        printf("This is the leaf node in current zone\n");
    }
    printf("\n");
    printf("\n");
}
