
struct send_log_params {
    string username<256>;
    string operation<256>;
    string filename<256>;
    string date_time<22>;
};

program RPC_LOGGER {
   version RPC_LOGGER_VER {
        int send_log_rpc(struct send_log_params args) = 1;
   } = 1;
} = 100495924;
