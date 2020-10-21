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
    
I2CChannel::I2CChannel(int busNumber, int mode)
    : address((i2cAddrBase + std::to_string(busNumber)).c_str())
{
    if ((this->channel = ::open(this->address, mode)) < 0)
    {
        std::cout << "failed to open i2c read address " << this->address << std::endl;
        throw std::runtime_error("failed to open channel");
    }
    
    
    std::cout << "successfully opened i2c channel" << std::endl
              << "  => address " << this->address << std::endl
              << "  => mode " << mode << std::endl;
    
    this->bind();
}

void I2CChannel::bind()
{
    if (ioctl(this->channel, I2C_SLAVE, this->address) < 0)
    {
        std::cout << " addr: 0x" << std::hex << this->address << std::dec
                  << " error: " << strerror(errno)
                  << std::endl;
        throw std::runtime_error("failed to bind channel address");
    }
    
    std::cout << "successfully bound channel #" << this->channel
        << "to address " << "0x" << std::hex << this->address << std::dec
        << std::endl;
}

I2CChannel::~I2CChannel()
{
    if (this->channel > 0)
    {
        close(this->channel);
        this->channel = -1;
    }
}
    
I2CBus::I2CBus(int busNumber, int address)
    : busNumber(busNumber),
      address(address),
      i2cRead(I2CChannel(busNumber, O_RDONLY)),
      i2cWrite(I2CChannel(busNumber, O_WRONLY))
{
}

bool I2CBus::read(passbutter::Command cmd, int length, std::string &data, int retryCount)
{
    unsigned char buffer[256];
    std::vector<int> cmdData;
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, cmdData);
        if (::read(this->i2cRead.channel, buffer, length) != length)
        {
            continue;
        }
        
        data = std::string(buffer, buffer + length);
        return true;
    }
    
    std::cout << "Failed to acquire bus access and/or talk to slave.." << std::endl;
    return false;
}

bool I2CBus::write(passbutter::Command cmd, std::vector<int> data)
{
    std::string sample = std::to_string(cmd);
    
    for (std::vector<int>::iterator it = std::begin(data); it != std::end(data); ++it) {
        sample += std::to_string(*it);
    }
    
    unsigned char buffer[256];
    std::copy(sample.begin(), sample.end(), buffer);

    if (::write(this->i2cWrite.channel, buffer, sample.length()) != sample.length())
    {
        printf("Failed to write to the i2c bus.\n");
        return false;
    }
    
    return true;
}

I2CBus::~I2CBus()
{
}

ThunderBorg::ThunderBorg(const char *name)
    : name_(name)
{
}

ThunderBorg::~ThunderBorg()
{
}

std::vector<int> ThunderBorg::detectBoards(int busNumber)
{
    std::cout << "detect boards.." << std::endl;
    std::vector<int> boardAddrs;
    
    for (int address = 0x03; address <= 0x78; address++)
    {
        try
        {
            I2CBus i2cBus = I2CBus(busNumber, address);
            
            std::string data;
            if (i2cBus.read(COMMAND_GET_ID, I2C_MAX_LEN, data) && data.length() == I2C_MAX_LEN) {
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
