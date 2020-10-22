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
    if ((this->file_i2c = ::open(this->path, mode)) < 0)
    {
        std::cerr << "failed to open i2c channel [" << this->path << "]!" << std::endl;
        throw std::runtime_error("failed to i2c path");
    }
    
    
    std::cout << "successfully opened i2c channel #" << this->file_i2c << " [" << this->path << "," << mode << "]" << std::endl;
    this->bind();
}

void I2CChannel::bind()
{
    if (ioctl(this->file_i2c, I2C_SLAVE, this->address) < 0)
    {
        std::cerr << "failed to bind channel [0x" << std::hex << this->address << std::dec << "]: error=" << strerror(errno) << "!" << std::endl;
        throw std::runtime_error("failed to bind channel address");
    }
    
    std::cout << "successfully bound channel #" << this->file_i2c << " [" << "0x" << std::hex << this->address << std::dec << "]" << std::endl;
}

I2CChannel::~I2CChannel()
{
    if (this->file_i2c > 0)
    {
	std::cout << "close i2c channel #" << this->file_i2c << " [0x" << std::hex << this-> address << std::dec << "]" << std::endl;
        close(this->file_i2c);
        this->file_i2c = -1;
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
    unsigned char buffer[256] = {0};
    std::vector<int> cmdData;
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, cmdData);
        int resultLength = ::read(this->i2cRead.file_i2c, buffer, length);
        if (resultLength <= 0) continue;
       
        data = std::string(reinterpret_cast<char*>(buffer), resultLength);
        std::cout << "read result length=" << resultLength << " => data= " << std::hex << buffer << std::dec << std::endl;
        printf("0x%+01x\n", buffer);
	
	unsigned char tmp[2] = {0};
	for (int j = 0; j < resultLength-1; j += 2)
	{
            tmp[0] = buffer[j];
            tmp[1] = buffer[j+1];
            printf("%+02x\n", tmp);
	}

        if (data[0] == cmd)
        {
            return true;
        }
    }
    
    std::cerr << "failed to read data from channel #" << this->i2cRead.file_i2c << " [0x" << std::hex << this->i2cRead.address << std::dec << "]!" << std::endl;
    return false;
}

bool I2CBus::write(passbutter::Command cmd, std::vector<int> data)
{
    std::string sample = std::to_string(cmd);
    
    for (std::vector<int>::iterator it = std::begin(data); it != std::end(data); ++it) {
        sample += std::to_string(*it);
    }
    
    int resultLength = ::write(this->i2cWrite.file_i2c, sample.c_str(), sample.length());
    std::cout << "write sample '" << sample << "' with len=" << sample.length() << " => result len=" << resultLength << std::endl;

    if (resultLength != sample.length())
    {
	    std::cerr << "failed to write to the i2c bus!" << std::endl;
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
