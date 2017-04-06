DIR_REDIS = redis
DIR_DES = DES

OBJ += server.o \
	  server_config.o \
	  payload.o \
	  socket_tools.o \
	  serialize.o \
	  statistics.o

OBJ += $(DIR_REDIS)/crc16.o \
	   $(DIR_REDIS)/connect.o \
	   $(DIR_REDIS)/redis.o

OBJ += $(DIR_DES)/cipher.o \
	   $(DIR_DES)/payload_cipher.o


LIB += -lpthread
LIB += -lconfig
LIB += -lhiredis
CFLAGS += -g

TARGET = server

all : server cleanobj 
server : $(OBJ)
	@cc $(CFLAGS) -o $(TARGET) $(OBJ) $(LIB)
#redis.o : redis.h

.PHONY: cleanobj
cleanobj:
	rm -f $(OBJ)

.PHONY: clean
clean:
	rm -f *.o server
