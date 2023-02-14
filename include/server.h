#pragma once
#include "common.h"
#include <functional>
#include <set>
#include <string>

template <typename MessageType> class Server {
	friend class Session<MessageType>;

public:
	// 监听端口
	Server(const std::string &ip, int port);
	~Server();
	// 接受到包的回调
	void onMessage(
		std::function<void(Session<MessageType> *sess, MessageType type,
						   size_t length, const void *content)>);
	// 产生错误的回调
	void onEventError(
		std::function<void(Session<MessageType> *sess, EventError event)>);
	// 成功连接的回调
	void onEventConnect(
		std::function<void(Session<MessageType> *sess, EventConnect event)>);
	// 对方连接断开的回调
	void onEventDisconnect(
		std::function<void(Session<MessageType> *sess, EventDisconnect event)>);
	// 发送报文
	void send(Session<MessageType> *sess, MessageType type, size_t length,
			  const void *content);
	// 对所有连接发送报文
	void broadcast(MessageType type, size_t length, const void *content);
	// 开始运行
	void run();
	// 主动关闭连接
	void close(Session<MessageType> *s);
	// 关闭所有连接，停止服务
	void stop();
	bool is_stopped();

private:
	void doAccept();
	std::mutex set_mutex;
	std::set<Session<MessageType> *> session_set;

	asio::io_context io_ctx;
	asio::ip::tcp::acceptor acc;
	std::function<void(Session<MessageType> *sess, MessageType type, int length,
					   const void *content)>
		message_handler;
	std::function<void(Session<MessageType> *sess, EventError event)>
		event_error_handler;
	std::function<void(Session<MessageType> *sess, EventConnect event)>
		event_connect_handler;
	std::function<void(Session<MessageType> *sess, EventDisconnect event)>
		event_disconnect_handler;
	bool is_stop;
};

template <typename MessageType>
inline Server<MessageType>::Server(const std::string &ip, int port)
	: set_mutex(), session_set(), io_ctx(),
	  acc(io_ctx,
		  asio::ip::tcp::endpoint(asio::ip::address::from_string(ip), port)) {
	is_stop = false;
	doAccept();
}
template <typename MessageType> inline Server<MessageType>::~Server() {
	stop();
}
// 接受到包的回调
template <typename MessageType>
inline void Server<MessageType>::onMessage(
	std::function<void(Session<MessageType> *sess, MessageType type,
					   size_t length, const void *content)>
		h) {
	message_handler = h;
}
// 产生错误的回调
template <typename MessageType>
inline void Server<MessageType>::onEventError(
	std::function<void(Session<MessageType> *sess, EventError event)> h) {
	event_error_handler = h;
}
// 成功连接的回调
template <typename MessageType>
inline void Server<MessageType>::onEventConnect(
	std::function<void(Session<MessageType> *sess, EventConnect event)> h) {
	event_connect_handler = h;
}
// 对方连接断开的回调
template <typename MessageType>
inline void Server<MessageType>::onEventDisconnect(
	std::function<void(Session<MessageType> *sess, EventDisconnect event)> h) {
	event_disconnect_handler = h;
}
// 发送报文
template <typename MessageType>
inline void Server<MessageType>::send(Session<MessageType> *sess,
									  MessageType type, size_t length,
									  const void *content) {
	if (!sess)
		return;
	PkgHeader<MessageType> header{type, length};

	char *buf = new char[length + sizeof(header)];
	memcpy(buf, &header, sizeof(header));
	if (length)
		memcpy(buf + sizeof(header), content, length);

	sess->write(length + sizeof(header), buf);
	delete[] buf;
}
// 对所有连接发送报文
template <typename MessageType>
inline void Server<MessageType>::broadcast(MessageType type, size_t length,
										   const void *content) {
	std::lock_guard<std::mutex> lock(set_mutex);
	for (auto s = session_set.begin(); s != session_set.end(); s++) {
		send(*s, type, length, content);
	}
}
// 开始运行
template <typename MessageType> inline void Server<MessageType>::run() {
	io_ctx.run();
}
// 主动关闭连接
template <typename MessageType>
inline void Server<MessageType>::close(Session<MessageType> *s) {
	if (event_disconnect_handler) {
		event_disconnect_handler(s, EventDisconnect{});
	}
	std::lock_guard<std::mutex> lock(set_mutex);
	if (session_set.count(s)) {
		s->socket.close();
		session_set.erase(s);
		delete s;
	}
}
// 关闭所有连接，停止服务
template <typename MessageType> inline void Server<MessageType>::stop() {
	std::lock_guard<std::mutex> lock(set_mutex);
	for (auto s = session_set.begin(); s != session_set.end(); s++) {
		(*s)->socket.close();
		delete *s;
	}
	session_set.clear();
	io_ctx.stop();
	is_stop = true;
}

template <typename MessageType> inline bool Server<MessageType>::is_stopped() {
	return is_stop;
}

template <typename MessageType> inline void Server<MessageType>::doAccept() {
	acc.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
		if (!ec) {
			auto session = new Session<MessageType>(
				std::move(socket),
				[this](Session<MessageType> *s, MessageType type, size_t len,
					   const void *content) {
					if (message_handler) {
						message_handler(s, type, len, content);
					}
				},
				[this](Session<MessageType> *s, const std::string &msg) {
					if (event_error_handler) {
						event_error_handler(s, EventError{msg});
					}
					close(s);
				});
			session->run();
			if (true) {
				std::lock_guard<std::mutex> lock(set_mutex);
				session_set.insert(session);
			}
			if (event_connect_handler) {
				event_connect_handler(session, EventConnect{&session->socket});
			}
		} else {
			if (event_error_handler) {
				event_error_handler(
					nullptr, EventError{"accept failed: " + ec.message()});
			}
		}
		doAccept();
	});
}