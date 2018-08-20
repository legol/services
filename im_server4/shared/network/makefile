include ../makefile.inc.lv1

all: epoll_event.o server_thread.o framed_transport.o client_thread.o
	@cp *.h $(TARGET_INC_PATH) ; cp *.o $(TARGET_LIB_PATH)

#epoll.o:epoll.c
#	cc -g -Wall -o epoll.o -c epoll.c

%.o:%.cpp
	$(DC)

%.o:%.cc
	$(DC)

%.o:%.c
	$(DC)

%.o:%.cpp %.h
	$(DC)

%.o:%.cc %.h
	$(DC)

%.o:%.c %.h
	$(DC)


clean:
	@rm -rf  *.o
	@rm -rf $(all)
