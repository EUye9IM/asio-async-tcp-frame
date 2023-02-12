#include "common.hpp"
#include <client.h>
#include <iostream>
using namespace std;
int main() {
	try {
		Client<MsgType> client(ip, port);
		client.onMessage([](MsgType type, int length, const void *content) {
			if (type == MsgType::STR) {
				cout << "receive: "
					 << string().assign((const char *)content, length) << endl;
			}
		});
		client.onEventError([](EventError event) {
			cout << "error: " + event.message << endl;
		});
		client.onEventConnect([](EventConnect event) {
			cout << "connect " << event.socket->local_endpoint() << " -> "
				 << event.socket->remote_endpoint() << endl;
		});
		client.onEventDisconnect(
			[&client](EventDisconnect event) { cout << "disconnect" << endl; });
		thread t([&client]() {
			string s;
			while (!client.is_stopped()) {
				cin >> s;
				cout << "send: " << s << endl;
				client.send(MsgType::STR, s.length(),
							reinterpret_cast<const void *>(s.c_str()));
				if (s == "exit")
					client.close();
			}
		});
		client.run();
		t.join();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}

	return 0;
}