#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <slamd_window/connection.hpp>
#include <slamd_window/message.hpp>
#include <thread>

namespace slamd {

Connection::Connection(
    std::string ip,
    ushort port
)
    : ip(ip),
      port(port),
      messages() {
    this->job_thread = std::thread(&Connection::job, this);
}

Connection::~Connection() {
    this->stop_requested = true;

    {
        std::lock_guard<std::mutex> lock(this->socket_mutex);
        if (this->active_socket) {
            std::error_code ec;
            this->active_socket->close(ec);
        }
    }

    if (this->job_thread.joinable()) {
        this->job_thread.join();
    }
}

asio::ip::tcp::socket Connection::connect(
    asio::io_context& io_context
) {
    asio::ip::tcp::endpoint endpoint(
        asio::ip::make_address(this->ip),
        this->port
    );

    while (true) {
        if (this->stop_requested) {
            throw std::runtime_error("Stop requested");
        }

        try {
            asio::ip::tcp::socket socket(io_context);
            socket.connect(endpoint);
            connected = true;
            SPDLOG_INFO("Successfully connected to {}:{}", ip, port);
            return socket;
        } catch (const std::exception& e) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Connection::job() {
    SPDLOG_INFO("Connection job started for {}:{}", ip, port);

    asio::io_context io_ctx;
    std::optional<asio::ip::tcp::socket> socket_opt = std::nullopt;

    while (!this->stop_requested) {
        if (!connected) {
            try {
                socket_opt.emplace(this->connect(io_ctx));
            } catch (...) {
                break;
            }
        }

        auto& socket = socket_opt.value();

        {
            std::lock_guard<std::mutex> lock(this->socket_mutex);
            this->active_socket = &socket;
        }

        try {
            uint32_t len_net;
            asio::read(
                socket,
                asio::buffer(&len_net, 4),
                asio::transfer_exactly(4)
            );
            uint32_t len = ntohl(len_net);

            auto message = std::make_unique<Message>(len);

            asio::read(
                socket,
                asio::buffer(message->data(), len),
                asio::transfer_exactly(len)
            );

            this->messages.push(std::move(message));

        } catch (const std::exception& e) {
            SPDLOG_ERROR("Error while reading from socket: {}", e.what());
            {
                std::lock_guard<std::mutex> lock(this->socket_mutex);
                this->active_socket = nullptr;
            }
            connected = false;
            socket.close();
            SPDLOG_INFO("Socket closed due to error.");
        }
    }

    SPDLOG_INFO("Connection job ended for {}:{}", ip, port);
}

}  // namespace slamd