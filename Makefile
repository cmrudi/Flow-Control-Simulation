#Makefile for Tucil 2 IF3130 Jaringan Komputer

#Compiler Base
CC = g++

#Library used by program
LIBS = -pthread -std=c++11

all: receiver transmitter

receiver: src/receiver.cpp
	$(CC) src/receiver.cpp src/dcomm.h -o bin/receiver $(LIBS) 

transmitter: src/transmitter.cpp
	$(CC) src/transmitter.cpp src/dcomm.h -o bin/transmitter $(LIBS)

#clean these files
clean:
	rm -f bin/transmitter bin/receiver bin/transmitter.o bin/receiver.o

