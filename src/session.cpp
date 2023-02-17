#include "session.h"
using namespace std;
using namespace asio;
Session::Session(asio::ip::tcp::socket socket)
	: socket(move(socket)), is_reading_head(true), is_writing(false),
	  bodylen(0) {
	stop_mtx.lock();
}
Session::~Session() { stop(); }
void Session::start() {
	auto guard = shared_from_this();
	doRead();
}
void Session::stop() {
	if (stop_mtx.try_lock()) {
		stop_mtx.unlock();
		return;
	}
	stop_mtx.unlock();
	socket.close();
}
void Session::write(PakSize length, const void *content) {
	if (stop_mtx.try_lock()) {
		stop_mtx.unlock();
		return;
	}
	auto guard = shared_from_this();
	const static size_t hlen = sizeof(PakSize);
	lock_guard<mutex> lock(send_mtx);
	void *dst = buffer_cast<void *>(send_buf.prepare(length + hlen));
	memcpy(dst, &length, hlen);
	if (length) {
		memcpy(reinterpret_cast<char *>(dst) + hlen, content, length);
	}
	send_buf.commit(length + hlen);
	if (!is_writing) {
		doWrite();
	}
}
void Session::doRead() {
	if (stop_mtx.try_lock()) {
		stop_mtx.unlock();
		return;
	}
	auto guard = shared_from_this();
	const static size_t hlen = sizeof(PakSize);
	static const size_t recv_ones_bytenum = 1024;
	socket.async_read_some(
		recv_buf.prepare(recv_ones_bytenum),
		[this](const std::error_code &ec, size_t len) {
			if (stop_mtx.try_lock()) {
				stop_mtx.unlock();
				return;
			}
			if (ec) {
				if (error_callback)
					error_callback(true, ec);
			} else {
				recv_buf.commit(len);
				while (true) {
					if (is_reading_head) {
						if (recv_buf.size() >= sizeof(hlen)) {
							is_reading_head = false;
							memcpy(&bodylen,
								   asio::buffer_cast<const PakSize *>(
									   recv_buf.data()),
								   hlen);
							recv_buf.consume(hlen);
						} else {
							break;
						}
					} else {
						if (recv_buf.size() >= bodylen) {
							is_reading_head = true;
							if (read_callback)
								read_callback(bodylen,
											  asio::buffer_cast<const void *>(
												  recv_buf.data()));
							recv_buf.consume(bodylen);
						} else {
							break;
						}
					}
				}
				doRead();
			}
		});
}
void Session::doWrite() {
	if (stop_mtx.try_lock()) {
		stop_mtx.unlock();
		return;
	}
	auto guard = shared_from_this();
	if (send_buf.size() == 0) {
		is_writing = false;
		return;
	}
	is_writing = true;
	socket.async_write_some(send_buf.data(),
							[this](const std::error_code &ec, size_t len) {
								if (stop_mtx.try_lock()) {
									stop_mtx.unlock();
									return;
								}
								if (ec) {
									if (error_callback)
										error_callback(false, ec);
								} else {
									send_buf.consume(len);
									std::lock_guard<std::mutex> lock(send_mtx);
									doWrite();
								}
							});
}