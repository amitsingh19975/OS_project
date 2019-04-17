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

#define MAX_BYTE 2048

namespace chat_utility{

    struct SocketConnection;

    struct User{
        
        User():m_is_init(false){}
        User(std::string& user, std::string& pass):m_username(std::move(user)),m_password(std::move(pass)),m_is_init(true){
            assert(m_username.size() < MAX_BYTE / 2);
            assert(m_password.size() < MAX_BYTE / 2);
        }
        User(std::string& user):m_username(std::move(user)),m_is_init(true){
            assert(m_username.size() < MAX_BYTE / 2);
            m_password = std::string(getpass(""));
            assert(m_password.size() < MAX_BYTE / 2);
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
        std::string                         m_conn_to;
        std::string                         m_mess_send;
        std::string                         m_mess_recv;
        std::mutex                          m_mu;
        std::map<uint32_t,std::string>      m_list;
        terminal::Terminal                  m_ter;
        User                                m_user;
        int                                 m_fd{-1};
        bool                                m_connected{true};
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
        char payload[MAX_BYTE]  = {0};
        char buff[MAX_BYTE]     = {0};
        char type[10] = {0};
        char what[MAX_BYTE - 10] = {0};
        m_conn_to = m_list.at(idx);
        size_t len = sprintf(payload,"%s",m_conn_to.c_str());
        
        write(m_fd,payload,len);
        len = read(m_fd,buff,MAX_BYTE);
        
        sscanf(buff,"%s : %s",type,what);
        
        if(strcmp(type,"SUCCESS") == 0){
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
        char payload[MAX_BYTE]  = {0};
        char buff[MAX_BYTE]     = {0};
        char type[10] = {0};
        char what[MAX_BYTE - 10] = {0};
        
        size_t len = sprintf(payload,"%s : %s",m_user.m_username.c_str(),m_user.m_password.c_str());
        auto temp = write(m_fd,payload,len);
        terminal::disable();
        std::cout<<payload<<"==>"<<temp<<'\n';
        exit(0);
        len = read(m_fd,buff, MAX_BYTE);
        m_user.m_password.clear();
        
        sscanf(buff,"%s : %s",type,what);
        
        if(strcmp(type,"SUCCESS") == 0){
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
            auto str = format<Bit_3_4<FG::RED>,TF::BOLD>("Connect to SERVER!\r\n");
            write(1,str.c_str(),str.size());
            return;
        }
        char payload[MAX_BYTE];
        std::string str;
        while(m_connected){
            if(read(0,payload,MAX_BYTE) == -1){
                m_connected = false;
                str = format<Bit_3_4<FG::RED>,TF::BOLD>("Server Got Disconnectd!\r\n");
                write(1,str.c_str(),str.size());
                break;
            }else{
                write(m_fd, payload, MAX_BYTE);
            }
        }
    }

    auto SocketConnection::recv() noexcept{
        if(m_fd != -1){
            auto str = format<Bit_3_4<FG::RED>,TF::BOLD>("Connect to SERVER!\r\n");
            write(1,str.c_str(),str.size());
            return;
        }
        char payload[MAX_BYTE];
        std::string str;
        while(m_connected){
            if(read(m_fd, payload, MAX_BYTE) == -1){
                m_connected = false;
                str = format<Bit_3_4<FG::RED>,TF::BOLD>("Server Got Disconnectd!\r\n");
                write(1,str.c_str(),str.size());
                break;
            }else{
                str = format<Bit_3_4<FG::BRIGHT_MAGENTA>,TF::BOLD>(m_conn_to) + ": " +std::string(payload) +"\r\n";
                write(1,str.c_str(),str.size());
            }
        }
    }
}

#endif // CLIENT_UTILITY_H
