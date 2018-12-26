#include ../public/Makefile.def
MODULE_NAME=im_server

BINDIR=./output
TARGET=apps/server
TARGET1=apps/test_client
TARGET2=apps/echo_server
TARGET_TEST=__tests__/test
INCLUDE_PATH=./include
LIBRARY_PATH=./lib
SHARED=shared

GTEST_LIB=/home/ubuntu/git_root/googletest/lib
GMOCK_LIB=/home/ubuntu/git_root/googletest/lib
GTEST_INC=/home/ubuntu/git_root/googletest/googletest/include
GMOCK_INC=/home/ubuntu/git_root/googletest/googlemock/include

INC=-I$(INCLUDE_PATH) -I./ -I$(GTEST_INC) -I$(GMOCK_INC)
LIB=-L$(LIBRARY_PATH) -lrt -lcrypto -ldl -lz $(LIBRARY_PATH)/*.o -lboost_thread -lboost_regex -lboost_system -lpthread

#DC=g++ -g -Wall -DBUILD_TIME="$(BUILD_TIME)"  -DPROJECT_VERSION="$(PROJECT_BUILD_VERSION)" -DMODULE_VERSION="$(MODULE_BUILD_VERSION)" -DTAG_VERSION="$(TAG_LAST)" -o $@ -c $<
#DT=g++ -g -Wall -DBUILD_TIME="$(BUILD_TIME)"  -DPROJECT_VERSION="$(PROJECT_BUILD_VERSION)" -DMODULE_VERSION="$(MODULE_BUILD_VERSION)" -DTAG_VERSION="$(TAG_LAST)" -o $@ $^

DC=g++ -std=c++17 -std=gnu++17 -g -Wall -o $@ -c $<
DT=g++ -std=c++17 -std=gnu++17 -g -Wall -o $@ $^

AR = ar r
RANLIB = ranlib

release:sub_dir static_lib clrtarget $(TARGET) $(TARGET1) $(TARGET2) $(TARGET_TEST)
	cp $(TARGET) $(BINDIR)
	cp $(TARGET1) $(BINDIR)
	cp $(TARGET2) $(BINDIR)
	cp $(TARGET_TEST) $(BINDIR)

sub_dir: dir
#   -@mkdir include
#   -@mkdir lib
	@for d in $(SHARED) ; do \
	make -C $$d ; \
	echo "" ;\
	done

dir:
	@if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi
	@if [ ! -d $(LIBRARY_PATH) ]; then mkdir $(LIBRARY_PATH); fi

static_lib:
	$(AR) ./lib/lib$(MODULE_NAME).a ./lib/*.o
	$(RANLIB) ./lib/lib$(MODULE_NAME).a

clrtarget:
	-@rm -rf $(TARGET)
	-@rm -rf $(TARGET1)
	-@rm -rf $(TARGET_TEST)
	-@rm -rf $(TARGET2)

OBJS_APPS_SERVER = apps/server.o
apps/server:$(OBJS_APPS_SERVER)
	$(DT) $(LIB)
	@echo ""

OBJS_APPS_TEST_CLIENT = apps/test_client.o
apps/test_client:$(OBJS_APPS_TEST_CLIENT)
	$(DT) $(LIB)
	@echo ""

OBJS_TEST = __tests__/test.o __tests__/echo_server.o
__tests__/test:$(OBJS_TEST)
	$(DT) $(LIB) $(GTEST_LIB)/libgtest.a $(GMOCK_LIB)/libgmock.a
	@echo ""

OBJS_ECHO_SERVER = apps/echo_server.o
apps/echo_server:$(OBJS_ECHO_SERVER)
	$(DT) $(LIB)
	@echo ""

%.o:%.cpp
	$(DC) $(INC)

%.o:%.cc
	$(DC) $(INC)

%.o:%.c
	$(DC) $(INC)

%.o:%.cpp %.h
	$(DC) $(INC)

%.o:%.cc %.h
	$(DC) $(INC)

%.o:%.c %.h
	$(DC) $(INC)

clean: dir
	@for d in $(SHARED) ; do \
	make clean -C $$d ; \
	echo "" ;\
	done
	-@rm -rf $(INCLUDE_PATH)/*.h
	-@rm -rf $(INCLUDE_PATH)/json
	-@rm -rf $(LIBRARY_PATH)/*.o $(LIBRARY_PATH)/*.a
	-@rm -rf  *.o
	-@rm -rf  apps/*.o
	-@rm -rf $(TARGET)
	-@rm -rf $(TARGET1)
	-@rm -rf $(TARGET2)
	-@rm -rf $(TARGET_TEST)
	-@rm -rf output/*
