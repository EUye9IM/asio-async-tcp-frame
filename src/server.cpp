#include "server.h"

using namespace std;
using namespace asio;
using asio::ip::tcp;

static const int HD_LEN = sizeof(PakHeadData);

Server::Server(const std::string &ip, int port)
	: io_ctx(), acc(io_ctx, tcp::endpoint(ip::address::from_string(ip), port)) {
	doAccept();
}

Server::~Server() { stop(); }
void Server::write(Session *session, PakHeadData head_data, size_t length,
				   const void *content) {
	SessionPtr s;
	if (true) {
		lock_guard<mutex> guard(sess_pool_mtx);
		if (0 == session_pool.count(session))
			return;
		s = session_pool[session];
	}
	if (length < 0)
		length = 0;
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr)
		return;
	memcpy(buf, &head_data, HD_LEN);
	if (length)
		memcpy(buf + HD_LEN, content, length);
	s->write(length + HD_LEN, buf);
	delete[] buf;
}
void Server::broadcast(PakHeadData head_data, size_t length,
					   const void *content) {
	if (length < 0)
		length = 0;
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr)
		return;
	memcpy(buf, &head_data, HD_LEN);
	if (length)
		memcpy(buf + HD_LEN, content, length);
	lock_guard<mutex> guard(sess_pool_mtx);
	for (auto &s : session_pool) {
		s.second->write(length + HD_LEN, buf);
	}
	delete[] buf;
}
void Server::run() { io_ctx.run(); }
void Server::stop() {
	acc.close();
	closeAll();
}
void Server::close(Session *session) {
	lock_guard<mutex> guard(sess_pool_mtx);
	if (session_pool.count(session)) {
		_disconnect(session);
		session_pool.erase(session);
	}
}
void Server::closeAll() {
	lock_guard<mutex> guard(sess_pool_mtx);
	for (auto &s : session_pool) {
		_disconnect(s.first);
		session_pool.erase(s.first);
	}
}
void Server::doAccept() {
	acc.async_accept([this](const error_code &ec, tcp::socket socket) {
		if (!ec) {
			auto session = make_shared<Session>(move(socket));
			if (true) {
				lock_guard<mutex> guard(sess_pool_mtx);
				session_pool[session.get()] = session;
			}
			session->error_callback =
				[this, s = session.get()](bool at_reading,
										  const error_code &ec) {
					if (ec != error::eof) {
						_error(s, ec.message());
					}
					this->close(s);
				};
			session->read_callback =
				[this, s = session.get()](PakSize length, const void *content) {
					int bodylen = length - HD_LEN;
					if (bodylen < 0)
						_error(s, "read failed: content is too short");
					PakHeadData hd;
					memcpy(&hd, content, HD_LEN);
					_message(s, hd, bodylen,
							 reinterpret_cast<const char *>(content) + HD_LEN);
				};
			_connect(session.get(), session->socket);
			session->start();
			doAccept();
		} else {
			_error(nullptr, "accept failed: " + ec.message());
		}
	});
}
void Server::_message(Session *session, PakHeadData head_data, size_t length,
					  const void *content) {
	if (onMessage)
		onMessage(session, head_data, length, content);
}
void Server::_error(Session *session, const std::string &message) {
	if (onError)
		onError(session, message);
}
void Server::_connect(Session *session, const asio::ip::tcp::socket &socket) {
	if (onConnect)
		onConnect(session, socket);
}
void Server::_disconnect(Session *session) {
	if (onDisconnect)
		onDisconnect(session);
}