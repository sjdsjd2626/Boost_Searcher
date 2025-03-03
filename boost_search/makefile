all:parser searcher http-server
parser:parser.cc
	g++ -o $@ $^ -std=c++11 -lboost_system -lboost_filesystem
searcher:searcher.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp
http-server:http-server.cc
	g++ -o $@ $^ -std=c++11 -lpthread -ljsoncpp
.PHONY:clean
clean:
	rm -f parser searcher http-server