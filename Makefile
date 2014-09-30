all: webserver clean

webserver: webserver.o socket.o
		g++ -lpthread webserver.o socket.o -o webserver

webserver.o: webserver.cpp
		g++ -c -lpthread webserver.cpp

socket.o: socket.cpp
		g++ -c socket.cpp

clean:
		rm -rf *.o
