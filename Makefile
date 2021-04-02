CFLAGS= -O2 -Wall

all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o
	gcc -o server $^

main.o: main.c httpd.h
	gcc $(CFLAGS) -c -o $*.o $*.c

httpd.o: httpd.c httpd.h
	gcc $(CFLAGS) -c -o $*.o $*.c

