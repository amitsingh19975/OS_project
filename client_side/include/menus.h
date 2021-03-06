#ifndef MENUS_H
#define MENUS_H

#include "client_utility.h"
#include <future>

using namespace terminal;

namespace chat_utility{

    using header_color = Bit_3_4<FG::BLUE>;
    using menu_color = Bit_3_4<FG::GREEN>;

    enum Menu{
        MAIN = -1,
        LOGIN,
        USER,
        CHAT
    };

    struct key_pram{
        Terminal const& t;
        int& selected;
        int& pressed;
        bool& key_event_running;
        size_t& max;
        int& key;
        key_pram(Terminal const& t, int& selected, int& pressed, int& key, bool &key_event_running, size_t& max):
            t(t),selected(selected),pressed(pressed),key_event_running(key_event_running),max(max),key(key){}
    };

    bool key_event(Terminal const& t, int& selected, int& pressed, size_t max) noexcept{
        switch(t.keyEvent()){
            case 'q':
                write(1, "\x1b[2J", 4);
                write(1, "\x1b[H", 3);
                disable();
                exit(0);
                return false;
            case ARROW_DOWN:
                if(selected < max - 1) selected++;
                return true;
            case ARROW_UP:
                if(selected > 0) selected--;
                return true;
            case ENTER_KEY:{
                pressed = selected;
                selected = 0;
                return true;
            }
            case 'r':{
                pressed = -2;
                return true;
            }
            default:
                break;  
        }
        return true;
    }

    bool key_event_async(key_pram& kp) noexcept{
        while(kp.key_event_running){
            kp.key = kp.t.keyEvent();
            switch(kp.key){
                case 'q':
                    write(1, "\x1b[2J", 4);
                    write(1, "\x1b[H", 3);
                    disable();
                    exit(0);
                case ARROW_DOWN:
                    if(kp.selected < kp.max - 1) kp.selected++;
                    break;
                case ARROW_UP:
                    if(kp.selected > 0) kp.selected--;
                    break;
                case ENTER_KEY:{
                    kp.pressed = kp.selected;
                    kp.selected = 0;
                    return false;
                }
                case 'y':
                case 'Y':
                    return true;
                default:
                    break;  
            }
        }
        return true;
    }

    auto main_menu_helper(terminal::Terminal& t, std::vector<std::string>& list,int selected){
        
        t.clearScreen();
        int y = 0;
        auto b = format<header_color,TF::BOLD>("Welcome to OS Project");
        t.wScreenCentreX(b, y,2);

        y += 3;
        for(auto i = 0; i < list.size(); i++){
            if(selected == i){
                b = format(list[i],menu_color{FG::WHITE,BG::GREEN});
            }else{
                b = format<menu_color>(list[i]);
            }
            t.wScreenCentreX(b, y++);
        }
        
        t.wScreen("\r\n");

    }

    auto login_menu(terminal::Terminal& t, User& user) {
        std::string user_string = "Username: ";
        std::string username, pass;
        
        auto u = format<menu_color,TF::BOLD,TF::ITALIC>(user_string);
        int x = user_string.size();
        int y = 0;
        t.wScreen(u,0);
        t.setCursor(x + 1,0);

        terminal::disable();
        std::cin>>username;
        user = User(username);
        t.init();
    }


    auto user_menu_helper(terminal::Terminal& t, std::map<uint32_t,std::string> const& list
        ,int selected){
        
        t.clearScreen();
        int y = 0;
        auto b = format<header_color,TF::BOLD>("Online Users");
        t.wScreenCentreX(b, y,2);

        y += 3;

        for(auto const& [key,val] : list){
            
            if(selected == key){
                b = format(val,menu_color{FG::WHITE,BG::GREEN});
            }else{
                b = format<menu_color>(val);
            }
            t.wScreenCentreX(b, y++);
        }
        
        t.wScreen("\r\n");

    }
    
