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

int main(int argv, char** argc){
    Terminal t;
    User user;
    SocketConnection    s;
    int selected        =   MAIN;
    bool is_running     =   true;
    int max_scroll      =   0;
    Menu menu           =   MAIN;
            
    std::vector<std::string> list = {"Login","Exit"};
    padding_vector(list);

    std::map<uint32_t,std::string> user_list = {
        {0,"amit"},
        {1,"aamir"},
        {2,"sv8"}
    };

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
                user_menu(t,user_list);
                break;
            }
            case LOGIN:{
                login_menu(t,user);
                s.set_user(user);
                // if(s.conn() == -1){
                //     t.eprint("Error: Unable to Connect To Server!\r\n");
                //     disable();
                //     exit(0);
                // }
                auto mess = s.login();
                // t.sprint("Hello");
                selected = USER;
                break;
            }
        }
        // is_running = key_event(t,selected, pressed, max_scroll);
        
    }
    disable();
    return 0;
}