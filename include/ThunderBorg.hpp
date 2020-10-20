#pragma once

#include <string>
#include <vector>

namespace passbutter
{
    
enum Command
{
    COMMAND_SET_LED1            = 1,     // Set the colour of the ThunderBorg LED
    COMMAND_GET_LED1            = 2,     // Get the colour of the ThunderBorg LED
    COMMAND_SET_LED2            = 3,     // Set the colour of the ThunderBorg Lid LED
    COMMAND_GET_LED2            = 4,     // Get the colour of the ThunderBorg Lid LED
    COMMAND_SET_LEDS            = 5,     // Set the colour of both the LEDs
    COMMAND_SET_LED_BATT_MON    = 6,     // Set the colour of both LEDs to show the current battery level
    COMMAND_GET_LED_BATT_MON    = 7,     // Get the state of showing the current battery level via the LEDs
    COMMAND_SET_A_FWD           = 8,     // Set motor A PWM rate in a forwards direction
    COMMAND_SET_A_REV           = 9,     // Set motor A PWM rate in a reverse direction
    COMMAND_GET_A               = 10,    // Get motor A direction and PWM rate
    COMMAND_SET_B_FWD           = 11,    // Set motor B PWM rate in a forwards direction
    COMMAND_SET_B_REV           = 12,    // Set motor B PWM rate in a reverse direction
    COMMAND_GET_B               = 13,    // Get motor B direction and PWM rate
    COMMAND_ALL_OFF             = 14,   // Switch everything off
    COMMAND_GET_DRIVE_A_FAULT   = 15,    // Get the drive fault flag for motor A, indicates faults such as short-circuits and under voltage
    COMMAND_GET_DRIVE_B_FAULT   = 16,    // Get the drive fault flag for motor B, indicates faults such as short-circuits and under voltage
    COMMAND_SET_ALL_FWD         = 17,    // Set all motors PWM rate in a forwards direction
    COMMAND_SET_ALL_REV         = 18,    // Set all motors PWM rate in a reverse direction
    COMMAND_SET_FAILSAFE        = 19,    // Set the failsafe flag, turns the motors off if communication is interrupted
    COMMAND_GET_FAILSAFE        = 20,    // Get the failsafe flag
    COMMAND_GET_BATT_VOLT       = 21,    // Get the battery voltage reading
    COMMAND_SET_BATT_LIMITS     = 22,    // Set the battery monitoring limits
    COMMAND_GET_BATT_LIMITS     = 23,    // Get the battery monitoring limits
    COMMAND_WRITE_EXTERNAL_LED  = 24,    // Write a 32bit pattern out to SK9822 / APA102C
    COMMAND_GET_ID              = 0x99,  // Get the board identifier
    COMMAND_SET_I2C_ADD         = 0xAA,  // Set a new I2C address

    COMMAND_VALUE_FWD           = 1,     // I2C value representing forward
    COMMAND_VALUE_REV           = 2,     // I2C value representing reverse

    COMMAND_VALUE_ON            = 1,     // I2C value representing on
    COMMAND_VALUE_OFF           = 0,     // I2C value representing off

    COMMAND_ANALOG_MAX          = 0x3FF // Maximum value for analog readings   
};

class ThunderBorg
{
    
public:
    ThunderBorg(const char *name);
    ~ThunderBorg();
    
    std::vector<int> detectBoards(int busNumber = 1);
    bool write(Command cmd, std::vector<int> data);
    bool read(passbutter::Command cmd, int length, std::string &data, int retryCount = 3);
    
private:
    const char *I2CADDR                     = "/dev/i2c-";       // I2C device
    
    const int I2C_SLAVE                     = 0x0703;
    const int PWM_MAX                       = 255;
    const int I2C_MAX_LEN                   = 6;
    const float VOLTAGE_PIN_MAX             = 36.3;              // Maximum voltage from the analog voltage monitoring pin
    const float VOLTAGE_PIN_CORRECTION      = 0.0;               // Correction value for the analog voltage monitoring pin
    const float BATTERY_MIN_DEFAULT         = 7.0;               // Default minimum battery monitoring voltage
    const float BATTERY_MAX_DEFAULT         = 35.0;              // Default maximum battery monitoring voltage

    const int I2C_ID_THUNDERBORG             = 0x15;
    
    int busNumber;
    int i2cAddress;
    int i2cRead;
    int i2cWrite;
    
    std::string name_;
    
    void initBus(int busNumber, int address);
    void bind(int fd, int addr);
};

}