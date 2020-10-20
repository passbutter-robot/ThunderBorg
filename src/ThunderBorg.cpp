#include "../include/ThunderBorg.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <sstream>

namespace passbutter
{

ThunderBorg::ThunderBorg(const char *name)
    : name_(name)
{
}

ThunderBorg::~ThunderBorg()
{
    if (this->i2cRead > 0)
    {
        close(this->i2cRead);
        this->i2cRead = -1;
    }
    
    if (this->i2cWrite > 0)
    {
        close(this->i2cWrite);
        this->i2cWrite = -1;
    }
}

void ThunderBorg::initBus(int busNumber, int address)
{
    this->busNumber = busNumber;
    this->i2cAddress = address;
    const char* i2cAddr = I2CADDR + this->busNumber;
    
    std::stringstream errMsg;
    if ((this->i2cRead = ::open(i2cAddr, O_RDONLY)) < 0)
    {
        errMsg << "failed to open i2c read address " << i2cAddr;
        throw std::runtime_error(errMsg.str().c_str());
    }
    
    if ((this->i2cWrite = ::open(i2cAddr, O_WRONLY)) < 0)
    {
        errMsg << "failed to open i2c write address " << i2cAddr;
        throw std::runtime_error(errMsg.str().c_str());
    }
    
    bind(this->i2cRead, this->i2cAddress);
    bind(this->i2cWrite, this->i2cAddress);
}

void ThunderBorg::bind(int fd, int addr)
{
    std::stringstream errMsg;
    
    if (ioctl(fd, I2C_SLAVE, addr) < 0)
    {
        errMsg << name_.c_str()
            << " addr: 0x" << std::hex << addr << std::dec
            << " error: " << strerror(errno);
        throw std::runtime_error(errMsg.str().c_str());
    }
    
    std::cout << "Channel #" << fd
        << " for " << name_.c_str()
        << "0x" << std::hex << addr << std::dec
        << " is open." << std::endl;
}

bool ThunderBorg::read(passbutter::Command cmd, int length, std::string &data, int retryCount)
{
    unsigned char buffer[256];
    std::vector<int> cmdData;
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, cmdData);
        if (::read(this->i2cRead, buffer, length) != length)
        {
            continue;
        }
        
        data = std::string(buffer, buffer + length);
        return true;
    }
    
    std::cout << "Failed to acquire bus access and/or talk to slave.." << std::endl;
    return false;
}

bool ThunderBorg::write(passbutter::Command cmd, std::vector<int> data)
{
    std::string sample = std::to_string(cmd);
    
    for (std::vector<int>::iterator it = std::begin(data); it != std::end(data); ++it) {
        sample += std::to_string(*it);
    }
    
    unsigned char buffer[256];
    std::copy(sample.begin(), sample.end(), buffer);

    if (::write(this->i2cWrite, buffer, sample.length()) != sample.length())
    {
        printf("Failed to write to the i2c bus.\n");
        return false;
    }
    
    return true;
}

std::vector<int> ThunderBorg::detectBoards(int busNumber)
{
    std::cout << "detect boards.." << std::endl;
    std::vector<int> boardAddrs;
    
    for (int address = 0x03; address <= 0x78; address++)
    {
        try
        {
            initBus(busNumber, address);
            std::string data;
            read(COMMAND_GET_ID, I2C_MAX_LEN, data);
            if (data.length() == I2C_MAX_LEN) {
                if (data[1] == I2C_ID_THUNDERBORG)
                {
                    std::cout << "found thunderborg at " << address << std::endl;
                    boardAddrs.push_back(address);
                }
            }
        }
        catch (...)
        {
            
        }
    }
    
    if (boardAddrs.size() == 0)
    {
        std::cout << "no boards detected.." << std::endl;
    }
    else
    {
        std::cout << "found " << boardAddrs.size() << " boards.." << std::endl;
    }
    
    return boardAddrs;
}

}
