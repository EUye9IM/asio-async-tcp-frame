#pragma once
#include <asio.hpp>

#include <functional>
#include <memory>
class Session;
using SessionPtr = std::shared_ptr<Session>;
class Session : public std::enable_shared_from_this<Session> {
public:
	using PakSize = unsigned int;
	Session(asio::ip::tcp::socket socket);
	~Session();
	void start();
	void stop();
	void write(PakSize length, const void *content);

	asio::ip::tcp::socket socket;
	std::function<void(Session *session, PakSize length, const void *content)>
		read_callback;
	std::function<void(Session *session, bool err_when_reading,
					   const std::error_code &error_code)>
		error_callback;

private:
	void doRead();
	void doWrite();

	bool is_reading_head;
	bool is_writing;
	PakSize bodylen;
	std::mutex send_mtx;
	std::mutex stop_mtx;
	asio::streambuf recv_buf;
	asio::streambuf send_buf;
};