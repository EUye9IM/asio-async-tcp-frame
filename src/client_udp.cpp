#include "client_udp.h"

using namespace asio::ip;

ClientUDP::ClientUDP()
	: io_ctx(new asio::io_context()),
	  socket_(*io_ctx, udp::endpoint(udp::v4(), 0)) {}
ClientUDP::ClientUDP(asio::io_context &io_context)
	: io_ctx(nullptr), socket_(io_context, udp::endpoint(udp::v4(), 0)) {}
ClientUDP::~ClientUDP() {
	if (io_ctx) {
		io_ctx->stop();
		delete io_ctx;
	}
}
// 发送
void ClientUDP::write(const std::string &ip, short port, PakHeadData head_data,
					  size_t length, const void *content) {
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr) {
		if (onError)
			onError("write error: bad alloc");
		return;
	}
	*(PakHeadData *)buf = head_data;

	if (length)
		memcpy(buf + HD_LEN, content, length);

	socket_.send_to(asio::buffer(buf, length + HD_LEN),
					asio::ip::udp::endpoint(address::from_string(ip), port));
	delete[] buf;
}

// 发送
void ClientUDP::write(asio::ip::udp::endpoint endpoint, PakHeadData head_data,
					  size_t length, const void *content) {
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr) {
		if (onError)
			onError("write error: bad alloc");
		return;
	}
	*(PakHeadData *)buf = head_data;

	if (length)
		memcpy(buf + HD_LEN, content, length);

	socket_.send_to(asio::buffer(buf, length + HD_LEN), endpoint);
	delete[] buf;
}