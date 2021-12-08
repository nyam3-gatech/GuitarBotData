
#ifndef MOTOR_INTERFACE_H
#define MOTOR_INTERFACE_H

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "dynamixel_sdk.h"  

// Control table address
#define ADDR_TORQUE_ENABLE          64                 // Control table address is different in Dynamixel model
#define ADDR_GOAL_POSITION          116
#define ADDR_PRESENT_POSITION       132
#define ADDR_GOAL_CURRENT			102
#define ADDR_PRESENT_CURRENT		126
#define ADDR_OP_MODE				11

// Protocol version
#define PROTOCOL_VERSION                2.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define BAUDRATE                        57600
#define DEVICENAME                      "COM3"      // Check which port is being used on your controller
															// ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define CURRENT_CONTROL					0
#define POSITION_CONTROL				3

#define ESC_ASCII_VALUE                 0x1b

class MotorInterface
{
private:

	dynamixel::PortHandler* portHandler;
	dynamixel::PacketHandler* packetHandler;
	int dxl_comm_result;
	uint8_t dxl_error;

	int status;
	int op_mode;

public:

	MotorInterface();
	~MotorInterface();

	int setup();

	void enableTorque(int id);
	void disableTorque(int id);

	void setPositionControl(int id);
	void setCurrentControl(int id);

	void setGoalPosition(int id, int goal);
	void setGoalCurrent(int id, int goal);

	int getStatus();

};

#endif MOTOR_INTERFACE_H

