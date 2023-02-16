#include "common.hpp"
#include <iostream>
#include <map>
#include <session.h>
using namespace std;

int main() {
	try {
		asio::io_context ctx;
		asio::ip::tcp::acceptor acc(
			ctx,
			asio::ip::tcp::endpoint(asio::ip::address::from_string(ip), port));
		mutex smapmtx;
		map<Session *, SessionPtr> smap;
		function<void()> onAcc = [&onAcc, &acc, &smap, &smapmtx]() {
			acc.async_accept([&onAcc, &smap,
							  &smapmtx](std::error_code ec,
										asio::ip::tcp::socket socket) {
				if (!ec) {
					auto session = make_shared<Session>(std::move(socket));
					if (true) {
						lock_guard<mutex> guard(smapmtx);
						smap[session.get()] = session;
					}
					session->error_callback =
						[&smap, &smapmtx](Session *session,
										  bool err_when_reading,
										  const std::error_code &ec) {
							if (ec == asio::error::eof) {
								cout << "disconnect" << endl;
							} else {
								cout << "error: " << ec.message();
							}
							session->stop();
							lock_guard<mutex> guard(smapmtx);
							smap.erase(session);
						};
					session->read_callback = [&smap,
											  &smapmtx](Session *session,
														Session::PakSize length,
														const void *content) {
						auto s = string((const char *)content,
										(const char *)content + length);
						cout << "read: " << s << endl;
						session->write(length, content);
					};
					session->start();
					cout << "accept" << endl;
				} else {
					cout << "accept failed: " + ec.message() << endl;
				}
				onAcc();
			});
		};
		onAcc();
		mutex w;
		ctx.run();
		w.lock();
		w.lock();
	} catch (const std::exception &e) {
		cout << "exception: " << e.what() << '\n';
	}

	return 0;
}