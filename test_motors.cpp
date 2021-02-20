#include <iostream>
#include <unistd.h>
#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    passbutter::MotorControl motorControl("test");
    
    std::vector<int> boardAddrs = motorControl.detectBoards(1);
    if (boardAddrs.size() == 0)
    {
        std::cout << "no boards detected.." << std::endl;
    }
    
    motorControl.initBus(boardAddrs[0]);
    
    motorControl.setMotor1(0.3);
    usleep(1000);
    motorControl.setMotor1(0);
    
    return 0;
}
