#pragma once
#include "common.h"
#include <asio.hpp>
#include <functional>
#include <mutex>

template <typename MessageType> class Client {
	friend class Session<MessageType>;

public:
	// 构造时连接
	Client(const std::string &ip, int port);
	~Client();
	// 接受到包的回调
	void onMessage(std::function<void(MessageType type, size_t length,
									  const void *content)>);
	// 产生错误的回调
	void onEventError(std::function<void(EventError event)>);
	// 成功连接的回调
	void onEventConnect(std::function<void(EventConnect event)>);
	// 对方连接断开的回调
	void onEventDisconnect(std::function<void(EventDisconnect event)>);
	// 发送报文
	void send(MessageType type, size_t length, const void *content);
	// 开始运行
	void run();
	// 主动关闭连接
	void close();

private:
	asio::io_context io_ctx;
	Session<MessageType> session;
	std::function<void(MessageType type, size_t length, const void *content)>
		message_handler;
	std::function<void(EventError event)> event_error_handler;
	std::function<void(EventConnect event)> event_connect_handler;
	std::function<void(EventDisconnect event)> event_disconnect_handler;
	std::mutex send_mutex;
};

template <typename MessageType>
inline Client<MessageType>::Client(const std::string &server_ip,
								   int server_port)
	: io_ctx(), session(
					std::move(asio::ip::tcp::socket(io_ctx)),
					[this](Session<MessageType> *s, MessageType type,
						   size_t len, const void *content) {
						if (message_handler) {
							message_handler(type, len, content);
						}
					},
					[this](Session<MessageType> *s, const std::string &msg) {
						if (event_error_handler) {
							event_error_handler(EventError{msg});
						}
					}) {
	session.socket.async_connect(
		asio::ip::tcp::endpoint(asio::ip::address::from_string(server_ip),
								server_port),
		[this](std::error_code ec) {
			if (!ec) {
				session.run();
				if (event_connect_handler) {
					event_connect_handler(EventConnect{&session.socket});
				}
			} else {
				if (event_error_handler) {
					event_error_handler(
						EventError{"connect failed: " + ec.message()});
				}
			}
		});
}
template <typename MessageType> inline Client<MessageType>::~Client() {
	close();
}
template <typename MessageType>
inline void Client<MessageType>::onMessage(
	std::function<void(MessageType type, size_t length, const void *content)>
		handler) {
	message_handler = handler;
}
template <typename MessageType>
inline void Client<MessageType>::onEventError(
	std::function<void(EventError event)> handler) {
	event_error_handler = handler;
}
template <typename MessageType>
inline void Client<MessageType>::onEventConnect(
	std::function<void(EventConnect event)> handler) {
	event_connect_handler = handler;
}
template <typename MessageType>
inline void Client<MessageType>::onEventDisconnect(
	std::function<void(EventDisconnect event)> handler) {
	event_disconnect_handler = handler;
}
template <typename MessageType>
inline void Client<MessageType>::send(MessageType type, size_t length,
									  const void *content) {
	PkgHeader<MessageType> header{type, length};
	char *buf = new char[length + sizeof(header)];
	memcpy(buf, &header, sizeof(header));
	if (length)
		memcpy(buf + sizeof(header), content, length);
	session.write(length + sizeof(header), buf);
	delete[] buf;
}
template <typename MessageType> inline void Client<MessageType>::run() {
	io_ctx.run();
}
template <typename MessageType> inline void Client<MessageType>::close() {
	session.socket.close();
	io_ctx.stop();
}