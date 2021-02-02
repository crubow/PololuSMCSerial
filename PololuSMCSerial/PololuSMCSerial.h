/*
file:: PololuSMCSerial.h
author:: Colin Rubow
lastDateModified:: 1/22/21
description:: a compilation of SERIAL code for Arduino through the Pololu Simple Motor
			Controller User Guide.
			Assuming Arduino UNO (pin layout is not final):
			PIN 03 -> Simple Motor Controller RX
			PIN 04 -> Simple Motor Controller TX
			PIN 05 -> Simple Motor Controller RST
			PIN 06 -> Simple Motor Controller ERR
			NOTE:: study the code examples in the user manuel for application
*/




#ifndef POLOLUSMCSERIAL_H
#define POLOLUSMCSERIAL_H

// This library allows software serial communication which is slower than hardware but keeps the UART
// lines open for programming nad debugging.
#include <SoftwareSerial.h>
// This library will enable special int objects to run the serial communication
#include <inttypes.h>
#include <Arduino.h>

// Variable IDs
#define ERROR_STATUS 0
#define LIMIT_STATUS 3
#define TARGET_SPEED 20
#define INPUT_VOLTAGE 23
#define TEMPERATURE 24

// Motor limit IDs
#define FORWARD_ACCELERATION 5
#define REVERSE_ACCELERATION 9
#define DECELERATION 2

class PololuSMCSerial
{
private:
	SoftwareSerial smcSerial;
public:

	// construction of controller object will require the rx and tx pins as uint8_t (or byte) (PIN 03 for RX and PIN 04 for TX is the rec.)
	PololuSMCSerial(uint8_t rxPin, uint8_t txPin) : smcSerial(rxPin, txPin) {}
	~PololuSMCSerial() {}

	// required to allow motors to move
	// must be called when controller restarts and after any error
	void exitSafeStart() { smcSerial.write(0x83); }

	// Speed should be a number from -3200 to 3200 (polarity determines direction)
	void setMotorSpeed(int speed)
	{
		if (speed < 0)
		{
			smcSerial.write(0x86); // motor reverse command
			speed = -speed;
		}
		else { smcSerial.write(0x85); } // motor forward command
		smcSerial.write(speed & 0x1F);
		smcSerial.write(speed >> 5 & 0x7F);
	}

	// set the baud rate (19200 bps is recommended)
	void setBaudRate(int br) { smcSerial.begin(br); }
	
	// If the Simple Motor Controller has automatic baud detection
	// enabled, we first need to send it the byte 0xAA (170 in decimal)
	// so that it can learn the baud rate.
	void detectBaudRate() { smcSerial.write(0xAA); }

	//default set up. Will call serial's begin(), write() and exitSafeStart() in one method
	void start()
	{
		smcSerial.begin(19200);
		delay(5);
		smcSerial.write(0xAA);
		exitSafeStart();
	}

	// read a serial byte (returns -1 if nothing recieved after the timeout expires)
	int readByte()
	{
		char c;
		if (smcSerial.readBytes(&c, 1) == 0) { return -1; }
		return (uint8_t)c;
	}

	// will set the forward motor limit to limitValue
	unsigned char setForwardMotorLimit(unsigned int limitValue)
	{
		smcSerial.write(0xA2);
		smcSerial.write(FORWARD_ACCELERATION);
		smcSerial.write(limitValue & 0x7F);
		smcSerial.write((limitValue >> 7) & 0x1F);
		return readByte();
	}

	// will set the reverse motor limit to limitValue
	unsigned char setReverseMotorLimit(unsigned int limitValue)
	{
		smcSerial.write(0xA2);
		smcSerial.write(REVERSE_ACCELERATION);
		smcSerial.write(limitValue & 0x7F);
		smcSerial.write((limitValue >> 7) & 0x1F);
		return readByte();
	}

	// will set the deceleration limit to limitValue
	unsigned char setDecelerationLimit(unsigned int limitValue)
	{
		smcSerial.write(0xA2);
		smcSerial.write(DECELERATION);
		smcSerial.write(limitValue & 0x7F);
		smcSerial.write((limitValue >> 7) & 0x1F);
		return readByte();
	}

	// returns the specified variable as an unsigned integer
	// if the requested variable is signed, the value returned by this function
	// should be typecast as an int
	unsigned int getVariable(unsigned char variableID)
	{
		smcSerial.write(0xA1);
		smcSerial.write(variableID);
		return readByte() + 256 * readByte();
	}

};

#endif