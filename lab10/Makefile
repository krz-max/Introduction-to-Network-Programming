
all: lab10srv.cpp lab10cli.cpp
	g++ -static -o server.out lab10srv.cpp
	g++ -static -o client.out lab10cli.cpp

submit: all
	python3 submit.py ./server.out ./client.out

.PHONY: clean
clean:
	rm *.out