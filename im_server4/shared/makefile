COMPONENTS=network
TARGET_INC_PATH=../include
TARGET_LIB_PATH=../lib
INC_PATH=./include
LIB_PATH=./lib

all: dir
	@for d in $(COMPONENTS) ; do \
	make all -C $$d ; \
	echo "" ;\
	done
	@cp -r $(INC_PATH)/* $(TARGET_INC_PATH)
	@cp $(LIB_PATH)/*.o $(TARGET_LIB_PATH)
	@echo "" ; 

clean: dir
	@for d in $(COMPONENTS) ; do \
	make clean -C $$d ; \
	echo "" ;\
	done
	@echo ""
	@rm -rf $(INC_PATH)/*.h
	@rm -rf $(LIB_PATH)/*.o

dir:
	@if [ ! -d $(INC_PATH) ]; then mkdir $(INC_PATH); fi
	@if [ ! -d $(LIB_PATH) ]; then mkdir $(LIB_PATH); fi
