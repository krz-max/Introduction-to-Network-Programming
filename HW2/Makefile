CC = g++
CFLAG = -Wall
PORT = 6000
PATH_TO_CONFIG = ./config.txt
all:
	${CC} ${CFLAG} HW2.cpp -o dns

.PHONY: clean
clean:
	rm *.out
test:
	./dns ${PORT} ${PATH_TO_CONFIG}