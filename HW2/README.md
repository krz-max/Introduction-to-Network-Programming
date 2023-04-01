## HW2 : Your own dns server - RFC 1035
* This homework is a DNS server that can correctly respond when users ask for a DNS record of a particular domain name. For more details about the domain system and protocol, please refer to [RFC 1035](https://www.rfc-editor.org/rfc/rfc1035).
* First, the DNS server has to read a configuration file containing the configuration of the DNS server.
* Your DNS server should respond to a query if a queried domain is handled by your server (based on the configuration).
* For domains not handled by your server, your DNS server has to forward the request to the configured foreign server.
* Note: You only need to handle protocol messages delivered in UDP.
* In this homework, you do not need to consider the situation when OPCODE is not 0.
* The required types in this homework and their meaning are listed below:
```
A     : a host address in IPv4
AAAA  : a host address in IPv6
NS    : an authoritative name server 
CNAME : the canonical name for an alias
SOA   : marks the start of a zone of authority
MX    : mail exchange
TXT   : text strings 
```
### Configuration Files
* The format of the configuration file of this homework is defined as follows.
```
<forwardIP>
<domain 1>,<path of zone file 1>
<domain 2>,<path of zone file 2>
...
```
### Zone Files
* The format of a zone file containing records of a domain is defined as follows.
```
<domain>
<NAME>,<TTL>,<CLASS>,<TYPE>,<RDATA>
<NAME>,<TTL>,<CLASS>,<TYPE>,<RDATA>
...
```
* The format of <RDATA> for each RR type is summarized below.
```
A     : <ADDRESS>
AAAA  : <ADDRESS>
NS    : <NSDNAME>
CNAME : <CNAME>
SOA   : <MNAME> <RNAME> <SERIAL> <REFRESH> <RETRY> <EXPIRE> <MINIMUM>
MX    : <PREFERENCE> <EXCHANGE>
TXT   : <TXT-DATA>
```
### Example
* Run your server :
```console
./dns 1053 config.txt
```
* Below are the list of commands you could use to test the server (type them on another terminal to make dns query)
```
dig @127.0.0.1 -p 1053 example1.org
dig @127.0.0.1 -p 1053 example1.org ns
dig @127.0.0.1 -p 1053 example1.org mx
dig @127.0.0.1 -p 1053 www.example1.org

dig @127.0.0.1 -p 1053 yahoo.com A
dig @127.0.0.1 -p 1053 yahoo.com AAAA
dig @127.0.0.1 -p 1053 yahoo.com ns
dig @127.0.0.1 -p 1053 www.nycu.edu.tw cname
dig @127.0.0.1 -p 1053 yahoo.com soa
dig @127.0.0.1 -p 1053 gmail.com mx
dig @127.0.0.1 -p 1053 gmail.com txt
```
