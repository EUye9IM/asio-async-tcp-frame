#include "client_tcp.h"

using namespace std;
using namespace asio;
using asio::ip::tcp;

ClientTCP::ClientTCP(const std::string &ip, int port)
	: io_ctx(new asio::io_context()) {
	session = make_shared<Session>(move(tcp::socket(*io_ctx)));
	session->read_callback = [this](PakSize length, const void *content) {
		int bodylen = length - HD_LEN;
		if (bodylen < 0)
			_error("read failed: content is too short");
		PakHeadData hd;
		memcpy(&hd, content, HD_LEN);
		_message(hd, bodylen, reinterpret_cast<const char *>(content) + HD_LEN);
	};
	session->error_callback = [this](bool err_when_reading,
									 const error_code &ec) {
		if (ec != asio::error::eof) {
			_error(string() + (err_when_reading ? "read" : "write") +
				   " failed: " + ec.message());
		}
		_disconnect();
	};
	session->socket.async_connect(
		tcp::endpoint(ip::address::from_string(ip), port),
		[this](std::error_code ec) {
			if (!ec) {
				session->start();
				_connect(session->socket);
			} else {
				_error(ec.message());
			}
		});
}
ClientTCP::ClientTCP(asio::io_context &io_context, const std::string &ip,
					 int port)
	: io_ctx(nullptr) {
	session = make_shared<Session>(move(tcp::socket(io_context)));
	session->read_callback = [this](PakSize length, const void *content) {
		int bodylen = length - HD_LEN;
		if (bodylen < 0)
			_error("read failed: content is too short");
		PakHeadData hd;
		memcpy(&hd, content, HD_LEN);
		_message(hd, bodylen, reinterpret_cast<const char *>(content) + HD_LEN);
	};
	session->error_callback = [this](bool err_when_reading,
									 const error_code &ec) {
		if (ec != asio::error::eof) {
			_error(string() + (err_when_reading ? "read" : "write") +
				   " failed: " + ec.message());
		}
		_disconnect();
	};
	session->socket.async_connect(
		tcp::endpoint(ip::address::from_string(ip), port),
		[this](std::error_code ec) {
			if (!ec) {
				session->start();
				_connect(session->socket);
			} else {
				_error(ec.message());
			}
		});
}
ClientTCP::~ClientTCP() {
	session->stop();
	if (io_ctx) {
		delete io_ctx;
	}
}
void ClientTCP::write(PakHeadData head_data, size_t length,
					  const void *content) {
	if (length < 0)
		length = 0;
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr)
		return;
	memcpy(buf, &head_data, HD_LEN);
	if (length)
		memcpy(buf + HD_LEN, content, length);
	session->write(length + HD_LEN, buf);
	delete[] buf;
}
void ClientTCP::run() {
	if (io_ctx)
		io_ctx->run();
}
void ClientTCP::close() { session->stop(); }
void ClientTCP::_message(PakHeadData head_data, size_t length,
						 const void *content) {
	if (onMessage)
		onMessage(head_data, length, content);
}
void ClientTCP::_error(const std::string &message) {
	if (onError)
		onError(message);
}
void ClientTCP::_connect(const asio::ip::tcp::socket &socket) {
	if (onConnect)
		onConnect(socket);
}
void ClientTCP::_disconnect() {
	if (onDisconnect)
		onDisconnect();
}