    auto chat_menu(terminal::Terminal& t, SocketConnection& sc){
        auto str = format<Bit_3_4<FG::MAGENTA>,TF::BOLD,TF::UNDERLINE>("User: " +sc.get_username() + "\r\n\r\n");
        t.wScreenCentreX(str);
        std::future<int> s = std::async(std::launch::async,&SocketConnection::send,&sc);
        std::future<int> r = std::async(std::launch::async,&SocketConnection::recv,&sc);

        auto ret_r = r.get();
        auto ret_s = s.get();
        if(ret_r <= 2 || ret_s){
            sc.close_con();
            exit(0);
        }
    }

    std::pair<COMMANDS,int> user_menu(terminal::Terminal& t, SocketConnection& sc){
        int selected = 0;
        int pressed = -1;
        bool refresh = false;
        auto prev = -1;
        size_t size{0};
        size_t prev_size{0};
        bool key_event_running{true};
        
        if(!sc.get_user_list().size()){
            return {COMMANDS::NONE,1};
        }

        auto comp_map = [](auto const& m1, auto const& m2) -> bool{
            return std::equal(m1.begin(),m1.end(),m2.begin(),m2.end(),[](auto const& el1, auto const& el2){
                return el1 == el2;
            });
        };
        int key_pressed{-1};
        key_pram kp(t,selected,pressed, key_pressed,key_event_running,size);
        std::thread key_t(key_event_async, std::ref(kp));

        auto temp_map = sc.get_user_list(); 

        while(pressed == -1){
            char buff[2048] = {0};
            if(selected != prev  || refresh){
                user_menu_helper(t,sc.get_user_list(),selected);
                prev = selected;
                temp_map = sc.get_user_list();
                size = temp_map.size();
                refresh = false;
            }
            auto len = recv(sc.fd(),buff,2048,MSG_DONTWAIT);
            if(len > 0){
                refresh = true;
                auto cmd = parse_commands(buff);
                
                if(cmd.find(COMMANDS::EXIT) != cmd.end()){
                    t.eprint("EXIT");
                    sc.close_con();
                    return {COMMANDS::NONE,-1};
                }else if(auto it = cmd.find(COMMANDS::SVR_ERR); it != cmd.end()){
                    t.eprint(it->second.at(0));
                    sc.close_con();
                    return {COMMANDS::NONE,-1};
                }else if(auto it = cmd.find(COMMANDS::SVR_WRN); it != cmd.end()){
                    t.wprint(it->second.at(0));
                    sc.close_con();
                    return {COMMANDS::NONE,-1};
                }
                
                if(auto it = cmd.find(COMMANDS::PERMISSION); it != cmd.end()){
                    auto user = cmd.find(COMMANDS::USER);
                    if(user == cmd.end()){
                        std::string str = "/exit";
                        write(sc.fd(),str.c_str(),str.size());
                        t.eprint("Unable to parse user\n");
                        sc.close_con();
                    }
                    std::string str = user->second[0] + " asking for permission. Press Y or N\r\n";
                    t.wprint(str);
                    
                    auto key_press = key_pressed;
                    while(key_press != 'y' && key_press != 'n' && 
                        key_press != 'Y' && key_press != 'N'){
                            key_press = key_pressed;
                    };
                    
                    memset(buff,0,MAX_BYTE);
                    len = sprintf(buff,"/perm %c",static_cast<char>(key_press));
                    write(sc.fd(),buff,len);

                    if(key_press == 'y' || key_press == 'Y') {
                        key_event_running = false;
                        key_t.join();
                        return {COMMANDS::PERMISSION,0};
                    }
                    continue;
                }

                if(auto it = cmd.find(COMMANDS::SYNC); it != cmd.end()){
                    sc.set_map(it->second);
                }
            }
        }
        key_event_running = false;
        key_t.join();
        return {COMMANDS::NONE,pressed};
    }

    auto main_menu(terminal::Terminal& t, std::vector<std::string>& list){
        int selected = 0;
        int pressed = -1;
        while(pressed == -1){
            main_menu_helper(t,list,selected);
            key_event(t,selected,pressed,list.size());
        }
        return pressed;
    }
}


#endif // MENUS_H
