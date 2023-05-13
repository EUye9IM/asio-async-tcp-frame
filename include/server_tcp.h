#pragma once
#include "session.h"
#include <map>

class Server {
public:
	// 监听端口
	Server(const std::string &ip, int port);
	Server(asio::io_context &io_context, const std::string &ip, int port);
	~Server();
	// 消息回调
	std::function<void(Session *session, PakHeadData type, size_t length,
					   const void *content)>
		onMessage;
	// 错误回调
	std::function<void(Session *session, const std::string &message)> onError;
	// 连接回调
	std::function<void(Session *session, const asio::ip::tcp::socket &socket)>
		onConnect;
	// 断连回调
	std::function<void(Session *session)> onDisconnect;
	// 发送报文
	void write(Session *session, PakHeadData head_data, size_t length,
			   const void *content);
	// 广播报文
	void broadcast(PakHeadData head_data, size_t length, const void *content);
	// 开始运行（阻塞）
	void run();
	// 停止
	void stop();
	// 关闭连接
	void close(Session *session);
	// 关闭所有连接
	void closeAll();

private:
	void doAccept();
	void _message(Session *session, PakHeadData type, size_t length,
				  const void *content);
	void _error(Session *session, const std::string &socket);
	void _connect(Session *session, const asio::ip::tcp::socket &socket);
	void _disconnect(Session *session);
	asio::io_context *io_ctx;
	asio::ip::tcp::acceptor acc;
	std::mutex sess_pool_mtx;
	std::map<Session *, SessionPtr> session_pool;
};