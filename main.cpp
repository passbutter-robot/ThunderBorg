#include <iostream>
#include <unistd.h>
#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    passbutter::ThunderBorg thunderborg("test");
    
    std::vector<int> boardAddrs = thunderborg.detectBoards(1);
    if (boardAddrs.size() == 0)
    {
        std::cout << "no boards detected.." << std::endl;
    }
    
    thunderborg.initBus(boardAddrs[0]);
    
    thunderborg.setMotor1(0.3);
    usleep(1000);
    thunderborg.setMotor1(0);
    
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
