#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <string>

#include "asd.hpp"

#define MAX_Q 5

using boost::asio::ip::tcp;

void printConsole(std::string hostname, unsigned short port)
{
    std::ostringstream scope;
    std::ostringstream _id;
	scope <<
    "          <th scope=\"col\">" << hostname << ":" << port << "</th>\n";
	_id <<
    "          <td><pre id=\"session\" class=\"mb-0\"></pre></td>\n";

    std::string header = "Content-type: text/html\r\n\r\n";
	std::cout << header.c_str();

    std::string body = 
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "  <head>\n"
    "    <meta charset=\"UTF-8\" />\n"
    "    <title>Logger Console</title>\n"
    "    <link\n"
    "      rel=\"stylesheet\"\n"
    "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n"
    "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n"
    "      crossorigin=\"anonymous\"\n"
    "    />\n"
    "    <link\n"
    "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
    "      rel=\"stylesheet\"\n"
    "    />\n"
    "    <link\n"
    "      rel=\"icon\"\n"
    "      type=\"image/png\"\n"
    "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n"
    "    />\n"
    "    <style>\n"
    "      * {\n"
    "        font-family: 'Source Code Pro', monospace;\n"
    "        font-size: 1rem !important;\n"
    "      }\n"
    "      body {\n"
    "        background-color: #212529;\n"
    "      }\n"
    "      pre {\n"
    "        color: #cccccc;\n"
    "      }\n"
    "      b {\n"
    "        color: #01b468;\n"
    "      }\n"
    "    </style>\n"
    "  </head>\n"
    "  <body>\n"
    "    <table class=\"table table-dark table-bordered\">\n"
    "      <thead>\n"
    "        <tr>\n";
    body += scope.str().c_str();
    body +=
    "        </tr>\n"
    "      </thead>\n"
    "      <tbody>\n"
    "        <tr>\n";
    body += _id.str().c_str();
    body +=
    "        </tr>\n"
    "      </tbody>\n"
    "    </table>\n"
    "  </body>\n"
    "</html>\n";
	std::cout << body.c_str();
	std::fflush(stdout);
}

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

void printBanner(std::string hostname, unsigned short port)
{
	std::cout << "Connected from " << hostname << ":" << port << std::endl;
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

void dproc(unsigned char *encrypted_msg, unsigned char *key, unsigned char *iv,
		   unsigned char *decrypted_msg, size_t &decrypted_len)
{
	if (strlen((char *)encrypted_msg) == 0) return;
	decrypted_len = decrypt(encrypted_msg, strlen((char *)encrypted_msg), key, iv, decrypted_msg);
	std::cout << decrypted_msg << std::endl;
	std::fflush(stdout);
}

void out(unsigned char *msg)
{
	std::cout << msg;
	std::fflush(stdout);
}


class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket socket)
		: ssock(std::move(socket)) {}
	void start()
	{
		//printConsole(ssock.remote_endpoint().address().to_string(), ssock.remote_endpoint().port());
		boost::asio::async_read(ssock, boost::asio::buffer(encrypted_msg, 128),
			boost::asio::transfer_at_least(1),
			boost::bind(&Session::read_handler, shared_from_this(),
			boost::asio::placeholders::error));
	}
private:

	void read_handler(const boost::system::error_code& ec)
	{
		if (!ec) {
			//std::ostringstream ss;
			//data = ss.str();
			//out(encrypted_msg);
			dproc(encrypted_msg, key, iv, decrypted_msg, decrypted_len);
			//dproc(data, key, iv);
			//outdata(data);
			boost::asio::async_read(ssock, boost::asio::buffer(encrypted_msg, 128),
				boost::asio::transfer_at_least(1),
				boost::bind(&Session::read_handler, shared_from_this(),
				boost::asio::placeholders::error));
		} else if (ec == boost::asio::error::eof) {
			std::cout << ssock.remote_endpoint().address().to_string() << " had disconnected\n";
			return;
		} else {
			std::cout << "read_handler error: " << ec.message() << "\n";
		}
	}

	tcp::socket ssock;
	boost::asio::streambuf response;

	std::string data;
	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
	unsigned char *iv = (unsigned char *)"0123456789012345";
	unsigned char encrypted_msg[128];
	unsigned char decrypted_msg[128];
	size_t decrypted_len;
};

class Server {
public:
	Server(boost::asio::io_service &io_service, short port)
		: acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
			sock(io_service) { do_accept(); }

private:
	void do_accept()
	{
		acceptor.async_accept(sock, [this](boost::system::error_code ec) {
		if (!ec)
		{
			printBanner(sock.remote_endpoint().address().to_string(), sock.remote_endpoint().port());
			std::make_shared<Session>(std::move(sock))->start();
		}

		do_accept();
		});
	}

	tcp::acceptor acceptor;
	tcp::socket sock;
};

int main(int argc, char* argv[])
{
	try {
		boost::asio::io_service io_service;

		Server s(io_service, 6700);

		io_service.run();
	} catch (std::exception& e) {
		out_shell(e.what());
	}

	return 0;
}
