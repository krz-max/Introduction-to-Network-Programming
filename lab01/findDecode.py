
'''
10:33:29.053718 IP (tos 0x0, ttl 64, id 31003, offset 0, flags [DF], proto UDP (17), length 63)
    172.28.17.144.42913 > 172.28.16.1.53: UDP, length 35
E..?y.@.@.G............5.+z..............inp111.zoolab.org.....

10:33:33.581012 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 172.28.17.144 (00:15:5d:3c:19:74) tell 172.28.16.1, length 28
..........]3.'......]<.t....
'''
# tcpdump -v -qns 0 -A -r test.pcap > a.txt
import base64
import sys

def extractTTL(info):
    TTL = int(info[info.find('ttl')+4:info.find(',', info.find('ttl'))])
    return TTL
   
def extractTOS(info):
    TOS = hex( int( info[ info.find('tos')+4 : info.find(',', info.find('tos')) ] , 16) )
    return TOS

def extractDataLen(direction):
    datalen = int( direction[ direction.find('tcp') + 4 :])
    return datalen

def main(filename):
    with open(filename) as f:
        packet = {}
        temp = f.readline()
        while 1:
            line = temp
            if not line: # read eof
                break
            if 'IP (tos' in line:
                info = line
                # print(info, end="")
                TOS = extractTOS(info)
                TTL = extractTTL(info)
                direction = f.readline()
                if '140.113.213.213.10001 > 172.28.17.144.51984' in direction: # packet received from zoolab
                    data = ""
                    datalen = extractDataLen(direction)
                    while 1:
                        temp = f.readline()
                        # print(temp, end="")
                        if not temp:
                            break
                        # print(temp, end="")
                        if 'IP (tos' in temp:
                            break
                        else:
                            data += temp
                    if TTL in packet: # this is not unique
                        packet[TTL] = ""
                    else:
                        packet[TTL] = (datalen, data[len(data)-datalen-1:])
                else: # skip data
                    while 1:
                        temp = f.readline()
                        if 'IP (tos' in temp:
                            break
                        if not temp:
                            break
            else:
                while 1:
                    temp = f.readline()
                    if 'IP (tos' in temp:
                        break
        f.close()
        print(packet)
        for x in packet.values():
            if not x == "":
                print(base64.b64decode(x[1]))
                break
    return 0
                           
        
    
if __name__ == "__main__":
    main(sys.argv[1])