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
#include <map>
#include "terminal_raw.h"
#include "color.h"
#include "string_utility.h"
#include "command_utility.h"

#define MAX_BYTE 2048

#define DEBUG_E(x) terminal::disable();\
    std::cout<<x<<'\n';\
    exit(0);
#define DEBUG(x) terminal::disable();\
    std::cout<<x<<'\n';

namespace chat_utility{

    struct SocketConnection;


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
        auto recv() noexcept -> int;
        auto login() noexcept;
        auto close_con() noexcept;
        auto fd() const noexcept;
        auto get_username() const noexcept;
        auto conn_to(COMMANDS cmd, uint32_t) noexcept;
        void set_map(std::vector<std::string> const&) noexcept;
        int waiting_room(std::string_view, size_t) noexcept;
        [[nodiscard]] constexpr auto get_user_list() const noexcept
            ->std::map<uint32_t,std::string> const&;
        ~SocketConnection() noexcept;
        

    private:
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

    auto SocketConnection::get_username() const noexcept{
        return m_user.get_user();
    }

    auto SocketConnection::fd() const noexcept{
        return m_fd;
    }

    auto SocketConnection::close_con() noexcept{
        terminal::disable();
        close(m_fd);
        exit(1); 
    }

    SocketConnection::~SocketConnection() noexcept{
        terminal::disable();
        close(m_fd);
        exit(0);
    }

    auto SocketConnection::conn() noexcept{
        sockaddr_in client;

        m_fd = socket(AF_INET,SOCK_STREAM,0);

        client.sin_family = AF_INET;
        client.sin_port = htons(8096);

        inet_pton(AF_INET, "127.0.0.1", &client.sin_addr);
        return connect(m_fd,reinterpret_cast<sockaddr*>(&client), sizeof(client));
    }

    auto SocketConnection::conn_to(COMMANDS cmd, uint32_t idx) noexcept{
        if(m_fd == -1){
            m_ter.eprint("Not Connected to Server!\r\n");
            return 1;
        }
        if(cmd != COMMANDS::PERMISSION){
            char payload[MAX_BYTE]  = {0};
            char buff[MAX_BYTE]     = {0};
            char type[10] = {0};
            char what[MAX_BYTE - 10] = {0};
            m_conn_to = m_list.at(idx);
            m_conn_to = trim(m_conn_to);
            size_t len = sprintf(payload,"%s",m_conn_to.c_str());
            
            write(m_fd,payload,len);
            
            m_ter.wprint("Waiting for permission! Please wait...");
            DEBUG("")
            while(true){
                if(read(m_fd,buff,MAX_BYTE) == -1){
                    continue;
                }
                auto cmd = parse_commands(buff);
                
                if(cmd.find(COMMANDS::EXIT) != cmd.end()){
                    m_ter.eprint("EXIT");
                    return 1;
                }else if(auto it = cmd.find(COMMANDS::SVR_ERR); it != cmd.end()){
                    m_ter.eprint(it->second.at(0));
                    return 1;
                }else if(auto it = cmd.find(COMMANDS::SVR_WRN); it != cmd.end()){
                    m_ter.wprint(it->second.at(0));
                    return 0;
                }

                if(cmd.find(COMMANDS::SYNC) != cmd.end() ){
                    set_users_str(buff);
                    continue;
                }
                if(auto it = cmd.find(COMMANDS::PERMISSION); it != cmd.end()){
                    auto res = it->second.at(0);
                    if(res == "DENIED"){
                        m_ter.eprint("Permission Denied");
                        return 1;
                    }else if(res == "ACCEPTED"){
                        m_ter.sprint("Permission Accepted");
                        return 0;
                    }else{
                        m_ter.eprint(res);
                        return 1;
                    }
                }

            }
            

        }
        return 0;
    }

    auto SocketConnection::login() noexcept{
        if(m_fd == -1){
            m_ter.eprint("Not Connected to Server!\r\n");
            return 1;
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
            return 1;
        }
        
        size_t number_of_user{0};
        try{
            number_of_user = stoll(what);
            auto ret = waiting_room(buff,number_of_user);
            if(ret){
                m_ter.eprint("Server has Closed the Connection");
                return 1;
            }
        }catch(...){
            m_ter.eprint("Unable to parse to Integer");
            return 1;
        }
        return 0;
    }

    void SocketConnection::set_users_str(std::string_view str) noexcept{
        auto cmd = parse_commands(str);
        if(auto it = cmd.find(COMMANDS::SYNC); it == cmd.end()){
            std::string str = "/exit";
            write(m_fd,str.c_str(),str.size());
            m_ter.eprint("Unable to parse sync\n");
            close_con();
        }else{
            set_map(it->second);
        }
    }

