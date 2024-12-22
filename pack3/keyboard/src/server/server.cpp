#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <thread>
#include <sstream>
#include <iomanip>
#include <ctime>

using boost::asio::ip::tcp;

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // new session
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // fork again and kill parent
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    chdir("/");

    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
}

std::string set_filename(const tcp::endpoint& client_endpoint) {
    std::ostringstream filename;
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    filename << "client_"
       << client_endpoint.address().to_string() << "_"
       << std::put_time(&tm, "%Y%m%d%H%M%S")
       << ".log";

    return filename.str();
}

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket, const std::string& filename)
        : socket_(std::move(socket)), file_(filename, std::ios::app) {}

    void start() {
        if (!file_.is_open()) {
            std::cerr << "Failed to open file for client\n";
            return;
        }
        read_data();
    }

private:
    void read_data() {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string data(buffer_.data(), length);
                    file_ << data;
                    file_.flush();
                    read_data();
                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "Error reading from client: " << ec.message() << "\n";
                }
            });
    }

    tcp::socket socket_;
    std::array<char, 1024> buffer_;
    std::ofstream file_;
};


class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept_client();
    }

private:
    void accept_client() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto client_endpoint = socket.remote_endpoint();
                    std::string filename = set_filename(client_endpoint);
                    std::make_shared<ClientSession>(std::move(socket), filename)->start();
                } else {
                    std::cerr << "Error accepting connection: " << ec.message() << "\n";
                }
                accept_client();
            });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }

    short port = static_cast<short>(std::stoi(argv[1]));
    if (port <= 0 and port > 65535) {
        std::cerr << "Invalid port number\n";
        return EXIT_FAILURE;
    }

    // go deamon
    daemonize();

    try {
        boost::asio::io_context io_context;
        Server server(io_context, port);
        // std::cout << "Server started on port " << port << "\n";
        io_context.run();
    } catch (const std::exception& e) {
        // TODO: error handling, log to file
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
