#define PORT 8096
#define ONLINE 1
#define OFFLINE 0
#define MAX_CONN 10
#define MAX_LEN 100
#define MAX_MSG 2048
#define USERDIR "../users"
int server_init(int *);
int user_init();
void user_reset(int);
int authenticate(char*, char*);
int search_user(char*);
void close_connection(int);
void cat_online_user(char*, char*);

// Client processes
void *client_process_init(void *param);
void *client_process_send(void *param);
void *client_process_recv(void *param);

