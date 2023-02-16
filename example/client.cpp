#include "common.hpp"
#include <client.h>
#include <iostream>
using namespace std;
using asio::ip::tcp;

int main() {
	try {
		Client client(ip, port);
		client.onConnect = [](const tcp::socket &socket) {};
		client.onDisconnect = []() { cout << "Disonnect" << endl; };
		client.onError = [](const string &message) {
			cout << "Error: " << message << endl;
		};
		client.onMessage = [&client](PakHeadData type, size_t length,
									 const void *content) {
			cout << string(reinterpret_cast<const char *>(content),
						   reinterpret_cast<const char *>(content) + length)
				 << endl
				 << "  > ";
			cout.flush();
			string s;
			cin >> s;
			if (s == "exit") {
				client.close();
			} else {
				client.write(PakHeadData{}, s.length(), s.c_str());
			}
		};
		client.run();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}