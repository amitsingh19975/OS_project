#ifndef COMMAND_UTILITIES
#define COMMAND_UTILITIES

#include <string>
#include <unordered_map>
#include <vector>

namespace chat_utility{
    enum class COMMANDS : u_char{
        USER,
        SYNC,
        PERMISSION,
        YES,
        NO,
        EXIT,
        SVR_ERR,
        SVR_WRN,
        NONE
    };

    using commands = std::unordered_map<COMMANDS,std::vector<std::string>>;

    auto to_command(std::string_view str){
        if(str == "sync" || str == "/sync") return COMMANDS::SYNC;
        if(str == "svr_err" || str == "/svr_err") return COMMANDS::SVR_ERR;
        if(str == "svr_wrn" || str == "/svr_wrn") return COMMANDS::SVR_WRN;
        if(str == "perm" ||  str == "/perm") return COMMANDS::PERMISSION;
        if(str == "y" || str == "Y" || str == "/y" || str == "/Y") return COMMANDS::YES;
        if(str == "n" || str == "N" || str == "/n" || str == "/N") return COMMANDS::NO;
        if(str == "user" || str == "/user") return COMMANDS::USER;
        if(str == "exit" || str == "/exit") return COMMANDS::EXIT;
        else return COMMANDS::NONE;
    }
    
    std::string to_string(COMMANDS c){
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
            case COMMANDS::SVR_WRN:
                return std::string("svr_wrn");
            case COMMANDS::SVR_ERR:
                return std::string("svr_err");
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
        return make_tuple(to_command(com),mess);
    }

    int num_params(COMMANDS cmd){
        switch (cmd){
            case COMMANDS::SVR_ERR:
            case COMMANDS::SVR_WRN:
                return 0;
            case COMMANDS::USER:
                return 1;
            case COMMANDS::SYNC:
            case COMMANDS::PERMISSION:
                return -1;
            default:
                return 0;
        }
    }

    commands parse_commands(std::string_view str){
        if(!is_command(str)) return {{COMMANDS::NONE,{}}};
        commands cmd;
        COMMANDS com;
        std::string temp(str);

        auto it = temp.rfind('\n');
        if(it != std::string::npos){
            temp.erase(it);
        }

        std::stringstream ss(temp);
        ss>>std::noskipws;
        while(std::getline(ss,temp,' ')){
            char c;
            std::vector<std::string> params;
            if(is_command(temp)){
                temp = trim(temp);
                com = to_command(temp);
                if(com == COMMANDS::NONE){
                    continue;
                }
                temp.clear();
                int num_parameters = num_params(com);
                ss>>c;
                while(c !='/' && !ss.eof()){
                    if(c == ' ' && (num_parameters != 0 || num_parameters == -1)){
                        num_parameters = num_parameters > 0 ? num_parameters - 1 : num_parameters;
                        temp = trim(temp);
                        if(temp.size() != 0){
                            params.push_back(temp);
                        }                            
                        temp.clear();
                        ss>>c;
                        continue;
                    }
                    temp.push_back(c);
                    ss>>c;
                }
                temp = trim(temp);
                if(temp.size() != 0){
                    params.push_back(temp);
                }

                cmd[com] = params;
                ss.seekg(-1,std::ios_base::cur);
            }
        }
        return cmd;
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
        while(std::getline(ss,com,' ')){
            if(com == "/user") break;
        }
        com = trim(com);
        std::getline(ss,user,' ');
        user = trim(user);
        std::getline(ss,mess);
        mess = trim(mess);
        return make_tuple(to_command(com),user,mess);
    }
}

#endif // COMMAND_UTILITIES
