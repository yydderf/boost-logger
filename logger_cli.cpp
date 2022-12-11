#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <string>

#if defined(_WIN32)

#include <conio.h>

#else

#include <termio.h>
#include <unistd.h>

#endif

#include "asd.hpp"

static struct termios oldt, newt;

using boost::asio::ip::tcp;

void encode(std::string &data)
{
    std::string buf;
    buf.reserve(data.size());
    for (size_t p = 0; p < data.size(); p++) {
        switch (data[p]) {
        case '&':
            buf.append("&amp;");
            break;
        case '\"':
            buf.append("&quot;");
            break;
        case '\'':
            buf.append("&apos;");
            break;
        case '<':
            buf.append("&lt;");
            break;
        case '>':
            buf.append("&gt;");
            break;
        case '\n':
            buf.append("&NewLine;");
            break;
		case '\r':
			break;
        default:
            buf.append(&data[p], 1);
        }
    }
    data.swap(buf);
}

void out_shell(std::string data)
{
	encode(data);
	std::ostringstream ss;
	ss <<
	"<script>\n\tdocument.getElementById(\'session" <<
	"\').innerHTML += \'" << data << "\';\n</script>\n";
	std::cout << ss.str().c_str();
}

void eproc(std::string &data, unsigned char *key, unsigned char *iv,
		   unsigned char *cmsg, size_t &cmsg_len)
{
	unsigned char *c_data = (unsigned char *)data.c_str();
	cmsg_len = encrypt(c_data, strlen((char *)c_data), key, iv, cmsg);
}

void dproc(unsigned char *cmsg, unsigned char *key, unsigned char *iv,
		   unsigned char *dmsg, size_t &dmsg_len)
{
	//dmsg_len = decrypt(cmsg, strlen((char *)cmsg), key, iv, dmsg);
	dmsg_len = decrypt(cmsg, strlen((char *)cmsg), key, iv, dmsg);
	std::cout << dmsg << std::endl;
}

class client
{
public:
	client(boost::asio::io_service& serv, const std::string &hostname, const std::string &port)
	  : resolver_(serv), socket_(serv)
    {
		#ifndef _WIN32
		config();
		#endif
		tcp::resolver::query query(hostname, port);
		resolver_.async_resolve(query,
        boost::bind(&client::resolve_handler, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
    }
private:
	void config()
	{
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}

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
			std::cout << "% ";
			line += getchar();
			if (line[0] == '1') {
				tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
				return;
			}
			std::cout << std::endl;
			eproc(line, key, iv, cmsg, cmsg_len);
			//dproc(cmsg, key, iv, dmsg, dmsg_len);
			//boost::asio::async_write(socket_, boost::asio::buffer(line, line.size()),
			boost::asio::async_write(socket_, boost::asio::buffer(cmsg, cmsg_len),
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
			out_shell(ss.str());
			//std::cout << "connect_handler error: " << err.message() << "\n";
		}
	}

	void write_handler(const boost::system::error_code &ec)
	{
		cmsg_len = 0;
		line.clear();
		if (!ec) {
			std::cout << "% ";
			line += getchar();
			if (line[0] == '1') {
				tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
				return;
			}
			std::cout << std::endl;
			eproc(line, key, iv, cmsg, cmsg_len);
			//dproc(cmsg, key, iv, dmsg, dmsg_len);
			//dproc(line, key, iv);
			//boost::asio::async_write(socket_, boost::asio::buffer(line, line.size()),
			boost::asio::async_write(socket_, boost::asio::buffer(cmsg, cmsg_len),
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

		new client(io_service, "127.0.0.1", "6700");

		io_service.run();
	} catch (std::exception& e) {
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}
