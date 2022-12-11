CXX=g++
CXXFLAGS=-g -std=c++14 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))

all: http_server.cpp console.cpp logger.cpp logger_cli.cpp
	$(CXX) http_server.cpp -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) console.cpp -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) logger.cpp -o logger.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS) -lcrypto
	$(CXX) logger_cli.cpp -o logger_cli $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS) -lcrypto

logger: 
	$(CXX) logger.cpp -o logger.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS) -lcrypto
	$(CXX) logger_cli.cpp -o logger_cli $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS) -lcrypto

logger_cli_win:
	$(CXX) logger_cli_win.cpp -o logger_cli_win $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS) -lws2_32 -lwsock32

clean:
	rm -f http_server console.cgi logger.cgi logger_cli
