#include <iostream>
#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    passbutter::ThunderBorg thunderborg("test");
    
    std::vector<int> boardAddrs = thunderborg.detectBoards(1);
    
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
