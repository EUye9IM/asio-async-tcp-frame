#pragma once
#include "config.h"

#include <asio.hpp>
#include <string>

class ServerUDP {
public:
	ServerUDP(const std::string &ip, short port);
	ServerUDP(asio::io_context &io_context, const std::string &ip, short port);
	~ServerUDP();

	// 消息回调
	std::function<void(const asio::ip::udp::endpoint remote_addr,
					   PakHeadData type, size_t length, const void *content)>
		onMessage;
	// 错误回调
	std::function<void(const std::string &message)> onError;
	// 开始运行（阻塞）
	void run();
	// 停止
	void stop();
	// 发送
	void write(const asio::ip::udp::endpoint remote_addr, PakHeadData head_data,
			   size_t length, const void *content);

private:
	void do_receive();
	void _error(const std::string &message);
    char *receive_buf;
    static const int BUF_LEN=65536;
	asio::io_context *io_ctx;
	asio::ip::udp::socket socket_;
	asio::ip::udp::endpoint sender_endpoint_;
};