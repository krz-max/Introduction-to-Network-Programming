
lab3-1: lab03_1.cpp ../Header/MySocket.h
	g++ lab03_1.cpp -I ../Header -Wall -o lab03_1.out
demo3-1: lab03_1.out
	./lab03_1.out inp111.zoolab.org 10002
lab3-2: lab03_2.cpp ../Header/MySocket.h
	g++ lab03_2.cpp -I ../Header -Wall -o lab03_2.out
test3-2: lab03_2.out
	timeout 20 ./lab03_2.out 1;
demo3-2: lab03_2.out demo.sh
	./demo.sh

.PHONY: clean
clean:
	rm *.out