#include <iostream>
#include "dynx.hpp"

using namespace dynx;

int main() {
    Dynx<int> x = 5;
    Dynx<int> y([&](){
        return x * x;
    });
    while(x < 10){
        std::cout << x << " " << y << std::endl;
        x = x + 1;
    }
    //produces:
    //    5 25
    //    6 36
    //    7 49
    //    8 64
    //    9 81
}