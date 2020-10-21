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
    
I2CChannel::I2CChannel(int busNumber, int address, int mode)
    : path((i2cBasePath + std::to_string(busNumber)).c_str()),
      address(address)
{
    if ((this->channel = ::open(this->path, mode)) < 0)
    {
        std::cout << "failed to open i2c path " << this->path << std::endl;
        throw std::runtime_error("failed to i2c path");
    }
    
    
    std::cout << "successfully opened i2c channel" << std::endl
              << "  => address " << this->path << std::endl
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
      i2cRead(I2CChannel(busNumber, address, O_RDONLY)),
      i2cWrite(I2CChannel(busNumber, address, O_WRONLY))
{
}

bool I2CBus::read(passbutter::Command cmd, int length, std::string &data, int retryCount)
{
    unsigned char buffer[256];
    std::vector<int> cmdData;
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, cmdData);
        int resultLength = ::read(this->i2cRead.channel, &buffer, length);
	if (resultLength <= 0) continue;

        for (int j = 0; j < resultLength; j++) {
            printf("%c\n", buffer[j]);
        }

	data = std::string(reinterpret_cast<char*>(buffer), resultLength);
	std::cout << "read result length=" << resultLength << "buffer[0]=" << buffer[0] << ", data= " << data << std::endl;

	if (data[0] == cmd)
        {
	    return true;
        }
    }
    
    std::cout << "failed to read data from channel" << std::endl;
    return false;
}

bool I2CBus::write(passbutter::Command cmd, std::vector<int> data)
{
    std::string sample = std::to_string(cmd);
    
    for (std::vector<int>::iterator it = std::begin(data); it != std::end(data); ++it) {
        sample += std::to_string(*it);
    }
    
    std::cout << "write sample= " << sample << ", len=" << sample.length() << std::endl;

    int resultLength = ::write(this->i2cWrite.channel, sample.c_str(), sample.length());
    std::cout << "write sample= " << sample << ", target len=" << sample.length() << ", current len=" << resultLength << std::endl;

    if (resultLength != sample.length())
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
    
    for (int address = 0x03; address <= 0x20; address++)
    {
        try
        {
            I2CBus i2cBus = I2CBus(busNumber, address);
            
            std::string data;
            if (i2cBus.read(COMMAND_GET_ID, I2C_MAX_LEN, data)) {
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
