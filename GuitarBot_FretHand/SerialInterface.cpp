#include "SerialInterface.h"
#include <chrono>

#define ARD_BAUD 14400

SerialInterface::SerialInterface(std::string port)
{
	connected = false;
	if (arduino.openDevice(port.c_str(), ARD_BAUD) == 1) connected = true;

	sending = false;
	running = true;

	sendingThread = std::thread(SerialInterfaceLoopStatic, this);
	sendingThread.detach();
}

SerialInterface::~SerialInterface()
{
	connected = false;
	sending = false;
	running = false;
	arduino.closeDevice();
}

void SerialInterface::SerialInterfaceLoop()
{
	auto start = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::high_resolution_clock::now();
	long long time_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed - start).count();

	while (running)
	{
		if (sending && isConnected())
		{
			for (int i = 0; i < buffer.size();)
			{
				OrderTicket ticket = buffer[i];
				if (ticket.getTick() <= 0)
				{
					sendOrder(ticket);
					buffer.erase(buffer.begin() + i);
				}
				else
				{
					i++;
				}
			}

			elapsed = std::chrono::high_resolution_clock::now();
			time_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed - start).count();

			if (time_us > tick_us)
			{
				start = std::chrono::high_resolution_clock::now();
				
				int ticks = time_us / tick_us;

				for (OrderTicket ticket : buffer)
				{
					ticket.decrementTick(ticks);
				}
			}
		}
	}
}

void SerialInterface::SerialInterfaceLoopStatic(SerialInterface* ptr)
{
	ptr->SerialInterfaceLoop();
}

void SerialInterface::sendOrder(OrderTicket ticket)
{
	Order order = ticket.getOrderType();

	char charBuffer[5];
	charBuffer[0] = (int) order;

	int numDataBytes = getNumOrderDataBytes(order);

	for (int i = 0; i < numDataBytes; i++)
	{
		charBuffer[i + 1] = ticket.getDataByte(i);
	}

	arduino.writeBytes(charBuffer, 1 + numDataBytes);
}

void SerialInterface::addOrder(OrderTicket order)
{
	buffer.push_back(order);
}

void SerialInterface::startSending()
{
	sending = true;
}
void SerialInterface::stopSending()
{
	sending = false;
}

bool SerialInterface::isConnected()
{
	return connected;
}
bool SerialInterface::isSending()
{
	return sending;
}

OrderTicket::OrderTicket(uint32_t tick_, Order order_, uint32_t data_)
{
	tick = tick_;
	order = order_;
	data = data_;
}

void OrderTicket::decrementTick(int amount)
{
	tick -= amount;
}

uint32_t OrderTicket::getTick()
{
	return tick;
}
Order OrderTicket::getOrderType()
{
	return order;
}
uint32_t OrderTicket::getData()
{
	return data;
}
uint8_t OrderTicket::getDataByte(char byte)
{
	return (data >> (8 * byte)) & 0x000000FF;
}