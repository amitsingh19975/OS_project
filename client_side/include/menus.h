#ifndef MENUS_H
#define MENUS_H

#include "client_utility.h"

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
        }
        return true;
    }

    auto uniform_padding(std::string& el, size_t m){
        auto len = el.size() < m ? m - el.size() : 0;
        len /= 2;
        el = std::string(len,' ') + el + std::string(len,' ');
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


    auto chat_menu_helper(terminal::Terminal& t, SocketConnection& sc , int y){
        
        

    }

    auto chat_menu(terminal::Terminal& t, SocketConnection& sc , int y = 0){
        
        // chat_menu_helper(t,sc,y);
        std::thread s(&SocketConnection::send,&sc);
        std::thread r(&SocketConnection::recv,&sc);

        s.join();
        r.join();
    }

    auto user_menu(terminal::Terminal& t, std::map<uint32_t,std::string> const& list){
        int selected = 0;
        int pressed = -1;

        if(!list.size()){
            return -1;
        }

        while(pressed == -1){
            user_menu_helper(t,list,selected);
            key_event(t,selected,pressed,list.size());
        }
        return pressed;
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
