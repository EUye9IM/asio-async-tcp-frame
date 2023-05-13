#include "common.hpp"
#include <iostream>
#include <server_udp.h>
using namespace std;
using asio::ip::udp;

int main() {
	try {
		ServerUDP server(ip, port);
		server.onError = [](const string &message) {
			cout << "Error: " << message << endl;
		};
		server.onMessage = [&server](asio::ip::udp::endpoint remote_addr,
									 PakHeadData type, size_t length,
									 const void *content) {
			cout << "Message: " << remote_addr << ": "
				 << string(reinterpret_cast<const char *>(content),
						   reinterpret_cast<const char *>(content) + length)
				 << endl;
		};
		server.run();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}