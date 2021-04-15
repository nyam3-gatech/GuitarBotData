#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "serialib.h"
#include "Orders.h"

#include <string>
#include <vector>
#include <thread>

// OrderTicket encapsulates timing information, order, and order data
class OrderTicket
{
private:

	uint32_t tick; // delay until data is sent to Arduino
	Order order;
	uint32_t data;

public:

	OrderTicket(uint32_t, Order, uint32_t);

	void decrementTick(int = 1);

	uint32_t getTick();
	Order getOrderType();
	// Gets all data as an int
	uint32_t getData();
	// Returns the given index byte of data
	uint8_t getDataByte(char = 0);

};

// Handles transmitting order information to the Arduino
class SerialInterface
{
private:

	serialib arduino;

	bool connected;
	bool sending;
	bool running;

	// Separate thread allows for isolated control of timing
	std::thread sendingThread;

	// Buffer for OrderTicket objects
	std::vector<OrderTicket> buffer;

	// Length of each tick in microseconds
	const int tick_us = 250;

	// Runs on a separate thread to handle timing of sending orders
	void SerialInterfaceLoop();
	// Static variant of SerialInterfaceLoop() to run on a separate thread
	static void SerialInterfaceLoopStatic(SerialInterface*);

	// Send an order through the serial port to the Arduino
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

