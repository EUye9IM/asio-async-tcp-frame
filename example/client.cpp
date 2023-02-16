#include "common.hpp"
#include <iostream>
#include <session.h>
using namespace std;
int main() {
	try {
		asio::io_context ctx;
		{
			auto s = make_shared<Session>(move(asio::ip::tcp::socket(ctx)));
			s->read_callback = [](Session *session, Session::PakSize length,
								  const void *content) {
				cout << "read: "
					 << string((const char *)content,
							   (const char *)content + length)
					 << endl;
				session->stop();
			};
			s->error_callback = [](Session *session, bool err_when_reading,
								   const error_code &ec) {
				if (ec == asio::error::eof) {
					cout << "disconnect" << endl;
				} else {
					cout << "error: " << err_when_reading << ec.value()
						 << ec.message() << endl;
				}
			};
			s->socket.async_connect(
				asio::ip::tcp::endpoint(asio::ip::address::from_string(ip),
										port),
				[ss = s.get()](std::error_code ec) {
					if (!ec) {
						ss->start();
						cout << "connect, write: hello" << endl;
						ss->write(5, "hello");
					} else {
						cout << "connect failed: " << ec.message() << endl;
						exit(1);
					}
				});
			ctx.run();
		}
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}
	return 0;
}