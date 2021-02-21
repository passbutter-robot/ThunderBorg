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

bool I2CBus::read(passbutter::Command cmd, unsigned char *data, int length, int retryCount)
{
    unsigned char writeData[1] = {0};
    
    for (int i = 0; i < retryCount; i++)
    {
        write(cmd, writeData, 0);

        int resultLength = ::read(this->i2cRead.file_i2c, data, length);
        if (resultLength <= 0) continue;
        
        //std::cout << "read result len=" << resultLength << " => data=";
        //for (int j=0; j < resultLength; j++)
        //{
        //    printf("0x%x ", data[j]);
        //}
        //std::cout << std::endl;	

        if (data[0] == cmd)
        {
            return true;
        }
    }
    
    std::cerr << "failed to read data from channel #" << this->i2cRead.file_i2c << " [0x" << std::hex << this->i2cRead.address << std::dec << "]!" << std::endl;
    return false;
}

bool I2CBus::write(passbutter::Command cmd, unsigned char* data, int length)
{
    unsigned char writeData[length+1] = {0};
    writeData[0] = cmd;
    for (int i = 0; i < length; i++)
    {
        writeData[i+1] = data[i];
    }

    int resultLength = ::write(this->i2cWrite.file_i2c, writeData, length + 1);
    //std::cout << "write sample len=" << (length+1) << ", data=";
    //for (int i = 0; i <= length; i++)
    //{
    //    printf("0x%x ", writeData[i]);
    //}
    //std::cout << "=> result len=" << resultLength << std::endl;

    if (resultLength != (length+1))
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
    if (bus)
    {
        delete bus;
    }
}

