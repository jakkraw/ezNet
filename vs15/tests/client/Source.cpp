#include "../../../source/ezNetwork.h"
#include "../messages.h"

using namespace ezNetwork;

int main() {

	auto client = createClient();

	client->send(Greet());

	std::vector<Greet> greets;
	do {
		greets = client->recieve<Greet>();
	} while (greets.empty());

	
	return 0;
}