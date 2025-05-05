BIN_FILES_SERVER  = servidor 

CC = gcc

LDLIBS_SERVER = -lpthread -lm 
LDLIBS_FLAG = -fPIC 	# Para compilar los archivos .c de las librerías dinámicas

all: 
	make client
	make servidor


servidor: servidor.o
	$(CC) -o $@ $@.c protocol.c lines.c manage_platform.c $(LDLIBS_SERVER) 

client: client.o
	$(CC) -o $@ $@.c lines.c $(LDLIBS_SERVER) 


clean: 
	make clean-server
	rm -f tuples


clean-server:
	rm -f $(BIN_FILES_SERVER) *.o *.so
