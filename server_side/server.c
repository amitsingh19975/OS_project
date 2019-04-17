#include "utility.h"
#include "server.h"

// Number of online users and total users
int online_users = 0;
int total_users;

struct User {
    // Username and password
    char username[MAX_LEN];
    char password[MAX_LEN];
    // ONLINE Or OFFLINE
    u_int8_t status;
    // Its socket connection
    int connection;
    // Socket connection that it wants to chat to
    int connected_to;
    // Lock
    pthread_mutex_t mu;
} *users; 

int main(int argc, char const *argv[])
{
    // Initialize server and inits the server_fd value
    if (server_init()) {
        eprint("Error Initializing Server");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int server_init() {
    int server_fd;
    // online_users = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    // Create a socket connection
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        return 1;
    }
    
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    // {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror("Error in binding address to socket");
        return 1;
    }
    // Put server in listen mode
    if (listen(server_fd, MAX_CONN) < 0)
    {
        perror("listen");
        return 1;
    }
    sprint("Server listenting at port %d", PORT); 
    if(user_init()){
        perror("User Init");
        return 1;
    }
    int in_connection;
    // Run the client process threads
    pthread_t client_conns[MAX_CONN];
    pthread_attr_t client_thread_attr[MAX_CONN];
    // Accept incomming connections and create a thread for the connections
    // Main server loop
    int con_count = 0;
    while (con_count <= MAX_CONN) {
        // TODO: Create a case to handle more that MAX connections
        if ((in_connection = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Error accepting connection");
            return 1;
        }
        // Create a child proccess to chech if the connection is closed or not
        pid_t childProcess = fork();
        if (childProcess == (pid_t)-1) {
	        perror("Unable to create new process for client connection");
	        return 1;
        }
        else if (childProcess == 0) {
            pthread_attr_init(&client_thread_attr[con_count]);
            pthread_create(&client_conns[con_count], &client_thread_attr[con_count], client_process_init, (void *)&in_connection);
            con_count++;
        }
        else {
            // struct pollfd pfd;
            // pfd.fd = in_connection;
            // pfd.events = POLLIN | POLLHUP;
            // pfd.revents = 0;
            // while (pfd.revents == 0) {
            //     // Call poll with a timeout of 100 ms
            //     if (poll(&pfd, 1, 100) > 0){
            //         // if result > 0 , this means that there is either data available on the socket
            //         // or the socket has been closed
            //         char buffer[32];
            //         if (recv(in_connection, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0) {
            //             // If receive returns 0 this means connection has been closed 
            //             // kill child process
            //             puts("A client dropped connection");
            //             for (int i = 0; i< total_users; i++) {
            //                 if (users[i].status == ONLINE && users[i].connection == in_connection) {
            //                     online_users--;
            //                     printf("Online Users: %d", online_users);
            //                     printf("User %s closed connection\n", users[i].username);
            //                     user_reset(i);
            //                 }
            //             }
            //             kill (childProcess, SIGKILL);
            //             waitpid(childProcess, NULL, WNOHANG);
			// 	        close(in_connection);
            //         }
            //     }
            // }

        }
    }
    // Closing server
    for (size_t j = 0; j < con_count; j++)
    {
        pthread_attr_destroy(&client_thread_attr[j]);
        pthread_join(client_conns[j], NULL);
    }
    for (size_t j = 0; j < total_users; j++)
    {
        // Close all the online users
        if (users[j].status == ONLINE) {
            close(users[j].connection);
        }
    }
    return 0;
}

void *client_process_init(void *param) {
    int connection = *(int *)param;
    pthread_t tid_send;
    pthread_t tid_recv;
    char first_contact[2048] = {0};
    char username[MAX_LEN] = {0};
    char password[MAX_LEN] = {0};
    char user_to[MAX_LEN] = {0};
    read(connection, first_contact, 2048);
    sscanf(first_contact, "%s : %s", username, password);
    trim(username);
    trim(password);
    int index = authenticate(username, password);
    // payload: /user n <users>
    char payload[1024] = {0};
    sprintf(payload, "/user %d ",online_users);
    char response_message[2048] = {0};
    if (index >= 0) {
        // Broadcast to all online users new payload
        cat_online_user(payload);
        broadcast_to_online(payload);
        sprint("User: %s connected", users[index].username);
        users[index].status = ONLINE;
        users[index].connection = connection;
        online_users++;
        send(connection, payload, 1024, 0);
        printf("Payload: %s\n", payload);
    }
    else if (index == -1) {
        sprintf(response_message, "%s : %s", "FAIL", "WRONG USERNAME OR PASSWORD");
        send(connection, response_message, 2048, 0);
        close(connection);
        pthread_exit(NULL);
    } else if (index == -2) {
        sprintf(response_message, "%s : %s", "FAIL", "ALREADY LOGGED IN");
        send(connection, response_message, 2048, 0);
        close(connection);
        pthread_exit(NULL);
    }
}

void broadcast_to_online (char payload[]) {
    int i;
    for (int i = 0; i<total_users; i++) {
        if (users[i].status == ONLINE) {
            send(users[i].connection, payload, 1024, 0);
        }
    }
}

void cat_online_user(char payload[]) {
    for (int i = 0; i<total_users; i++) {
        if (users[i].status == ONLINE) {
            strcat(payload, users[i].username);
            strcat(payload, " ");
        }
    }
} 

int authenticate(char user[], char pass[]) {
    int index = search_user(user);

    if (index == -1){
        return -1;
    }
    if (strcmp(users[index].password, pass)) {
        return -1;
    }
    if (is_online(user)){
        return -2;
    }
    return index;
}

int search_user(char user[]) {
    int i;
    for ( i = 0; i < total_users; i++)
    {
        if (!strcmp(user, users[i].username)){
            return i;
        }
    }
    return -1;
}

int is_online(char user[]) {
    int i;
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE && !strcmp(user, users[i].username)) {
            return 1;
        }
    }
    return 0;
}

