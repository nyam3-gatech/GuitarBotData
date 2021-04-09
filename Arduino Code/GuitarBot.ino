#include <Adafruit_PWMServoDriver.h>
#include "Orders.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// TBD what the constants actually represent
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096) --> 4096 combos in 12 bits
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

#define DEG 60 //angle of travel
double botpulse = map(DEG, 0, 180, SERVOMIN, SERVOMAX); //converts degrees into usable data
double origin = map(DEG / 2, 0, 180, SERVOMIN, SERVOMAX); // maps out 0 angle as usable data
double toppulse = map(0, 0, 180, SERVOMIN, SERVOMAX); // maps out 0 angle as usable data

// Function pointers to turn the servo on/off or to move the cart
void (*servoOn_)(unsigned char pin);
void (*servoOff_)(unsigned char pin);
void (*moveCart_)(unsigned char distance);

void servoOn(unsigned char pin)
{
  pwm.setPWM(pin, 0, toppulse);
}

void servoOff(unsigned char pin)
{
  pwm.setPWM(pin, 0, origin);
}

void setup() {
  Serial.begin(14400); //"This tells the Arduino to get ready to exchange messages with the Serial Monitor at a data rate of 14400 bits per second."
  pwm.begin(); //Setups the I2C interface and hardware.
  pwm.setOscillatorFrequency(27000000); //Setter for the internally tracked oscillator used for freq calculations.
  pwm.setPWMFreq(SERVO_FREQ);

  servoOn_ = &servoOn;
  servoOff_ = &servoOff;
}

void loop() { 

  if(Serial.available() > 0)
  {
    Order order = Order(Serial.read());

    switch(order)
    {
    case RESET:
    case ALLOFF:
      for(int i = 1; i <= 18; i++)
      {
        servoOff_(i);
      }
      break;
    case OFF:
      servoOff_(readByte());
      break;
    case ON:
      servoOn_(readByte());
      break;
    case DAMP:
      // To Do
      break;
    case MOVE:
      moveCart_(readByte());
      break;
    } 
  }
}

unsigned char readByte()
{
  while(Serial.available() < 1); // Wait until a byte of data is available
  return Serial.read(); // Read and return the byte of data
}
