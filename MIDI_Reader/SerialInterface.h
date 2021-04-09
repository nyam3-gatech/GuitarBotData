#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "SerialClass.h"
#include "Orders.h"

#include <vector>
#include <thread>

class OrderTicket
{
private:

	uint32_t tick;
	Order order;
	uint32_t data;

public:

	OrderTicket(uint32_t, Order, uint32_t);

	void decrementTick(int = 1);

	uint32_t getTick();
	Order getOrderType();
	uint32_t getData();
	uint8_t getDataByte(char = 0);

};

class SerialInterface
{
private:

	Serial* arduino;

	bool sending;
	bool running;

	std::thread sendingThread;

	std::vector<OrderTicket> buffer;

	int tick_us = 2500;

	void SerialInterfaceLoop();
	static void SerialInterfaceLoopStatic(SerialInterface*);

	void sendOrder(OrderTicket);

public:

	SerialInterface(std::string);
	~SerialInterface();

	void addOrder(OrderTicket);
	
	void startSending();
	void stopSending();

	bool isConnected();
	bool isSending();

};

#endif

