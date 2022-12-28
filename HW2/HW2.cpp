#include "server.cpp"


int main(int argc, char** argv){
	dns Myserver("config.txt", htons(strtol(argv[1], nullptr, 10)));
	// Myserver.summary();
	Myserver.start();
	return 0;
}