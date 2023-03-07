all:build

build:main.c
	gcc -Wall main.c -o scheduler
run:build
	./scheduler input output
clean:
	rm -f scheduler
