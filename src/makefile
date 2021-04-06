
CFLAGS = -O2 -Wall -DDEBUG=DBG_TEST -I../utl -I.
LNFLAGS = -L../utl -L.
LIBS = -lutl -lhttpd

.c.o:
	gcc $(CFLAGS) -c -o $*.o $*.c

all: ../server

# Server
../server: ../utl/libutl.a libhttpd.a server.o
	gcc $(LNFLAGS) -o $@ $^ $(LIBS)

server.o: server.c httpd.h

# HTTPD library
request.o: request.c httpd.h
listener.o: listener.c httpd.h

libhttpd.a: listener.o request.o	
	ar -r $@ $^ 

## UTL (mini) library

../utl/libutl.a: 
	cd ../utl ; make

clean:
	rm -rf *.o libhttpd.a
	rm -rf ../server ./server

cleanall:	
	make clean
	cd ../utl; make clean

