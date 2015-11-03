FLAGS = -Wall -std=c++11
CC = g++
CFLAGS = $(FLAGS)
LIBS=-lGL -lGLU -lSDL2 -lGLEW
VERSION=-std=c++11

test: main.o mesh.o
	$(CC) $(CFLAGS) -o $@  main.o mesh.o $(LIBS) 

all:
	g++ -o out main.cpp mesh.cpp $(VERSION) $(LIBS)
