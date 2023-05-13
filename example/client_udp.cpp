#include "common.hpp"
#include <client_udp.h>
#include <iostream>
#include <server_udp.h>
using namespace std;

int main() {
	try {
		ClientUDP client;
		cout << "Connect success! You can type 'exit' if you want "
				"to close connection."
			 << endl;

		while (true) {
			cout << "  > ";
			cout.flush();
			string s;
			cin >> s;
			if (s == "exit") {
				break;
			} else {
				client.write(ip, port, PakHeadData{}, s.length(), s.c_str());
			}
		}
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}