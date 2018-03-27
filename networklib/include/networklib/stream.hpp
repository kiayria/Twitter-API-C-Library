#ifndef STREAM_HPP
#define STREAM_HPP
#include <cstddef>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

#include <networklib/detail/network.hpp>
#include <networklib/request.hpp>
#include <networklib/response.hpp>

namespace tal {
namespace detail {
struct Socket;
}  // namespace detail

class Stream {
   public:
    using Callback = std::function<void(const Response&)>;
    using Condition = std::function<bool(const Response&)>;

    /// Create an asynchronous Stream object with an authorized Request. Does
    /// not open the stream.
    Stream(const Request& request);

    /// Register a callback function to the stream which will be called when the
    /// Stream is open and a Response had been recieved.
    void register_function(Callback f1,
                           Condition f2 = [](const Response&) { return true; });

    /// Connects to the Stream endpoing and begins processing Responses.
    void open();

    /// Shuts down the stream, stopping processing.
    void close();

   private:
    Request request_;

    std::unique_ptr<detail::Socket> sock_stream_;

    std::vector<std::pair<Callback, Condition>> callbacks_;
    std::mutex callbacks_mutex_;

    bool reconnect_{false};
    std::mutex reconnect_mtx_;

    // Sends to all functions registered to this Stream.
    void send_response(const Response& response);

    void timer_expired(boost::system::error_code ec);

    // Reads from the socket, creates Response objects and sends them to
    // each callback. Checks for reconnect_ on beginning of every iteration.
    void dispatch(const boost::system::error_code& ec,
                  std::size_t bytes_transfered);
};

}  // namespace tal
#endif  // STREAM_HPP
