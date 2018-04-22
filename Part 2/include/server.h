
/*Initializes the servers*/
int init_servers(int num_shards, int num_server_threads);

/*Server Task*/
void* server_t(void* ptr);
