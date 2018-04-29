#include "../../../source/interface/ezNetwork.h"
#include "../messages.h"
#pragma comment(lib,"ezNetwork")

using namespace ezm;

int main() {
	Client client;
	client.send(Greet());

	std::vector<Greet> greets;
	do greets = client.recieve<Greet>();
	while (greets.empty());
}