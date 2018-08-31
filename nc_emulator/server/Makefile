CFLAGSCONSTRERROR= -D_REENTRANT -D_THREAD_SAFE -D_POSIX_C_SOURCE=200112L
CFLAGS=-D_REENTRANT -D_THREAD_SAFE 
LIBRARIES=-lpthread

all: 	server.exe

server.exe: server.o DBGpthread.o 
	gcc ${CFLAGS} -o server.exe server.o DBGpthread.o ${LIBRARIES}

server.o: server.c DBGpthread.h printerror.h
	gcc -c ${CFLAGS} server.c

DBGpthread.o: DBGpthread.c printerror.h
	gcc ${CFLAGS} -c DBGpthread.c

.PHONY:	clean

clean:
	-rm -f server.o sever.exe