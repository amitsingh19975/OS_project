#ifndef CLIENT_UTILITY_H
#define CLIENT_UTILITY_H

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
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

namespace chat_utility{

    using header_color = Bit_3_4<FG::BLUE>;
    using menu_color = Bit_3_4<FG::GREEN>;

    struct SocketConnection;

    struct User{
        
        User():m_is_init(false){}
        User(std::string_view user, std::string_view pass):m_username(user),m_password(pass),m_is_init(true){
            assert(m_username.size() < 1024);
            assert(m_password.size() < 1024);
        }
        User(std::string_view user):m_username(user),m_is_init(true){
            m_password = getpass("");
            assert(m_username.size() < 1024);
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
        bool        m_is_init{false};
        std::string m_username;
        std::string m_password;
    };

    struct SocketConnection{

        SocketConnection(User& user):m_user(std::move(user)),m_fd(-1){}
        SocketConnection() = default;

        constexpr auto set_user(User& user) noexcept;
        auto conn() noexcept;
        auto send() const noexcept;
        auto recv() const noexcept;
        auto login() noexcept;

    private:
        int             m_fd;
        User            m_user;
        std::mutex      m_mu;
    };

    auto SocketConnection::conn() noexcept{
        sockaddr_in client;

        m_fd = socket(AF_INET,SOCK_STREAM,0);

        client.sin_family = AF_INET;
        client.sin_port = htons(8096);

        inet_pton(AF_INET, "127.0.0.1", &client.sin_addr);
        return connect(m_fd,reinterpret_cast<sockaddr*>(&client), sizeof(client));
    }

    auto SocketConnection::login() noexcept{
        char payload[2048]  = {0};
        char buff[2048]     = {0};
        size_t len = sprintf(payload,"%s : %s",m_user.m_username.c_str(),m_user.m_password.c_str());
        write(m_fd,payload,len);
        len = read(m_fd,buff,2048);
        m_user.m_password.clear();
    }

    constexpr auto SocketConnection::set_user(User& user) noexcept{
        assert(user.is_set());
        m_user = std::move(user);
    }

    auto uniform_padding(std::string& el, size_t m){
        auto len = el.size() < m ? m - el.size() : 0;
        len /= 2;
        el = std::string(len,' ') + el + std::string(len,' ');
    }

    auto main_menu(terminal::Terminal& t, int selected = 1){
        const int Max = 20;        
        t.clearScreen();
        int y = 0;
        auto b = format<header_color,TF::BOLD>("Welcome to OS Project");
        t.wScreenCentre(b, y,2);
        
        std::vector<std::string> list = {"Login","Exit"};
        
        auto m = 0u;
        for(auto const& el : list){
            if(m < el.size()){
                m = el.size();
            }
        }

        for(auto & el : list){
            uniform_padding(el, m + 5);
        }
        y += 3;
        for(auto i = 0; i < list.size(); i++){
            if(selected == i){
                b = format(list[i],menu_color{FG::WHITE,BG::GREEN});
            }else{
                b = format<menu_color>(list[i]);
            }
            m = m > b.size() ? m : b.size();
            t.wScreenCentre(b, y++);
        }
        
        t.wScreen("\r\n");

    }

}


#endif // CLIENT_UTILITY_H
