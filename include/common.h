#pragma once
#include <asio.hpp>
#include <functional>
#include <mutex>
struct EventError {
	std::string message;
};
struct EventConnect {
	const asio::ip::tcp::socket *socket;
};

struct EventDisconnect {};

template <typename MessageType> struct PkgHeader {
	MessageType type;
	size_t body_length;
};

struct EmptyType {};
template <typename MessageType, typename Datatype = EmptyType> class Session {

public:
	Session(Datatype, asio::ip::tcp::socket,
			std::function<void(Session<MessageType, Datatype> *, MessageType,
							   size_t, const void *)>
				read_callback,
			std::function<void(Session<MessageType, Datatype> *,
							   const std::string &)>
				error_callback);
	Session(asio::ip::tcp::socket,
			std::function<void(Session<MessageType, Datatype> *, MessageType,
							   size_t, const void *)>
				read_callback,
			std::function<void(Session<MessageType, Datatype> *,
							   const std::string &)>
				error_callback);

	void write(size_t length, const void *buf);
	Datatype data;
	asio::ip::tcp::socket socket;
	PkgHeader<MessageType> pkg_header;
	void run();

private:
	void doRead();
	void doWrite();
	std::function<void(Session<MessageType, Datatype> *, MessageType, size_t,
					   const void *)>
		read_callback;
	std::function<void(Session<MessageType, Datatype> *, const std::string &)>
		error_callback;
	std::mutex write_mutex;
	bool is_writing;
	bool is_reading_body;
	asio::streambuf read_buffer;
	asio::streambuf write_buffer;
};

template <typename MessageType, typename Datatype>
inline Session<MessageType, Datatype>::Session(
	Datatype data, asio::ip::tcp::socket socket,
	std::function<void(Session<MessageType, Datatype> *, MessageType, size_t,
					   const void *)>
		read_callback,
	std::function<void(Session<MessageType, Datatype> *, const std::string &)>
		error_callback)
	: data(data), socket(std::move(socket)), pkg_header(),
	  read_callback(read_callback), error_callback(error_callback),
	  write_mutex(), is_writing(false), is_reading_body(false), read_buffer(),
	  write_buffer() {}
template <typename MessageType, typename Datatype>
inline Session<MessageType, Datatype>::Session(
	asio::ip::tcp::socket socket,
	std::function<void(Session<MessageType, Datatype> *, MessageType, size_t,
					   const void *)>
		read_callback,
	std::function<void(Session<MessageType, Datatype> *, const std::string &)>
		error_callback)
	: socket(std::move(socket)), pkg_header(), read_callback(read_callback),
	  error_callback(error_callback), write_mutex(), is_writing(false),
	  is_reading_body(false), read_buffer(), write_buffer() {}
template <typename MessageType, typename Datatype>
inline void Session<MessageType, Datatype>::write(size_t length,
												  const void *buf) {
	if (length <= 0 || buf == nullptr)
		return;
	std::lock_guard lock(write_mutex);
	memcpy(asio::buffer_cast<void *>(write_buffer.prepare(length)), buf,
		   length);
	write_buffer.commit(length);
	if (!is_writing) {
		doWrite();
	}
}
template <typename MessageType, typename Datatype>
inline void Session<MessageType, Datatype>::run() {
	doRead();
}
template <typename MessageType, typename Datatype>
inline void Session<MessageType, Datatype>::doRead() {
	static const size_t bytes_num = 1024;
	socket.async_read_some(
		read_buffer.prepare(bytes_num),
		[this](const std::error_code &ec, size_t len) {
			if (ec)
				error_callback(this, "read failed: " + ec.message());
			else {
				read_buffer.commit(len);
				while (1) {
					if (is_reading_body) {
						if (read_buffer.size() >= pkg_header.body_length) {
							is_reading_body = false;
							read_callback(this, pkg_header.type,
										  pkg_header.body_length,
										  asio::buffer_cast<const void *>(
											  read_buffer.data()));
							read_buffer.consume(pkg_header.body_length);
						} else {
							break;
						}
					} else {
						if (read_buffer.size() >= sizeof(pkg_header)) {
							is_reading_body = true;
							memcpy(&pkg_header,
								   asio::buffer_cast<
									   const PkgHeader<MessageType> *>(
									   read_buffer.data()),
								   sizeof(pkg_header));
							read_buffer.consume(sizeof(pkg_header));
						} else {
							break;
						}
					}
				}
				doRead();
			}
		});
}
template <typename MessageType, typename Datatype>
inline void Session<MessageType, Datatype>::doWrite() {
	if (write_buffer.size() == 0) {
		is_writing = false;
		return;
	}
	is_writing = true;
	socket.async_write_some(
		write_buffer.data(), [this](const std::error_code &ec, size_t len) {
			if (ec)
				error_callback(this, "write failed: " + ec.message());
			else {
				write_buffer.consume(len);
				std::lock_guard lock(write_mutex);
				if (write_buffer.size() == 0)
					is_writing = false;
				else
					doWrite();
			}
		});
}