#pragma once
#include "connection.h"

namespace lan {
	using ConnectionPtr =  std::unique_ptr<lan::Connection>;

	struct ServerInfo {
		lan::Address address;

		ServerInfo(lan::Address& a) : address(a) {}

		lan::ConnectionPtr connect() {
			return std::make_unique<lan::Connection>(address);
		}

	};
}