std::vector<int> ThunderBorg::detectBoards(int busNumber, int addressStart, int addressEnd)
{
    std::cout << "detect boards.." << std::endl;
    std::vector<int> boardAddrs;
    
    for (int address = addressStart; address <= addressEnd; address++)
    {
        try
        {
            I2CBus i2cBus = I2CBus(busNumber, address);
            
            unsigned char data[I2C_MAX_LEN] = {0};
            if (i2cBus.read(COMMAND_GET_ID, data, I2C_MAX_LEN)) {
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

bool ThunderBorg::updateBoardAdress(int newBoardAddress, int currentBoardAddress, int busNumber)
{
    std::vector<int> boardAddresses;
    if (currentBoardAddress < 0)
    {
        std::cout << "no current board address given.. try to detect boards.." << std::endl;
        boardAddresses = this->detectBoards(busNumber);
    }
    else
    {
        std::cout << "try to find board with address " << currentBoardAddress << ".." << std::endl;
        boardAddresses = this->detectBoards(busNumber, currentBoardAddress, currentBoardAddress);
    }
    
    if (boardAddresses.size() == 0)
    {
        std::cerr << "no boards detected!" << std::endl;
        return false;
    }
    
    int boardAddress = boardAddresses[0];
    std::cout << "found " << boardAddresses.size() << " boards: first board address is " << boardAddress << "." << std::endl;
    
    I2CBus* bus = nullptr;
    try
    {
        std::cout << "try to setup the i2c bus for bus number " << busNumber << ".." << std::endl;
        bus = new I2CBus(busNumber, boardAddress);
    }
    catch(...)
    {
        std::cerr << "failed to setup the i2c bus for bus number " << busNumber << ", address " << boardAddresses[0] << "!" << std::endl;
        return false;
    }

    std::cout << "try to update the boards address " << boardAddress << " to " << newBoardAddress << std::endl; 
    try
    {
        unsigned char data[1] = { (unsigned char)newBoardAddress };
        bus->write(Command::COMMAND_SET_I2C_ADD, data, 1);
    }
    catch(...)
    {
        std::cerr << "failed to update the board address " << boardAddress << " to " << newBoardAddress << std::endl;
        return false;
    }

    usleep(100);
    
    std::cout << "check if the board address has been successfully updated.." << std::endl;
    boardAddresses = this->detectBoards(busNumber, newBoardAddress, newBoardAddress);

    if (boardAddresses.size() == 0 || boardAddresses[0] != newBoardAddress)
    {
        std::cerr << "failed to update the board address " << boardAddress << " to " << newBoardAddress << "!" << std::endl;
        return false;
    }

    std::cout << "the board address " << boardAddress << " has been successfully updated to " << newBoardAddress << std::endl;
    return true;
}

void ThunderBorg::initBus(int address, int busNumber)
{
    this->bus = new I2CBus(busNumber, address);
}

void ThunderBorg::setMotor(passbutter::Motor motor, double power)
{
    int pwm = 0;
    Command cmd;
    
    if (power < 0)
    {
        switch (motor) {
            case MOTOR_1:
                cmd = COMMAND_SET_A_REV;
                break;
            case MOTOR_2:
                cmd = COMMAND_SET_B_REV;
                break;
            default:
                throw std::runtime_error("motor not handled");
        }
    }
    else
    {
        switch (motor) {
            case MOTOR_1:
                cmd = COMMAND_SET_A_FWD;
                break;
            case MOTOR_2:
                cmd = COMMAND_SET_B_FWD;
                break;
            default:
                throw std::runtime_error("motor not handled");
        }
    }
    
    pwm = int(PWM_MAX * power);
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    if (power < 0) pwm *= -1;
    
    try 
    {
        unsigned char data[1] = { (unsigned char)pwm };
        this->bus->write(cmd, data, 1);
    }
    catch (...)
    {
        std::cerr << "failed to set motor " << motor << " to " << power << std::endl;
    }
}

void ThunderBorg::setMotor1(double power)
{
    setMotor(MOTOR_1, power);
}

void ThunderBorg::setMotor2(double power)
{
    setMotor(MOTOR_2, power);
}

MotorControl::MotorControl(const char *name)
    : ThunderBorg(name)
{
}

MotorControl::~MotorControl()
{
    this->stop();
}

MotorControl::stop()
{
    this->setMotor1(0);
    this->setMotor2(0);
}

StepperControl::StepperControl(const char* name, double maxPower, double holdingPower)
    : ThunderBorg(name)
{
    this->setMaxPower(maxPower);
    this->setHoldingPower(holdingPower);
    this->step = -1;
    this->position = 0;
}

StepperControl::~StepperControl()
{
    this->stop();
}

void StepperControl::stop()
{
    this->setMotor1(0);
    this->setMotor2(0);
}

void StepperControl::initSteps()
{
    if (this->step == -1)
    {
        auto p = this->sequence[this->sequence.size() -1];
        this->setMotor1(p[0]);
        this->setMotor2(p[1]);
        this->step = 0;
    }

    this->position = 0;
}

void StepperControl::setMaxPower(double maxPower)
{
    this->maxPower = maxPower;
    this->sequence = {{
        { +this->maxPower, +this->maxPower },
        { +this->maxPower, -this->maxPower },
        { -this->maxPower, -this->maxPower },
        { -this->maxPower, +this->maxPower }
    }};
}

void StepperControl::setHoldingPower(double holdingPower)
{
    this->holdingPower = holdingPower;
    this->sequenceHold = {{
        { +this->holdingPower, +this->holdingPower },
        { +this->holdingPower, -this->holdingPower },
        { -this->holdingPower, -this->holdingPower },
        { -this->holdingPower, +this->holdingPower }
    }};
}

void StepperControl::move(bool backwards)
{
    if (this->step < 0) this->step = this->sequence.size() - 1;
    else if (this->step >= this->sequence.size()) this->step = 0;
    
    this->setMotor1(this->sequence[this->step][0]);
    this->setMotor2(this->sequence[this->step][1]);
    
    
    int direction = 1;
    if (backwards) direction = -1;
    
    this->step += direction;
    this->position += direction;
}

void StepperControl::holdPosition()
{
    if (this->step < this->sequenceHold.size())
    {
        this->setMotor1(this->sequenceHold[this->step][0]);
        this->setMotor2(this->sequenceHold[this->step][1]);
    }
}

}
