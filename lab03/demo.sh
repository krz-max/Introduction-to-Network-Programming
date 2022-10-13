#!/bin/sh
timeout 20 ./lab03_2.out 1;   sleep 5  # send at 1 MBps
timeout 20 ./lab03_2.out 1.5; sleep 5  # send at 1.5 MBps
timeout 20 ./lab03_2.out 2;   sleep 5  # send at 2 MBps
timeout 20 ./lab03_2.out 3; sleep 5
