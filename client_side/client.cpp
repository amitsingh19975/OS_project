#include "include/client_utility.h"
#include <cmath>
using namespace std;
using namespace terminal;

int main(int argv, char** argc){
    Terminal t;
    t.init();

    t.wScreen("Hello",23,4);

    disable();
    return 0;
}