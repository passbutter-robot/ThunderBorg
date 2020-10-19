#include <iostream>
#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    passbutter::ThunderBorg thunderborg("test");
    
    thunderborg.detectBoards();
    
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
