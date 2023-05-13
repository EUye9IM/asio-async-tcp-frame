#include "common.hpp"
#include <iostream>
#include <server_tcp.h>
using namespace std;
using asio::ip::tcp;

int main() {
	try {
		Server server(ip, port);
		server.onConnect = [&server](Session *session,
									 const tcp::socket &socket) {
			cout << "Connect: 0x" << hex << session << endl;
			const char *info = "Connect success! You can type 'exit' if you "
							   "want to close connection.";
			server.write(session, PakHeadData{}, strlen(info), info);
		};
		server.onDisconnect = [](Session *session) {
			cout << "Disonnect: 0x" << hex << session << endl;
		};
		server.onError = [](Session *session, const string &message) {
			cout << "Error: 0x" << hex << session << ": " << message << endl;
		};
		server.onMessage = [&server](Session *session, PakHeadData type,
									 size_t length, const void *content) {
			cout << "Message: 0x" << hex << session << ": "
				 << string(reinterpret_cast<const char *>(content),
						   reinterpret_cast<const char *>(content) + length)
				 << endl;
			server.write(session, PakHeadData{}, length, content);
		};
		server.run();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}