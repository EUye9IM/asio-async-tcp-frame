#include "common.hpp"
#include <client_udp.h>
#include <iostream>
#include <server_udp.h>
using namespace std;

int main() {
	try {
		ServerUDP server(ip, port + 1);
		server.onError = [](const string &message) {
			cout << "Error: " << message << endl;
		};
		cout << "Connect success! You can type 'exit' if you want "
				"to close connection."
			 << endl
			 << "  > ";
		cout.flush();
		string s;
		cin >> s;
		if (s == "exit") {
			server.stop();
			return 0;
		} else {
			server.write(asio::ip::udp::endpoint(
							 asio::ip::address::from_string(ip), port),
						 PakHeadData{}, s.length(), s.c_str());
		}
		server.onMessage = [&server](asio::ip::udp::endpoint remote_addr,
									 PakHeadData type, size_t length,
									 const void *content) {
			cout << string(reinterpret_cast<const char *>(content),
						   reinterpret_cast<const char *>(content) + length)
				 << endl
				 << "  > ";
			cout.flush();
			string s;
			cin >> s;
			if (s == "exit") {
				server.stop();
			} else {
				remote_addr.port(port);
				server.write(remote_addr, PakHeadData{}, s.length(), s.c_str());
			}
		};
		server.run();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}