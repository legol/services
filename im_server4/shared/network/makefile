include ../makefile.inc.lv1

TESTS=__tests__/epoll_event_test __tests__/framed_packet_test

all: epoll_event.o server_thread.o framed_transport.o client_thread.o framed_packet_receiving.o framed_packet_sending.o $(TESTS)
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

# test epoll event
OBJS_EPOLL_EVENT_TEST = __tests__/epoll_event_test.o
__tests__/epoll_event_test:$(OBJS_EPOLL_EVENT_TEST)
	$(DT) epoll_event.o /usr/lib/libgtest.a -lrt -lcrypto -ldl -lz -lpthread
	@echo ""

# test framed packet
OBJS_FRAMED_PACKET_TEST = __tests__/framed_packet_test.o
__tests__/framed_packet_test:$(OBJS_FRAMED_PACKET_TEST)
	$(DT) epoll_event.o /usr/lib/libgtest.a -lrt -lcrypto -ldl -lz -lpthread
	@echo ""

clean:
	@rm -rf  *.o
	@rm -rf $(all)
	@rm -rf  __tests__/*.o
	@rm -rf $(TESTS)
