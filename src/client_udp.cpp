#include "client_udp.h"

using namespace asio::ip;

ClientUDP::ClientUDP(const std::string &ip, short port)
	: io_ctx(new asio::io_context()),
	  socket_(*io_ctx, udp::endpoint(udp::v4(), 0)),
	  remote_endpoint(address::from_string(ip), port) {}
ClientUDP::ClientUDP(asio::io_context &io_context, const std::string &ip,
					 short port)
	: io_ctx(nullptr), socket_(io_context, udp::endpoint(udp::v4(), 0)),
	  remote_endpoint(address::from_string(ip), port) {}
ClientUDP::~ClientUDP() {
	if (io_ctx)
		delete io_ctx;
}
// 开始运行（阻塞）
void ClientUDP::run() {
	if (io_ctx) {
		io_ctx->run();
	}
}
// 停止
void ClientUDP::stop() {
	if (io_ctx) {
		io_ctx->stop();
	}
}
// 发送
void ClientUDP::write(PakHeadData head_data, size_t length,
					  const void *content) {
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr) {
		if (onError)
			onError("write error: bad alloc");
		return;
	}
	*(PakHeadData *)buf = head_data;

	if (length)
		memcpy(buf + HD_LEN, content, length);

	socket_.async_send_to(
		asio::buffer(buf, length + HD_LEN), remote_endpoint,
		[this, buf](std::error_code ec, std::size_t /*bytes_sent*/) {
			if (ec) {
				if (onError)
					onError("send error: bad alloc" + ec.message());
			}
			delete[] buf;
		});
}
