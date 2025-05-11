BIN_FILES_SERVER  = servidor/servidor rpc/servidor-rpc

CC = gcc

LDLIBS_SERVER = -lpthread -lm -ldl -ltirpc
CFLAGS += -g -I/usr/include/tirpc
CPPFLAGS += -D_REENTRANT
SERVER_DIR = servidor
RPC_DIR = rpc


all: 
	make rpc_logger_xdr
	make proxy-rpc
	make servidor-rpc
	make servidor

rpc_logger_xdr:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@.o $@.c 

proxy-rpc: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $(SERVER_DIR)/rpc_logger_clnt.o $(SERVER_DIR)/rpc_logger_clnt.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $(SERVER_DIR)/rpc_logger_client.o $(SERVER_DIR)/rpc_logger_client.c 

servidor-rpc: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $(RPC_DIR)/rpc_logger_svc.o $(RPC_DIR)/rpc_logger_svc.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $(RPC_DIR)/rpc_logger_server.o $(RPC_DIR)/rpc_logger_server.c
	$(CC) $(CFLAGS) -o $(RPC_DIR)/$@ $(RPC_DIR)/rpc_logger_svc.o $(RPC_DIR)/rpc_logger_server.o rpc_logger_xdr.o  $(LDLIBS_SERVER) 

servidor: $(SERVER_DIR)/servidor.o
	$(CC) -o $(SERVER_DIR)/$@ $(SERVER_DIR)/$@.c $(SERVER_DIR)/protocol.c $(SERVER_DIR)/lines.c $(SERVER_DIR)/manage_platform.c $(SERVER_DIR)/rpc_logger_clnt.o $(SERVER_DIR)/rpc_logger_client.o rpc_logger_xdr.o $(LDLIBS_SERVER) 


clean: 
	make clean-server
	rm -rf active_users registered_users


clean-server:
	rm -f $(BIN_FILES_SERVER) servidor/*.o rpc/*.o
