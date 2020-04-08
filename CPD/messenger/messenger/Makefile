CXX = g++
CXXFLAGS = -c -std=c++11 -Wall -g

all: messenger_server messenger_client

messenger_server: messenger_server.o user.o common.o
	$(CXX) -o messenger_server -pthread messenger_server.o user.o common.o

messenger_client: messenger_client.o user.o common.o
	$(CXX) -o messenger_client -pthread messenger_client.o user.o common.o

messenger_server.o: messenger_server.cpp
	$(CXX) $(CXXFLAGS) messenger_server.cpp 

messenger_client.o: messenger_client.cpp
	$(CXX) $(CXXFLAGS) messenger_client.cpp

user.o: user.cpp user.h
	$(CXX) $(CXXFLAGS) user.cpp

common.o: common.cpp common.h
	$(CXX) $(CXXFLAGS) common.cpp

clean:
	rm -f *.o messenger_server messenger_client
