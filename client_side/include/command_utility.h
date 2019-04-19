#ifndef COMMAND_UTILITIES
#define COMMAND_UTILITIES

#include <string>

namespace chat_utility{
    
    enum class COMMANDS : u_char{
        USER,
        SYNC,
        PERMISSION,
        YES,
        NO,
        EXIT,
        NONE
    };
    

    auto string_to_commands(std::string_view str){
        if(str == "sync") return COMMANDS::SYNC;
        if(str == "perm") return COMMANDS::PERMISSION;
        if(str == "y" || str == "Y") return COMMANDS::YES;
        if(str == "n" || str == "N") return COMMANDS::NO;
        if(str == "user") return COMMANDS::USER;
        if(str == "exit") return COMMANDS::EXIT;
        else return COMMANDS::NONE;
    }
    
    auto commands_to_string(COMMANDS c){
        switch (c){
            case COMMANDS::SYNC:
                return std::string("sync");
            case COMMANDS::USER:
                return std::string("user");
            case COMMANDS::PERMISSION:
                return std::string("perm");
            case COMMANDS::YES:
                return std::string("y");
            case COMMANDS::NO:
                return std::string("n");
            case COMMANDS::EXIT:
                return std::string("exit");
            default:
                return std::string("none");
        }
    }
    
    auto is_command(std::string_view str){
        return str[0] == '/';
    }

    auto parse_message(std::string_view str){

        if(!is_command(str)){
            return make_tuple(COMMANDS::NONE,std::string());
        }
        COMMANDS c;
        std::string com;
        std::string mess;
        std::string temp(str);
        temp.erase(0,1);
        auto it = temp.find('\n');
        if(it != std::string::npos){
            temp.erase(it);
        }
        std::stringstream ss(temp);
        std::getline(ss,com,' ');
        std::getline(ss,mess);
        return make_tuple(string_to_commands(com),mess);
    }

    auto parse_user(std::string_view str){
        if(!is_command(str)){
            return make_tuple(COMMANDS::NONE,std::string(),std::string(str));
        }
        COMMANDS c;
        std::string com;
        std::string user;
        std::string temp(str);
        std::string mess;
        temp = trim(temp);
        std::stringstream ss(temp);
        std::getline(ss,com,' ');
        com = trim(com);
        std::getline(ss,user,' ');
        user = trim(user);
        std::getline(ss,mess,'\n');
        mess = trim(mess);
        return make_tuple(string_to_commands(com),user,mess);
    }
}

#endif // COMMAND_UTILITIES
