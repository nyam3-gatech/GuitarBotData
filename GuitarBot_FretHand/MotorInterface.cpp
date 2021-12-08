#include "MotorInterface.h"

MotorInterface::MotorInterface()
{
	portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
	packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
	dxl_comm_result = COMM_TX_FAIL;
	dxl_error = 0;
	this->setup();
}
MotorInterface::~MotorInterface()
{
	delete portHandler;
	delete packetHandler;
}

int MotorInterface::setup()
{
	if (portHandler->openPort() && portHandler->setBaudRate(BAUDRATE))
	{
		status = 1;
		return 1;
	}
	status = 0;
	return 0;
}

void MotorInterface::enableTorque(int id)
{
	packetHandler->write1ByteTxRx(portHandler, id, ADDR_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
}
void MotorInterface::disableTorque(int id)
{
	packetHandler->write1ByteTxRx(portHandler, id, ADDR_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
}

void MotorInterface::setPositionControl(int id)
{
	packetHandler->write1ByteTxRx(portHandler, id, ADDR_OP_MODE, POSITION_CONTROL, &dxl_error);
	op_mode = POSITION_CONTROL;
}
void MotorInterface::setCurrentControl(int id)
{
	packetHandler->write1ByteTxRx(portHandler, id, ADDR_OP_MODE, CURRENT_CONTROL, &dxl_error);
	op_mode = CURRENT_CONTROL;
}

void MotorInterface::setGoalPosition(int id, int goal)
{
	packetHandler->write4ByteTxRx(portHandler, id, ADDR_GOAL_POSITION, goal, &dxl_error);
}
void MotorInterface::setGoalCurrent(int id, int goal)
{
	packetHandler->write2ByteTxRx(portHandler, id, ADDR_GOAL_CURRENT, goal, &dxl_error);
}

int MotorInterface::getStatus()
{
	return status;
}