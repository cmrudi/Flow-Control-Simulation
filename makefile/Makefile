#Makefile for Tucil 2 IF3130 Jaringan Komputer

#directory for program sources
VPATH = ../src
#diretory for bin 
BUILDDIR = ../bin

#Base Compiler
CC = g++

#Library used by program
LIBS = -pthread -std=c++11

all: receiver transmitter

receiver: receiver.cpp
	$(CC) receiver.cpp dcomm.h -o receiver $(LIBS)

transmitter: transmitter.cpp
	$(CC) transmitter.cpp -o transmitter $(LIBS)

#clean these files
clean:
	rm -f transmitter receiver transmitter.o receiver.o

