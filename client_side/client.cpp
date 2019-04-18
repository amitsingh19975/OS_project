#include "include/client_utility.h"
#include "include/menus.h"
#include <cmath>
using namespace std;
using namespace terminal;
using namespace chat_utility;

auto padding_vector(std::vector<std::string>& list){
    auto m = 0u;
    for(auto const& el : list){
        if(m < el.size()){
            m = el.size();
        }
    }

    for(auto & el : list){
        uniform_padding(el, m + 5);
    }
}


auto wait(Terminal& t){
    int selected = 0;
    int pressed = 0;
    int max = 0;
    key_event(t,selected, pressed, max);
}

int main(int argv, char** argc){
    Terminal t;
    User user;
    uint32_t user_idx;        
    SocketConnection s(t);
    int selected        =   MAIN;
    bool is_running     =   true;
    int max_scroll      =   0;
    Menu menu           =   MAIN;
            
    std::vector<std::string> list = {"Login","Exit"};
    padding_vector(list);

    // std::map<uint32_t,std::string> user_list = {
    //     {0,"amit"},
    //     {1,"aamir"},
    //     {2,"sv8"}
    // };
    auto user_list = s.get_user_list();
    padding_map(user_list);

    t.init();

    while(is_running){
        t.clearScreen();
        switch(selected){
            case MAIN:{
                max_scroll = list.size();
                auto temp = main_menu(t,list);
                if( temp == 1) is_running = false;
                else{
                    selected = LOGIN;
                }
                break;
            }
            case USER :{
                max_scroll = user_list.size();
                user_idx = user_menu(t, s);
                if(user_idx == -1){
                    selected = MAIN;
                    t.eprint("No User Found!");
                    wait(t);
                }
                selected = CHAT;
                break;
            }
            case LOGIN:{
                login_menu(t,user);
                s.set_user(user);
                if(s.conn() == -1){
                    t.eprint("Error: Unable to Connect To Server!\r\n");
                    s.close_con();
                }
                auto res = s.login();
                if(!res){
                    wait(t);
                    s.close_con();
                }
                user_list = s.get_user_list();
                selected = USER;
                break;
            }
            case CHAT:{
                s.conn_to(user_idx);
                disable();
                chat_menu(t,s);
                if(s.fd() == -1){
                    is_running = false;
                }
                break;
            }
        }
        // is_running = key_event(t,selected, pressed, max_scroll);
        
    }
    s.close_con();
    return 0;
}