#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <vector>

#include "include/ThunderBorg.hpp"

int main(int argc, char **argv) {
    passbutter::ThunderBorg thunderBorg("thunderborg");
    std::vector<int> boardAddresses;
    
    if (argc > 3) 
    {
        boardAddresses = thunderBorg.detectBoards(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    }
    else if (argc > 2)
    {
        boardAddresses = thunderBorg.detectBoards(atoi(argv[1]), atoi(argv[2]));
    }
    else if (argc > 1)
    {
        boardAddresses = thunderBorg.detectBoards(atoi(argv[1]));
    }
    else
    {
	boardAddresses = thunderBorg.detectBoards();
    }

    std::cout << boardAddresses.size() << " boards detected:" << std::endl;
    for (int i = 0; i < boardAddresses.size(); i++)
    {
        std::cout << "  [" << i << "] => " << boardAddresses[i] << std::endl;
    }
    
    return 0;
}
