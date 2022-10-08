#!/bin/bash

a=test.pcap
b=a.txt
c=findDecode.py

tcpdump -v -qns 0 -A -r $a > $b
python3 $c $b
