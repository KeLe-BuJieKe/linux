.PHONY:all	
all:tcp_client tcp_server

tcp_client:tcp_client.cc
	g++ -std=c++11 -o $@ $^

tcp_server:tcp_server.cc
	g++ -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm tcp_client tcp_server
