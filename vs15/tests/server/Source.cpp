#include "../messages.h"
#include "../../../source/interface/ezNetwork.h"
#pragma comment(lib,"ezNetwork.lib")
using namespace ezMultiplayer;

int main() {

	Server server;
	auto greet = Greet();
	server.send(greet);
	greet.i++;
	server.send(greet);
	greet.i++;
	server.send(greet);

	auto greets = server.recieve<Greet>();

	while(true){}

	return 0;
}