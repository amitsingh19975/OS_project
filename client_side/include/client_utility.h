#ifndef CLIENT_UTILITY_H
#define CLIENT_UTILITY_H

#include <iostream>
#include <vector>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
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

    enum class COMMANDS : u_char{
        USER,
        NONE
    };
    

    auto string_to_commands(std::string_view str){
        if(str == "user") return COMMANDS::USER;
        else return COMMANDS::NONE;
    }
    
    auto commands_to_string(COMMANDS c){
        switch (c){
        
        case COMMANDS::USER:
            return std::string("user");
        default:
            return std::string("none");
        }
    }
    
    auto is_command(std::string_view str){
        return str[0] == '/';
    }

    auto parse_message(std::string_view str){
        if(!is_command){
            return make_tuple(COMMANDS::NONE,str,std::ptrdiff_t(-1));
        }
        COMMANDS c;
        std::string com;
        size_t i = 1u;
        for(; i < str.size(); i++){
            if(str[i] == ' '){
                break;
            }
            com.push_back(str[i]);
        }

        return make_tuple(string_to_commands(com),str,std::ptrdiff_t(i+1));
    }


    auto uniform_padding(std::string& el, size_t m){
        auto len = el.size() < m ? m - el.size() : 0;
        len /= 2;
        el = std::string(len,' ') + el + std::string(len,' ');
    }

    template<typename T>
    auto padding_map(std::map<T,std::string>& list){
        auto m = 0u;
        for(auto const& [k,el] : list){
            if(m < el.size()){
                m = el.size();
            }
        }

        for(auto & [k,el] : list){
            uniform_padding(el, m + 5);
        }
    }

    struct User{
        
        User():m_is_init(false){}
        User(std::string& user, std::string& pass):m_username(std::move(user)),m_password(std::move(pass)),m_is_init(true){
            assert(m_username.size() <= 100);
            assert(m_password.size() <= 100);
        }
        User(std::string& user):m_username(std::move(user)),m_is_init(true){
            assert(m_username.size() <= 100);
            m_password = std::string(getpass(""));
            assert(m_password.size() <= 100);
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
        auto conn() noexcept;
        auto send() noexcept;
        auto recv() noexcept;
        auto login() noexcept;
        auto conn_to(uint32_t) noexcept;
        auto close_con() noexcept;
        [[nodiscard]] constexpr auto get_user_list() const noexcept
            ->std::map<uint32_t,std::string> const&;

    private:
        void waiting_room(std::string_view, size_t) noexcept;
        void set_users(std::string_view) noexcept;
        void set_users_str(std::string_view) noexcept;

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

    auto SocketConnection::close_con() noexcept{
        terminal::disable();
        close(m_fd);
        exit(1);
    }

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
        m_conn_to = trim(m_conn_to);
        size_t len = sprintf(payload,"%s",m_conn_to.c_str());
        write(m_fd,payload,len);
        // len = read(m_fd,buff,MAX_BYTE);
        
        // sscanf(buff,"%s : %s",type,what);
        
        // if(strcmp(type,"SUCCESS") == 0){
        //     m_ter.sprint(buff);
        // }else{
        //     m_ter.eprint(buff);
        // }
        return 1;
    }

    auto SocketConnection::login() noexcept{
        if(m_fd == -1){
            m_ter.eprint("Not Connected to Server!\r\n");
            return 0;
        }
        char payload[MAX_BYTE]  = {0};
        char buff[MAX_BYTE]     = {0};
        
        size_t len = sprintf(payload,"%s : %s",m_user.m_username.c_str(),m_user.m_password.c_str());
        m_user.m_password.clear();
        
        write(m_fd,payload,len);

        read(m_fd,buff, MAX_BYTE);

        std::stringstream mess(buff);
        std::string what;

        if(is_command(buff)){
            m_ter.sprint("Successfully connected");
            std::getline(mess,what,' ');
            std::getline(mess,what,' ');
        }else{
            m_ter.eprint(buff);
            return 0;
        }
        
        size_t number_of_user{0};
        try{
            number_of_user = stoll(what);
            waiting_room(buff,number_of_user);
        }catch(...){
            m_ter.eprint("Unable to parse to Integer");
            return 0;
        }
        return 1;
    }

    void SocketConnection::set_users_str(std::string_view str) noexcept{
        std::string temp(str);
        std::stringstream ss(temp);
        std::string s;
        int i = 0;
        int j = 0;

        while(std::getline(ss,s,' ')){
            if(i++ < 2) continue;
            m_list[j++] = s;
        }

        padding_map(m_list);
    }

    void SocketConnection::waiting_room(std::string_view str, size_t size) noexcept{
        if(size != 0) set_users_str(str);

        char buff[MAX_BYTE];

        while(size == 0){
            if((read(m_fd,buff, MAX_BYTE) == -1) 
                && !is_command(buff) 
                && std::get<0>(parse_message(buff)) != COMMANDS::USER){
                m_ter.eprint("Internal Server Error");
                close_con();
            }

            set_users_str(buff);
            size = m_list.size();
        }
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
        if(m_fd == -1){
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
        if(m_fd == -1){
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
