all: webserver clean

webserver: webserver.o socket.o
		g++ webserver.o socket.o -o webserver

webserver.o: webserver.cpp
		g++ -c webserver.cpp

socket.o: socket.cpp
		g++ -c socket.cpp

clean:
		rm -rf *.o
