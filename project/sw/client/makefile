#use this compiler:
CC=gcc
CC_OPTIONS=-v -Wall -W -D_GNU_SOURCE -DMAX_NUM_OF_FILES=10 -DMAX_NUM_OF_LINES=1024 -DRECEIVE_BUFFER_SIZE=9000
TEST_OPTIONS=-DTEST_SERVER_ABORT
CC_DEBUG_OPTIONS=-v -g --save-temps -W -DGETPROTOBYNUMBER $(CC_OPTIONS) 
PROG_NAME=client
TARGET=$(PROG_NAME).run
SRC=$(PROG_NAME).c

#Create macro for header files' incude directory
#H_INCLUDES=./../../include
#Implements next wildcard: ./mylib/includes/h_files/*c
#H_SOURCE=$(wildcard $(H_INCLUDES)/*.h)


all: non_debug
alldebug: debug

debug:
	$(CC) $(CC_DEBUG_OPTIONS) $(SRC) -o $(TARGET)
#$(CC) $(CC_DEBUG_OPTIONS) $(SRC) ../source/parse_args.h -o $(TARGET)


non_debug:
	$(CC) $(CC_OPTIONS) $(SRC) -o $(TARGET)
  
test_abort: 
	$(CC) $(CC_OPTIONS) $(SRC) $(TEST_OPTIONS) -o $(TARGET)


clean : 
#rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	rm -f *.run *.i *.o *.s *.core
