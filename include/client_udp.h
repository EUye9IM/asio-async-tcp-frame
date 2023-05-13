#pragma once
#include "config.h"

#include <asio.hpp>
#include <string>

class ClientUDP {
public:
	ClientUDP();
	ClientUDP(asio::io_context &io_context);
	~ClientUDP();

	// 开始运行（阻塞）
	void run();
	// 执行一次（发送）
	void run_once();
	// 停止
	void stop();
	// 发送
	void write(const std::string &ip, short port, PakHeadData head_data,
			   size_t length, const void *content);
	void write(asio::ip::udp::endpoint endpoint, PakHeadData head_data,
			   size_t length, const void *content);
	// 错误回调
	std::function<void(const std::string &message)> onError;

private:
	asio::io_context *io_ctx;
	asio::ip::udp::socket socket_;
};