#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <cstring>

#define MAX_Q 5

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
	Session(tcp::socket socket) : socket_(std::move(socket)) {}

	void start()
	{
		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(
			boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length) {
			sscanf(data_, "%s %s %s %s %s", REQUEST_METHOD, REQUEST_URI,
					SERVER_PROTOCOL, _, HTTP_HOST);

			if (!ec) {
				do_write(length);
			}
		});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(
			socket_, boost::asio::buffer(status_str, strlen(status_str)),
			[this, self](boost::system::error_code ec, std::size_t) {
			if (!ec) {

				strcpy(REMOTE_ADDR, socket_.remote_endpoint().address().to_string().c_str());
				sprintf(REMOTE_PORT, "%u", socket_.remote_endpoint().port());

				strncpy(REQUEST_URI_CPY, REQUEST_URI, sizeof(REQUEST_URI_CPY));

				tok = strtok(REQUEST_URI_CPY, "?");
				strncpy(EXEC_FNAME + 1,	tok, sizeof(EXEC_FNAME) - 1);
				EXEC_FNAME[strlen(EXEC_FNAME)] = 0;
				tok = strtok(NULL, "?");
				if (tok != NULL) {
					strncpy(QUERY_STRING, tok, sizeof(QUERY_STRING));
					QUERY_STRING[strlen(QUERY_STRING)] = 0;
				}

				setenv("REQUEST_METHOD", REQUEST_METHOD, 1);
				setenv("REQUEST_URI", REQUEST_URI, 1);
				setenv("QUERY_STRING", QUERY_STRING, 1);
				setenv("SERVER_PROTOCOL", SERVER_PROTOCOL, 1);
				setenv("HTTP_HOST", HTTP_HOST, 1);
				setenv("SERVER_ADDR", SERVER_ADDR, 1);
				setenv("SERVER_PORT", SERVER_PORT, 1);
				setenv("REMOTE_ADDR", REMOTE_ADDR, 1);
				setenv("REMOTE_PORT", REMOTE_PORT, 1);

				int sock;
				switch (fork()) {
				case -1:
					break;
				case 0:
					sock = socket_.native_handle();
					dup2(sock, STDERR_FILENO);
					dup2(sock, STDIN_FILENO);
					dup2(sock, STDOUT_FILENO);
					socket_.close();

					if (execlp(EXEC_FNAME, EXEC_FNAME, NULL) < 0) {
						std::cout << "Content-type:text/html\r\n\r\nexec failed";
					}
					break;
				default:
					socket_.close();
					break;
				}

				*QUERY_STRING = 0;

				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
	char status_str[128] = "HTTP/1.1 200 OK\n";
	char *tok;
	char REQUEST_URI_CPY[1024];
	char REQUEST_METHOD[20];
	char REQUEST_URI[1024];
	char QUERY_STRING[512];
	char SERVER_PROTOCOL[64];
	char HTTP_HOST[64];
	char SERVER_ADDR[64];
	char SERVER_PORT[10];
	char REMOTE_ADDR[64];
	char REMOTE_PORT[10];
	char EXEC_FNAME[64] = ".";
	char _[64];
};

class Server {
public:
	Server(boost::asio::io_context &ioc, short port)
		: acceptor_(ioc, tcp::endpoint(tcp::v4(), port)),
			socket_(ioc) {
			do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
		if (!ec)
		{
			std::make_shared<Session>(std::move(socket_))->start();
		}

		do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

int main(int argc, char *argv[]) {
	try {
		if (argc != 2) {
			std::cerr << "usage: " << argv[0] << " <port>\n";
			return 1;
		}

		boost::asio::io_context ioc;

		Server s(ioc, std::atoi(argv[1]));

		ioc.run();
	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
