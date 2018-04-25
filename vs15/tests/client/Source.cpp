#include "../../../source/ezNetwork.h"
#include "../messages.h"
#pragma comment(lib,"ezNetwork")
using namespace ezNetwork;

int main() {
	auto client = createClient();

	client->send(Greet());

	std::vector<Greet> greets;
	do greets = client->recieve<Greet>();
	while (greets.empty());
}