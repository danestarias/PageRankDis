CC=g++ -std=c++11 -Wall -fopenmp -L/home/daniel90/zmq/lib -I/home/daniel90/zmq/include
LDFLAGS= -lpthread -lzmqpp -lzmq -lsodium -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system

all: server worker

server: server.cc
	$(CC) -o server server.cc $(LDFLAGS)

worker: worker.cc
	$(CC) -o worker worker.cc $(LDFLAGS)
	
clean:
	rm -f worker server

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/daniel90/zmq/lib 		debian
#export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/home/daniel90/zmq/lib 	IOS