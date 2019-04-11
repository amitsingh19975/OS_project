#ifndef CLIENT_UTILITY_H
#define CLIENT_UTILITY_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "terminal_raw.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace chat_utility{

    struct SocketConnection{


    private:
        std::mutex mu;
    };

}


#endif // CLIENT_UTILITY_H
