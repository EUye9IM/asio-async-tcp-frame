#include "common.hpp"
#include <iostream>
#include <server.h>
using namespace std;
int main() {
	try {
		Server<MsgType> server(ip, port);
		server.onMessage([&server](Session<MsgType> *sess, MsgType type,
								   int length, const void *content) {
			if (type == MsgType::STR) {
				cout << "receive: " << sess->socket.remote_endpoint() << ": "
					 << string().assign((const char *)content, length) << endl;

				if (string().assign((const char *)content, length) == "exit") {
					server.stop();
				}
				server.send(sess, type, length, content);
			}
		});
		server.onEventError([](Session<MsgType> *sess, EventError event) {
			cout << "error: ";
			if (sess)
				cout << sess->socket.remote_endpoint() << ": ";
			cout << event.message << endl;
		});
		server.onEventConnect([](Session<MsgType> *sess, EventConnect event) {
			cout << "connect " << sess->socket.remote_endpoint() << ": "
				 << event.socket->local_endpoint() << " -> "
				 << event.socket->remote_endpoint() << endl;
		});
		server.onEventDisconnect(
			[&server](Session<MsgType> *sess, EventDisconnect event) {
				cout << "disconnect " << sess->socket.remote_endpoint() << endl;
			});
		server.run();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}

	return 0;
}