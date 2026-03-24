#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <chrono>
#include <slamd/net/connection.hpp>

namespace slamd {
namespace _net {

bool Connection::is_alive() {
    return this->alive;
}

Connection::~Connection() {
    this->stop_requested = true;
    // Use OS-level shutdown to unblock any pending asio::write on the worker
    // thread. We avoid calling ASIO socket methods here because they are not
    // thread-safe with concurrent operations on the worker.
    auto handle = this->socket.native_handle();
    if (handle >= 0) {
        ::shutdown(handle, SHUT_RDWR);
    }
    if (this->worker.joinable()) {
        this->worker.join();
    }
}

Connection::Connection(
    asio::ip::tcp::socket socket
)
    : alive(true),
      socket(std::move(socket)) {
    SPDLOG_INFO("Client connected");

    this->worker = std::thread(&Connection::job, this);
}

void Connection::write(
    const std::vector<uint8_t>& msg
) {
    auto msg_ptr = std::make_shared<std::vector<uint8_t>>(msg);
    this->message_queue.push(msg_ptr);
}

void Connection::write(
    std::shared_ptr<std::vector<uint8_t>> msg
) {
    this->message_queue.push(msg);
}

void Connection::job() {
    while (!this->stop_requested) {
        auto maybe_buffer =
            this->message_queue.timeout_pop(std::chrono::milliseconds(10));

        if (!maybe_buffer.has_value()) {
            if (!this->socket.is_open()) {
                SPDLOG_INFO("Socket closed, closing connection");
                this->alive = false;
                return;
            }

            continue;
        }

        std::shared_ptr<std::vector<uint8_t>> data = maybe_buffer.value();

        uint32_t len = htonl(data->size());

        std::vector<asio::const_buffer> buffers = {
            asio::buffer(&len, sizeof(len)),
            asio::buffer(*data)
        };

        std::error_code ec;
        asio::write(socket, buffers, ec);

        if (ec) {
            SPDLOG_INFO("Write failed: {}", ec.message());

            SPDLOG_ERROR("Closing connection...");
            try {
                socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                socket.close();
            } catch (...) {
            }
            this->alive = false;
            return;
        }
    }
}

}  // namespace _net
}  // namespace slamd