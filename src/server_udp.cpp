#include "server_udp.h"

using namespace asio::ip;

ServerUDP::ServerUDP(const std::string &ip, short port)
	: io_ctx(new asio::io_context()),
	  socket_(*io_ctx, udp::endpoint(address::from_string(ip), port)) {
	receive_buf = new char[BUF_LEN];
	if (!receive_buf) {
		throw std::runtime_error("bad alloc");
	}
	do_receive();
}
ServerUDP::ServerUDP(asio::io_context &io_context, const std::string &ip,
					 short port)
	: io_ctx(nullptr),
	  socket_(io_context, udp::endpoint(address::from_string(ip), port)) {

	receive_buf = new char[BUF_LEN];
	if (!receive_buf) {
		throw std::runtime_error("bad alloc");
	}
	do_receive();
}
ServerUDP::~ServerUDP() {
	if (io_ctx)
		delete io_ctx;
	if (receive_buf) {
		delete[] receive_buf;
	}
}
// 开始运行（阻塞）
void ServerUDP::run() {
	if (io_ctx) {
		io_ctx->run();
	}
}
// 停止
void ServerUDP::stop() {
	if (io_ctx) {
		io_ctx->stop();
	}
}
// 发送
void ServerUDP::write(const asio::ip::udp::endpoint remote_addr,
					  PakHeadData head_data, size_t length,
					  const void *content) {
	auto buf = new char[length + HD_LEN];
	if (buf == nullptr) {
		_error("write error: bad alloc");
		return;
	}
	*(PakHeadData *)buf = head_data;

	if (length)
		memcpy(buf + HD_LEN, content, length);

	socket_.async_send_to(
		asio::buffer(buf, length + HD_LEN), remote_addr,
		[this, buf](std::error_code ec, std::size_t /*bytes_sent*/) {
			if (ec) {
				_error(ec.message());
			}
			delete[] buf;
		});
}

void ServerUDP::do_receive() {
	socket_.async_receive_from(
		asio::buffer(receive_buf, BUF_LEN), sender_endpoint_,
		[this](std::error_code ec, std::size_t bytes_recvd) {
			if (!ec && bytes_recvd > HD_LEN) {
				if (onMessage) {
					onMessage(sender_endpoint_, *(PakHeadData *)receive_buf,
							  bytes_recvd - HD_LEN, receive_buf + HD_LEN);
				}
			}
			do_receive();
		});
}
void ServerUDP::_error(const std::string &message) {
	if (onError)
		onError(message);
}