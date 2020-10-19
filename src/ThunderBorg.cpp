#include "../include/ThunderBorg.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

namespace passbutter
{

ThunderBorg::ThunderBorg(const char *name)
    : name_(name)
{
    // open a channel to the I2C device
    dev_ = open(I2CADDR, O_RDWR);
 
    if (dev_ < 0)
    {
        std::cerr << "could not load I2C module.."
            << I2CADDR << " "
            << strerror(errno) << std::endl;
    }
}

ThunderBorg::~ThunderBorg()
{
    if (dev_ > 0)
    {
        close(dev_);
        dev_ = -1;
    }
}

bool ThunderBorg::bind(int addr)
{
    if (ioctl(dev_, I2C_SLAVE, addr) < 0)
    {
        std::cerr << name_.c_str()
            << " addr: 0x" << std::hex << addr << std::dec
            << " error: " << strerror(errno) << std::endl;
        return false;
    }
    
    std::cout << "Channel #" << dev_
        << " for " << name_.c_str()
        << "0x" << std::hex << addr << std::dec
        << " is open." << std::endl;
    return true;
}

bool ThunderBorg::read(passbutter::Command cmd, int length, std::string &data, int retryCount = 3)
{
    unsigned char buffer[256];
    std::vector<int> cmdData;
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, cmdData);
        if (::read(dev_, buffer, length) != length)
        {
            continue;
        }
        
        data = std::string(buffer, buffer + length);
        return true;
    }
    
    std::cout << "Failed to acquire bus access and/or talk to slave.." << std::endl;
    return false;
}

void ThunderBorg::write(passbutter::Command cmd, std::vector<int> data)
{
    std::string sample = std::to_string(cmd);
    
    for (std::vector<int>::iterator it = std::begin(data); it != std::end(data); ++it) {
        sample += std::to_string(*it);
    }
    
    unsigned char buffer[256];
    std::copy(sample.begin(), sample.end(), buffer);

    if (::write(dev_, buffer, sample.length()) != sample.length())
    {
        printf("Failed to write to the i2c bus.\n");
    }
}

void ThunderBorg::detectBoards()
{
    std::cout << "detect boards.." << std::endl;
}

}
