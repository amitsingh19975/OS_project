#include "utility.h"
#include "server.h"

// Number of online users and total users
int online_users = 0;
int con_count = 0;
int total_users;

struct User
{
    // Username and password
    char username[MAX_LEN];
    char password[MAX_LEN];
    // ONLINE Or OFFLINE
    u_int8_t status;
    // Its socket connection
    int connection;
    // Socket connection that it wants to chat to
    int recv_con_index[MAX_CONN];
    // Other connection index
    int send_con_index[MAX_CONN];
    int recv_len;
    int send_len;
    int perm[2];
    // Lock
    pthread_mutex_t mu;
} * users;

void graceful_exit(void)
{
    puts("Server Closing");
}

int main(int argc, char const *argv[])
{
    atexit(graceful_exit);
    // Initialize server and inits the server_fd value
    if (server_init())
    {
        eprint("Error Initializing Server");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int server_init()
{
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
    if (user_init())
    {
        perror("User Init");
        return 1;
    }
    int in_connection;
    // Run the client process threads
    pthread_t client_conns[MAX_CONN];
    pthread_attr_t client_thread_attr[MAX_CONN];
    // Accept incomming connections and create a thread for the connections
    // Main server loop
    while (con_count <= MAX_CONN)
    {
        // TODO: Create a case to handle more that MAX connections
        puts("Accepting connection");
        if ((in_connection = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Error accepting connection");
            return 1;
        }
        // Create a child proccess to chech if the connection is closed or not
        pthread_attr_init(&client_thread_attr[con_count]);
        pthread_create(&client_conns[con_count], &client_thread_attr[con_count], client_process_init, (void *)&in_connection);
        con_count++;
        printf("Number of connections %d \n", con_count);
    }
    // Closing server
    for (size_t j = 0; j < con_count; j++)
    {
        pthread_attr_destroy(&client_thread_attr[j]);
        pthread_join(client_conns[j], NULL);
    }
    return 0;
}

void *client_process_init(void *param)
{
    int connection = *(int *)param;
    pthread_t tid_chat;
    char first_contact[2048] = {0};
    char username[MAX_LEN] = {0};
    char password[MAX_LEN] = {0};
    char user_to[MAX_LEN] = {0};
    if (read(connection, first_contact, 2048) < 0)
    {
        // If connection drops before authentication
        con_count--;
        close(connection);
        pthread_exit(NULL);
    }
    sscanf(first_contact, "%s : %s", username, password);
    trim(username);
    trim(password);
    int index = authenticate(username, password);
    char response_message[2048] = {0};
    if (index >= 0)
    {
        sprint("User: %s connected", users[index].username);
        users[index].status = ONLINE;
        users[index].connection = connection;
        online_users++;
        // Broadcast to all online users new payload
        broadcast_to_online();
        // Accept user_to be connected to.
        // user_to can accept either of two values
        // 1-- user_to has username when client sends a request to be
        //     connected to another user
        // 2-- user_to can accept a request of form /perm <y,n> /user <username>
        char user_to[100] = {0};
        if (read(connection, user_to, 100) < 0)
        {
            pthread_mutex_lock(&users[index].mu);
            close_connection(index);
            pthread_mutex_unlock(&users[index].mu);
            pthread_exit(NULL);
        }
        else if (user_to[0] == '/') {
            // If it is on receiving end of permission then
            // user_to will be a /perm y or n
            // If permission is n then send
            char perm_cpy[100] = {0};
            strcpy(perm_cpy, user_to);
            perm_cpy[5] = 0;
            printf("User To value for %s: %s: ", users[index].username, user_to);
            if (!strcmp(perm_cpy, "/perm")) {
                // Wait for connection request to be made by other thread
                while (users[index].perm[0] == -1);
                // Check if the user response is y or n
                if (user_to[6] == 'y') {
                    // Response is yes set permission to 1
                    printf("User %s granting permission\n", users[index].username);
                    users[index].perm[1] = 1;
                    // Create connection with user that is requesting permission
                    online_users--;
                    users[index].status = CHATTING;
                    users[index].recv_len++;
                    users[index].recv_con_index[users[index].recv_len-1] = users[index].perm[0];
                    users[index].send_len++;
                    users[index].send_con_index[users[index].send_len-1] = users[index].perm[0];
                    broadcast_to_online();
                    // TODO: Create a thread to chat
                    pthread_create(&tid_chat, NULL, client_process_chat, (void *)&index);
                    pthread_join(tid_chat, NULL);
                    printf("%s Exiting chat thread", users[index].username);
                    pthread_mutex_lock(&users[index].mu);
                    close_connection(index);
                    pthread_mutex_unlock(&users[index].mu);
                    pthread_exit(NULL);
                } else {
                    // Response is no, set permission to 0
                    users[index].perm[1] = 0;
                    // Close connection and exit
                    pthread_mutex_lock(&users[index].mu);
                    close_connection(index);
                    pthread_mutex_unlock(&users[index].mu);
                    pthread_exit(NULL);
                }

            } else {
                // Wrong requset send close connection
                pthread_mutex_lock(&users[index].mu);
                close_connection(index);
                pthread_mutex_unlock(&users[index].mu);
                pthread_exit(NULL);
            }
        }
        else
        {
            int other_index = search_user(user_to);
            // TODO: Check if user is online
            char permission[MAX_MSG] = "/perm /user ";
            strcat(permission, users[other_index].username);
            printf("Permission message: %s\n", permission);
            send(users[other_index].connection, permission, MAX_MSG, 0);
            memset(permission, 0, MAX_MSG);
            // Check if a permission was alread made
            if (users[other_index].perm[0] == -1) {
                // Make a request for permission by setting the perm[0] to the current index
                users[other_index].perm[0] = index;
                // Wait for the permission to be accepted
                while (users[other_index].perm[1] == -1);
                if (users[other_index].perm[1] == 1) {
                    // Request accepted proceed to create a connection
                    online_users--;
                    users[index].status = CHATTING;
                    users[index].recv_len++;
                    users[index].recv_con_index[users[index].recv_len-1] = other_index;
                    users[index].send_len++;
                    users[index].send_con_index[users[index].send_len-1] = other_index;
                    broadcast_to_online();
                    // Send accepted to client so that it can proceed to chat room
                    sprintf(permission, "%s", "/perm ACCEPTED");
                    send(users[index].connection, permission, MAX_MSG, 0);
                    // TODO: Create a thread to chat
                    pthread_create(&tid_chat, NULL, client_process_chat, (void *)&index);
                    pthread_join(tid_chat, NULL);
                    printf("%s Exiting chat thread", users[index].username);
                    pthread_mutex_lock(&users[index].mu);
                    close_connection(index);
                    pthread_mutex_unlock(&users[index].mu);
                    pthread_exit(NULL);
                } else {
                    // Request denied send request denined and close the connection
                    sprintf(permission, "%s", "/perm DENIED");
                    send(users[index].connection, permission, MAX_MSG, 0);
                    pthread_mutex_lock(&users[index].mu);
                    close_connection(index);
                    pthread_mutex_unlock(&users[index].mu);
                    pthread_exit(NULL);
                }
            } else {
                // Requested user already got a request send permission denied and close connection
                sprintf(permission, "%s", "/perm DENIED");
                send(users[index].connection, permission, MAX_MSG, 0);
                pthread_mutex_lock(&users[index].mu);
                close_connection(index);
                pthread_mutex_unlock(&users[index].mu);
                pthread_exit(NULL);
            }
        }
    }
    else if (index == -1)
    {
        sprintf(response_message, "%s : %s", "FAIL", "WRONG USERNAME OR PASSWORD");
        send(connection, response_message, 2048, 0);
        close(connection);
        con_count--;
        pthread_exit(NULL);
    }
    else if (index == -2)
    {
        sprintf(response_message, "%s : %s", "FAIL", "ALREADY LOGGED IN");
        send(connection, response_message, 2048, 0);
        close(connection);
        con_count--;
        pthread_exit(NULL);
    }
}

void *client_process_chat(void *param)
{
    // FIXME: if other user drops the connection this user continues to read message
    // Drop this connection too if the other connections drops abruptly
    int index = *(int *)param;
    int to = users[index].send_con_index[users[index].send_len-1];
    char message[MAX_MSG];
    // If connection to either User drops then send /exit command to the other
    // Connected user so that he can go to menu screen.
    while (users[to].status == CHATTING)
    {
        int len = read(users[index].connection, message, 2048);
        if (len < 0)
        {
            // If the this connection drops then exit thread
            pthread_exit(NULL);
        }
        else
        {
            // See If it is an exit command
            if (message[0] == '/')
            {
                char ext_msg[2048];
                strcpy(ext_msg, message);
                ext_msg[5] = 0;
                if (!strcmp(ext_msg, "/exit"))
                {
                    printf("Exit requested by %s\n", users[index].username);
                    // Close both the connection
                    close_connection(to);
                    pthread_exit(NULL);
                }
            }
            write(users[to].connection, message, 2048);
        }
    }
}

void send_exit_message(int connection)
{
    char ext_command[2048] = {0};
    sprintf(ext_command, "%s", "/exit");
    write(connection, ext_command, 2048);
}

void broadcast_to_online()
{
    puts("Broadcasting to online users");
    char payload[1024] = {0};
    int i, count = 0;
    char buffer[1010] = {0};
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE)
        {
            count = cat_online_user(buffer, i);
            sprintf(payload, "/sync %d %s", count, buffer);
            write(users[i].connection, payload, 1024);
            memset(buffer, 0, 1010);
            memset(payload, 0, 1024);
        }
    }
}

int cat_online_user(char payload[], int except)
{
    int count = 0;
    int i;
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE && i != except)
        {
            strcat(payload, users[i].username);
            strcat(payload, " ");
            count++;
        }
    }
    return count;
}

