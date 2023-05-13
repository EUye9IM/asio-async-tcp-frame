#pragma once
#include "config.h"

#include <asio.hpp>
#include <string>

class ClientUDP {
public:
	ClientUDP(const std::string &ip, short port);
	ClientUDP(asio::io_context &io_context, const std::string &ip, short port);
	~ClientUDP();

	// 开始运行（阻塞）
	void run();
	// 停止
	void stop();
	// 发送
	void write(PakHeadData head_data, size_t length, const void *content);
	// 错误回调
	std::function<void(const std::string &message)> onError;

private:
	asio::io_context *io_ctx;
	asio::ip::udp::socket socket_;
	asio::ip::udp::endpoint remote_endpoint;
};