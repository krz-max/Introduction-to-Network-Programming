
lab4-1: lab04-1.cpp ../Header/MySocket.h
	g++ lab04-1.cpp -I ../Header -Wall -o nkat.out
demo4-1: nkat.out
	./nkat.out 10001 `which date`
demo4-2: nkat.out
	./nkat.out 10002 date
demo4-3: nkat.out
	./nkat.out 10003 /path/to/an/invalid/executable
demo4-4: nkat.out
	./nkat.out 10004 ls -la /tmp /home /xxx
demo4-5: nkat.out
	./nkat.out 10005 timeout 15 /bin/bash -c 'cd /; exec bash'

.PHONY: clean
clean:
	rm *.out