void close_connection(int connection) {
    close(connection);
    online_users--;
}

int user_init() {
    total_users = 0;
    DIR *d;
    struct dirent *dir;
    d=opendir(USERDIR);
    if (!d){
        perror("Opening User Directory Error ");
        return 1;
    }
    while(dir = readdir(d)) {
        if (dir->d_name[0] != '.') {
            // Count total number of users
            total_users++;
        }
    }
    closedir(d);
    printf("Total users: %d\n",total_users);
    // Allcate that many users
    users = (struct User*) malloc(total_users * sizeof(struct User));
    user_reset(-1);
    // Populate user with username and password
    int count = 0;
    char temp[300] = {0};
    d=opendir(USERDIR);
    if (!d){
        perror("Opening User Directory Error ");
        return 1;
    }
    while(dir = readdir(d)) {
        if (dir->d_name[0] != '.') {
            if (count < total_users) {
                strcpy(users[count].username, dir->d_name);
                sprintf(temp, "%s/%s", USERDIR, dir->d_name);
                int ufd = open(temp, O_RDONLY);
                memset(temp, 0, 300);
                if (read(ufd, temp, 300) > 0) {
                    trim(temp);
                    strcpy(users[count].password, temp);
                }
                memset(temp, 0, 300);
                close(ufd);
                count++;
            }
        }
    }
    closedir(d);
    // Show all initialized users
    puts("Initialized Users");
    for (size_t i = 0; i < total_users; i++)
    {
        puts(users[i].username);
    }
    return 0;
}

void user_reset(int index) {
    if (index == -1) {
        for (size_t i = 0; i < total_users; i++)
        {
            memset(users[i].username, 0, MAX_LEN);
            memset(users[i].password, 0, MAX_LEN);
            users[i].status = OFFLINE;
            users[i].connection = -1;
            users[i].connected_to = -1;
            pthread_mutex_init(&(users[i].mu), NULL);
        }
    }
    else {
        index = index % total_users;
        memset(users[index].username, 0, MAX_LEN);
        memset(users[index].password, 0, MAX_LEN);
        users[index].status = OFFLINE;
        users[index].connection = -1;
        users[index].connected_to = -1;
        pthread_mutex_init(&(users[index].mu), NULL);
    }
} 