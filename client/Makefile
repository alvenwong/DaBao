DIR_DES = DES

OBJ += client.o \
	  payload.o \
	  socket_tools.o \
	  serialize.o \
	  client_config.o \
	  statistics.o \
	  epoll.o \
	  utils.o

OBJ += $(DIR_DES)/cipher.o \
	   $(DIR_DES)/payload_cipher.o

LIB += -lpthread
LIB += -lconfig

CFLAGS += -g

all: client cleanobj
client: $(OBJ) 
	@cc $(CFLAGS) -o client $(OBJ) $(LIB)

.PHONY: cleanobj
cleanobj:
	rm -f *.o
	rm -f $(DIR_DES)/*.o

.PHONY: clean
clean:
	rm -f *.o client
