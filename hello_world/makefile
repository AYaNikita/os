all: hello_world
hello_world.o: hello_world.c
	gcc hello_world.c -o hello_world.o -c
hello_world: hello_world.o
	gcc -s hello_world.o -o hello_world
clean: 
	rm -f hello_world.o hello_world