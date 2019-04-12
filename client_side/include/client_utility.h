#ifndef CLIENT_UTILITY_H
#define CLIENT_UTILITY_H

#include <iostream>
#include <vector>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "terminal_raw.h"
#include "color.h"
#include "string_utility.h"
#include <map>

namespace chat_utility{

    struct SocketConnection;

    struct User{
        
        User():m_is_init(false){}
        User(std::string& user, std::string& pass):m_username(std::move(user)),m_password(std::move(pass)),m_is_init(true){
            assert(m_username.size() < 1024);
            assert(m_password.size() < 1024);
        }
        User(std::string& user):m_username(std::move(user)),m_is_init(true){
            assert(m_username.size() < 1024);
            m_password = std::string(getpass(""));
            assert(m_password.size() < 1024);
        }

        constexpr auto is_set() const noexcept{
            return m_is_init;
        }

        auto get_user() const noexcept{
            return m_username;
        }

    private:
        friend struct SocketConnection;
        bool                                m_is_init{false};
        std::string                         m_username;
        std::string                         m_password;
    };

    struct SocketConnection{

        SocketConnection() = default;
        SocketConnection(terminal::Terminal const& t):m_ter(t){}
        SocketConnection(User& user):m_user(std::move(user)),m_fd(-1){}
        auto set_user(User& user) noexcept;
        auto conn()     noexcept;
        auto conn_to(uint32_t)  noexcept;
        auto send()     noexcept;
        auto recv()     noexcept;
        auto login()    noexcept;
        [[nodiscard]] constexpr auto get_user_list() const noexcept
            ->std::map<uint32_t,std::string> const&;

    private:
        int                                 m_fd{-1};
        User                                m_user;
        terminal::Terminal                  m_ter;
        std::mutex                          m_mu;
        std::map<uint32_t,std::string>      m_list;
    };

    auto SocketConnection::conn() noexcept{
        sockaddr_in client;

        m_fd = socket(AF_INET,SOCK_STREAM,0);

        client.sin_family = AF_INET;
        client.sin_port = htons(8096);

        inet_pton(AF_INET, "127.0.0.1", &client.sin_addr);
        return connect(m_fd,reinterpret_cast<sockaddr*>(&client), sizeof(client));
    }

    auto SocketConnection::conn_to(uint32_t idx) noexcept{
        if(m_fd == -1){
            m_ter.eprint("Not Connected to Server!\r\n");
            return 0;
        }
        char payload[2048]  = {0};
        char buff[2048]     = {0};
        size_t len = sprintf(payload,"%s",m_list.at(idx).c_str());
        write(m_fd,payload,len);
        len = read(m_fd,buff,2048);
        char type[10] = {0};
        char what[2038] = {0};
        sscanf(buff,"%s : %s",type,what);
        if(strcmp(type,"Success")){
            m_ter.sprint(buff);
        }else{
            m_ter.eprint(buff);
        }
        return 1;
    }

    auto SocketConnection::login() noexcept{
        if(m_fd == -1){
            m_ter.eprint("Not Connected to Server!\r\n");
            return 0;
        }
        char payload[2048]  = {0};
        char buff[2048]     = {0};
        size_t len = sprintf(payload,"%s : %s",m_user.m_username.c_str(),m_user.m_password.c_str());
        write(m_fd,payload,len);
        len = read(m_fd,buff,2048);
        m_user.m_password.clear();
        char type[10] = {0};
        char what[2038] = {0};
        sscanf(buff,"%s : %s",type,what);
        if(strcmp(type,"Success")){
            m_ter.sprint(buff);
        }else{
            m_ter.eprint(buff);
        }
        return 1;
    }

    auto SocketConnection::set_user(User& user) noexcept{
        assert(user.is_set());
        m_user = std::move(user);
    }

    [[nodiscard]] constexpr auto SocketConnection::get_user_list() const noexcept
        ->std::map<uint32_t,std::string> const&{
        return m_list;
    } 

    auto SocketConnection::send() noexcept{
        if(m_fd != -1){
            m_ter.eprint("No Server found!\r\n");
            return;
        }
        
    }
}

#endif // CLIENT_UTILITY_H
