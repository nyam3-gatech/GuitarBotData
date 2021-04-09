#ifndef ORDERS_H
#define ORDERS_H

// Define the orders that can be sent and received
enum Order {
	RESET = 0, // Reset
	ALLOFF = 1, // Turn all servos off
	OFF = 2, // Turn a servo off
	ON = 3, // Turn a servo on
	DAMP = 4, // Turn a servo to the damped position
	MOVE = 5 // Move the carriage
};

typedef enum Order Order;

// Returns the number of data bytes associated with the order excluding the order code
int getNumDataBytes(Order order)
{
	switch (order)
	{
	case RESET:
	case ALLOFF:
		return 0;
	case OFF:
	case ON:
	case DAMP:
	case MOVE:
		return 1;
	}
}

#endif