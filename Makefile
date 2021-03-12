all: build run

build: main.c
	gcc main.c -o main
run: main
	./main