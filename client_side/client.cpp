#include "include/client_utility.h"
#include <cmath>
using namespace std;
using namespace terminal;
using namespace chat_utility;

int main(int argv, char** argc){
    Terminal t;
    User u("Amit","Singh");
    SocketConnection s;

    // while(true){

    // }
    t.init();
    main_menu(t);
    disable();
    return 0;
}