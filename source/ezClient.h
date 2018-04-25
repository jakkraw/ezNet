#pragma once
#include "ezNetwork.h"
#include "msgInterface.h"
#include "sockets.h"
using namespace ezNetwork;
using namespace ezNetwork::internal;


struct ezClient : private MsgInterface, public Client {
	Connection connection;

	ezClient(Address address) : connection(createClientSocket(address))
	{
		
	}

};