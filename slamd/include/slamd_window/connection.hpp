#pragma once

#include <asio.hpp>
#include <mutex>
#include <slamd_common/utils/thread_safe_queue.hpp>
#include <slamd_window/message.hpp>
#include <thread>

namespace slamd {

class Connection {
   public:
    Connection(std::string ip, ushort port);
    ~Connection();

   public:
    const std::string ip;
    const ushort port;
    bool connected = false;
    _utils::ThreadSafeQueue<std::unique_ptr<Message>> messages;

   private:
    asio::ip::tcp::socket connect(asio::io_context& io_context);

    void job();

    std::thread job_thread;
    std::atomic<bool> stop_requested = false;

    std::mutex socket_mutex;
    asio::ip::tcp::socket* active_socket = nullptr;
};

}  // namespace slamd
