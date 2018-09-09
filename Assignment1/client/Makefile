CFLAGSCONSTRERROR= -D_REENTRANT -D_THREAD_SAFE -D_POSIX_C_SOURCE=200112L
CFLAGS=-D_REENTRANT -D_THREAD_SAFE 

LIBRARIES=-lpthread

all: 	 client.exe

client.exe:  client.o
	gcc ${CFLAGS} -o client.exe client.o ${LIBRARIES}

client.o: client.c
	gcc -c ${CFLAGS} client.c 

.PHONY:	clean

clean:
	-rm -f client.o client.exe