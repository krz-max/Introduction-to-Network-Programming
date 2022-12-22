#include "src/server.cpp"


int main(int argc, char** argv){
	Server Myserver(htons(strtol(argv[1], NULL, 10)), htonl(INADDR_ANY));
	Myserver.setup();
	Myserver.start();
	return 0;
}