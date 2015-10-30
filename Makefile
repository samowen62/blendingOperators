FLAGS = -Wall -std=c++11
CC = g++
CFLAGS = $(FLAGS)
LIBS=-lSDL2 -lGL -lGLU
VERSION=-std=c++11

#works
#g++ -o out main.cpp mesh.cpp -std=c++11 -lGL -lGLU -lSDL2 -lGLEW

first:
	$(CC) main.cpp -o out $(LIBS) $(VERSION)

test: main.o mesh.o
	$(CC) $(CFLAGS) -o $@  main.o mesh.o $(LIBS) 

#i think works below
all:
	g++ -c -o main.o main.cpp -std=c++11 -lGL -lGLU
	g++ -c -o mesh.o mesh.cpp -std=c++11 -lGL -lGLU
	g++ -std=c++11 -o out main.o mesh.o -lGL -lGLU
