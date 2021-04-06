CFLAGS = -O2

.c.o:
	gcc $(CFLAGS) -c -o $*.o $*.c

libutl.a: dbg.o buf.o
	cat vrg.h dbg.h buf.h > utl.h
	ar -r $@ $^

clean:
	rm -rf *.o
	rm -rf libutl.a utl.h