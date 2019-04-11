#include "include/client_utility.h"
#include <cmath>
using namespace std;
using namespace fmt;
using namespace fmt::literals;

bool is_square(double num){
    return num == static_cast<int>(num);
}

int main(int argv, char** argc){
    size_t a = 1, b = 10000000;
    size_t count = 0;
    for(size_t i = a; i <= b; i++){
        if(is_square(sqrt(i))) count++;
    }
    cout<<count<<'\n';
    return 0;
}