#pragma once
#include "session.h"

class Client {
public:
	// 构造时连接
	Client(const std::string &ip, int port);
	~Client();
	// 发送报文
	void write(PakHeadData head_data, size_t length, const void *content);
	// 开始运行（阻塞）
	void run();
	// 主动关闭连接
	void close();
	// 消息回调
	std::function<void(PakHeadData type, size_t length, const void *content)>
		onMessage;
	// 错误回调
	std::function<void(const std::string &message)> onError;
	// 连接回调
	std::function<void(const asio::ip::tcp::socket &socket)> onConnect;
	// 断连回调
	std::function<void()> onDisconnect;

private:
	void _message(PakHeadData head_data, size_t length, const void *content);
	void _error(const std::string &socket);
	void _connect(const asio::ip::tcp::socket &socket);
	void _disconnect();
	asio::io_context io_ctx;
	SessionPtr session;
};