int authenticate(char user[], char pass[])
{
    int index = search_user(user);

    if (index == -1)
    {
        return -1;
    }
    if (strcmp(users[index].password, pass))
    {
        return -1;
    }
    if (is_online(user))
    {
        return -2;
    }
    return index;
}

int search_user(char user[])
{
    int i;
    for (i = 0; i < total_users; i++)
    {
        if (!strcmp(user, users[i].username))
        {
            return i;
        }
    }
    return -1;
}

int is_online(char user[])
{
    int i;
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE && !strcmp(user, users[i].username))
        {
            return 1;
        }
    }
    return 0;
}

void close_connection(int index)
{
    online_users--;
    printf("User %s logged out\n", users[index].username);
    send_exit_message(users[index].connection);
    close(users[index].connection);
    // Close all the recieving and sending connections
    users[index].status = OFFLINE;
    int i;
    for (i = 0; i < users[index].recv_len; i++)
    {
        close_send_connection(users[index].recv_con_index[i]);
    }
    for (i = 0; i < users[index].send_len; i++)
    {
        close_recv_connection(users[index].send_con_index[i]);
    }
    // If any connection is now empty on both send and recv close that connection
    // close_empty_connections();
    broadcast_to_online();

    user_reset(index);
    con_count--;
}

void close_recv_connection(int index)
{
    int i, j, k;
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE)
        {
            for (j = 0; j < users[i].recv_len; j++)
            {
                if (users[i].recv_con_index[j] == index)
                {
                    if (users[i].recv_len == 1 || j == users[i].recv_len - 1)
                    {
                        users[i].recv_len--;
                        break;
                    }
                    for (k = j + 1; k < users[i].recv_len; k++)
                    {
                        users[i].recv_con_index[k - 1] = users[i].recv_con_index[k];
                    }
                    users[i].recv_len--;
                    break;
                }
            }
        }
    }
}

