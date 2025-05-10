BIN_FILES_SERVER  = servidor/servidor rpc/servidor-rpc

CC = gcc

LDLIBS_SERVER = -lpthread -lm -ldl -ltirpc
LDLIBS_FLAG = -fPIC 	# Para compilar los archivos .c de las librerías dinámicas
CFLAGS += -g -I/usr/include/tirpc
CPPFLAGS += -D_REENTRANT


all: 
	make rpc_logger_xdr
	make proxy-rpc
	make servidor-rpc
	make servidor

rpc_logger_xdr:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@.o $@.c 

proxy-rpc: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o servidor/rpc_logger_clnt.o servidor/rpc_logger_clnt.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o servidor/rpc_logger_client.o servidor/rpc_logger_client.c 

servidor-rpc: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o rpc/rpc_logger_svc.o rpc/rpc_logger_svc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o rpc/rpc_logger_server.o rpc/rpc_logger_server.c
	$(CC) $(CFLAGS) -o rpc/$@ rpc/rpc_logger_svc.o rpc/rpc_logger_server.o rpc_logger_xdr.o  $(LDLIBS_SERVER) 

servidor: servidor/servidor.o
	$(CC) -o servidor/$@ servidor/$@.c servidor/protocol.c servidor/lines.c servidor/manage_platform.c servidor/rpc_logger_clnt.o servidor/rpc_logger_client.o rpc_logger_xdr.o $(LDLIBS_SERVER) 


clean: 
	make clean-server
	rm -rf active_users registered_users


clean-server:
	rm -f $(BIN_FILES_SERVER) servidor/*.o rpc/*.o
