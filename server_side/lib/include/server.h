#define PORT 8096
#define ONLINE 1
#define OFFLINE 0
#define MAX_CONN 10
#define MAX_LEN 100
#define USERDIR "users"
int server_init(int *);
int user_init();
void user_reset(int);

// Client processes
void *client_process_init(void *param);
void *client_process_send(void *param);
void *client_process_recv(void *param);

