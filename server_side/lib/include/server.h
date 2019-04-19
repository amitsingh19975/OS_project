#define PORT 8096
#define ONLINE 1
#define OFFLINE 0
#define CHATTING 2
#define MAX_CONN 10
#define MAX_LEN 100
#define MAX_MSG 2048
#define USERDIR "../users"
int server_init();
int user_init();
void user_reset(int);
int authenticate(char*, char*);
int search_user(char*);
void close_connection(int);
void close_recv_connection(int);
void close_send_connection(int);
int cat_online_user(char*, int);
void send_exit_message(int);
int is_online (char*);
void broadcast_to_online();
void close_empty_connections();
// Client processes
void *client_process_init(void *param);
void *client_process_chat(void *param);


