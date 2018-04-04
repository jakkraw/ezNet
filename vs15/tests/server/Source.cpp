#include "../../../source/ezNetwork.h"
#include "../messages.h"

using namespace ezNetwork;

int main() {

	auto server = createServer();
	auto& s = *server;

	auto greet = Greet();

	s.send(greet);

	greet.i++;
	s.send(greet);
	s.send(greet);

	auto greets = s.recieve<Greet>();

	return 0;
}