    int SocketConnection::waiting_room(std::string_view str, size_t size) noexcept{
        if(size != 0) set_users_str(str);

        char buff[MAX_BYTE];

        while(size == 0){
            auto cmd = parse_commands(buff);
            if((read(m_fd,buff, MAX_BYTE) == -1) 
                && !is_command(buff)){
                m_ter.eprint("Internal Server Error");
                close_con();
            }

            if(cmd.find(COMMANDS::EXIT) != cmd.end()){
                m_ter.eprint("EXIT");
                return 1;
            }

            if(auto it = cmd.find(COMMANDS::SVR_ERR); it != cmd.end()){
                m_ter.eprint(it->second.at(0));
                return 1;
            }else if(auto it = cmd.find(COMMANDS::SVR_WRN); it != cmd.end()){
                m_ter.wprint(it->second.at(0));
                return 0;
            }

            set_users_str(buff);
            size = m_list.size();
        }
        return 0;
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
            auto str = format<Bit_3_4<FG::RED>,TF::BOLD>("Connect to the SERVER!\r\n");
            write(1,str.c_str(),str.size());
            return 1;
        }
        
        m_connected = true;
       
        while(m_connected){
            char payload[MAX_BYTE] = {0};
            std::string str;
            if(read(0,payload,MAX_BYTE) == -1){
                m_connected = false;
                str = format<Bit_3_4<FG::RED>,TF::BOLD>("Server Got Disconnectd!\r\n");
                write(1,str.c_str(),str.size());
                return 1;
            }

            if(strlen(payload) == 0) continue;

            auto cmd = parse_commands(payload);
            
            if(cmd.find(COMMANDS::EXIT) != cmd.end()){
                write(m_fd, str.c_str(), MAX_BYTE);
                // str = format<Bit_3_4<FG::RED>,TF::BOLD>("Successfully left the Chat Room!\r\n");
                // write(1,str.c_str(),str.size());
                str = "/exit";
                write(m_fd,str.c_str(),str.size());
                m_ter.eprint("Successfully left the Chat Room!");
                m_connected = false;
                return 2;
            }else if(cmd.find(COMMANDS::SYNC) != cmd.end()){
                waiting_room(payload,10);
                continue;
            }else if (cmd.find(COMMANDS::NONE) != cmd.end()){
                str = "/user " + m_user.get_user() + " " + std::string(payload);
            }

            write(m_fd, str.c_str(), MAX_BYTE);
        }
        return 0;
    }

    auto SocketConnection::recv() noexcept -> int{
        if(m_fd == -1){
            auto str = format<Bit_3_4<FG::RED>,TF::BOLD>("Connect to the SERVER!\r\n");
            write(1,str.c_str(),str.size());
            return 1;
        }

        m_connected = true;
        
        while(m_connected){
            std::string str;
            char payload[MAX_BYTE] = {0};
            
            if(::recv(m_fd,payload,MAX_BYTE,MSG_DONTWAIT) == -1){
                continue;
            }
            
            if(strlen(payload) == 0) continue;

            auto cmd = parse_commands(payload);
            if(cmd.find(COMMANDS::EXIT) != cmd.end()){
                m_connected = false;
                m_ter.eprint("EXIT");
                return 2;
            }else if(auto it = cmd.find(COMMANDS::SVR_ERR); it != cmd.end()){
                m_ter.eprint(it->second.at(0));
                return 3;
            }else if(auto it = cmd.find(COMMANDS::SVR_WRN); it != cmd.end()){
                m_ter.wprint(it->second.at(0));
                return 4;
            }else if(auto it = cmd.find(COMMANDS::USER); it != cmd.end()){
                if(it->second.size() == 0) continue;
                str = format<Bit_3_4<FG::MAGENTA>,TF::BOLD>(it->second.at(0)) + " : " + it->second.at(1) + "\n";
            }else if(cmd.find(COMMANDS::SYNC) != cmd.end()){
                waiting_room(payload,10);
                continue;
            }
            
            if(str.size() != 0){
                write(1,str.c_str(),str.size());
            }
        }
        return 0;
    }

    void SocketConnection::set_map(std::vector<std::string> const& users) noexcept{
        m_list.clear();
        for(auto i = 0; i < users.size() - 1; i++){
                m_list[i] = users[i + 1];
        }
        padding_map(m_list);
    }

}

#endif // CLIENT_UTILITY_H
