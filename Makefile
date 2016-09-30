CFLAGS = -g -fpic
CC = gcc
LIBS = -lpthread 
OBJ_FILES = log_functions.o comm_work_queue.o comm_server.o comm_client.o comm_tcp_client.o comm_unix_client.o
HEADER_FILES = comm_domain.h comm_i.h comm_work_queue.h
ifeq ($(PLATFORM), ubuntu)
        LIBDIR = /usr/lib/x86_64-linux-gnu
else
        LIBDIR = /usr/lib64
endif
VERSION = 0.1

all: $(OBJ_FILES)
	$(CC) -shared -Wl,-soname,libcomm.so -o libcomm.so.$(VERSION) $(OBJ_FILES) $(LIBS)

clean:
	rm -f $(OBJ_FILES) libcomm.so*

install: all
	ln -sf libcomm.so.$(VERSION) libcomm.so
	cp -f libcomm.so.$(VERSION) $(LIBDIR)
	cp -f libcomm.so $(LIBDIR)
	cp -f $(HEADER_FILES) /usr/include/

uninstall:
	rm -f /usr/lib64/libcomm.so.$(VERSION) /usr/lib64/libcomm.so
	cd /usr/include/ && rm -f $(HEADER_FILES)
