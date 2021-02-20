#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    if (argc < 2)
    {
        std::cout << argv[0] << " new_board_address [current_board_address]" << std::endl;
        return -1;
    }

    int newBoardAddress = atoi(argv[1]);
    int currentBoardAddress = -1;

    if (argc > 2) 
    {
        currentBoardAddress = atoi(argv[2]);
    }

    passbutter::ThunderBorg thunderborg("thunderborg");
    thunderborg.updateBoardAdress(newBoardAddress, currentBoardAddress);
    
    return 0;
}