void close_send_connection(int index)
{
    int i, j, k;
    for (i = 0; i < total_users; i++)
    {
        if (users[i].status == ONLINE)
        {
            for (j = 0; j < users[i].send_len; j++)
            {
                if (users[i].send_con_index[j] == index)
                {
                    if (users[i].send_len == 1 || j == users[i].send_len - 1)
                    {
                        users[i].send_len--;
                        break;
                    }
                    for (k = j + 1; k < users[i].send_len; k++)
                    {
                        users[i].send_con_index[k - 1] = users[i].send_con_index[k];
                    }
                    users[i].send_len--;
                    break;
                }
            }
        }
    }
}

int user_init()
{
    total_users = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(USERDIR);
    if (!d)
    {
        perror("Opening User Directory Error ");
        return 1;
    }
    while (dir = readdir(d))
    {
        if (dir->d_name[0] != '.')
        {
            // Count total number of users
            total_users++;
        }
    }
    closedir(d);
    printf("Total users: %d\n", total_users);
    // Allcate that many users
    users = (struct User *)malloc(total_users * sizeof(struct User));
    user_reset(-1);
    // Populate user with username and password
    int count = 0;
    char temp[300] = {0};
    d = opendir(USERDIR);
    if (!d)
    {
        perror("Opening User Directory Error ");
        return 1;
    }
    while (dir = readdir(d))
    {
        if (dir->d_name[0] != '.')
        {
            if (count < total_users)
            {
                strcpy(users[count].username, dir->d_name);
                sprintf(temp, "%s/%s", USERDIR, dir->d_name);
                int ufd = open(temp, O_RDONLY);
                memset(temp, 0, 300);
                if (read(ufd, temp, 300) > 0)
                {
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

void user_reset(int index)
{
    if (index == -1)
    {
        for (size_t i = 0; i < total_users; i++)
        {
            memset(users[i].username, 0, MAX_LEN);
            memset(users[i].password, 0, MAX_LEN);
            users[i].status = OFFLINE;
            users[i].connection = -1;
            for (size_t j = 0; j < MAX_CONN; j++)
            {
                users[i].recv_con_index[j] = -1;
                users[i].send_con_index[j] = -1;
            }
            users[i].perm[0] = -1;
            users[i].perm[1] = -1;
            users[i].recv_len = 0;
            users[i].send_len = 0;
            pthread_mutex_init(&(users[i].mu), NULL);
        }
    }
    else
    {
        index = index % total_users;
        users[index].status = OFFLINE;
        users[index].connection = -1;
        for (size_t j = 0; j < MAX_CONN; j++)
        {
            users[index].recv_con_index[j] = -1;
            users[index].send_con_index[j] = -1;
        }
        users[index].recv_len = 0;
        users[index].send_len = 0;
        users[index].perm[0] = -1;
        users[index].perm[1] = -1;

    }
}