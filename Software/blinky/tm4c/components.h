/*
The MIT License (MIT)

Copyright (c) 2016 Cynara Krewe

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software, hardware and associated documentation files (the "Solution"), to deal
in the Solution without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Solution, and to permit persons to whom the Solution is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Solution.

THE SOLUTION IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOLUTION OR THE USE OR OTHER DEALINGS IN THE
SOLUTION.
 */

#ifndef TM4C_COMPONENTS_H_
#define TM4C_COMPONENTS_H_

#include "inc/hw_memmap.h"

#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "flow/flow.h"

class Uart0Receiver
:	public Flow::Component
{
public:
	Flow::OutPort<char> out;
	void run()
	{
		// Loop while there are characters in the receive FIFO.
		while(UARTCharsAvail(UART0_BASE))
		{
			// Read the next character from the UART and write it back to the UART.
			char received = UARTCharGetNonBlocking(UART0_BASE);
			if(received >= 0)
			{
				out.send(received);
			}
		}
	}
	Uart0Receiver()
	{
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
		GPIOPinConfigure(GPIO_PA0_U0RX);
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0);
		UARTConfigSetExpClk(UART0_BASE, 120 MHz, 115200,
		                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
		                             UART_CONFIG_PAR_NONE));
		UARTFIFOEnable(UART0_BASE);
		UARTEnable(UART0_BASE);
	}
};

class Uart0Transmitter
:	public Flow::Component
{
public:
	Flow::InPort<char> in;
	void run()
	{
		char toTransmit;
		while(UARTSpaceAvail(UART0_BASE) && in.receive(toTransmit)) // Transmit FIFO not full?
		{
			UARTCharPut(UART0_BASE, toTransmit);
		}
	}
	Uart0Transmitter()
	{
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
		GPIOPinConfigure(GPIO_PA1_U0TX);
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);
		UARTConfigSetExpClk(UART0_BASE, 120 MHz, 115200,
		                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
		                             UART_CONFIG_PAR_NONE));
		UARTFIFOEnable(UART0_BASE);
		UARTEnable(UART0_BASE);
	}
private:
	bool sending = false;
};

class Gpio
:	public Flow::Component
{
public:
	enum class Port : uint8_t
	{
		A = 'A',
		B = 'B',
		C = 'C',
		D = 'D',
		E = 'E',
		F = 'F',
		G = 'G',
		H = 'H',
		J = 'J',
		K = 'K',
		L = 'L',
		M = 'M',
		N = 'N',
		P = 'P',
		Q = 'Q',
		COUNT
	};
	enum class Direction
	{
		INPUT,
		OUTPUT
	};
	class Name
	{
	public:
		Name(Port port, unsigned int pin)
		:	port(port),
			pin(pin)
		{}
		Name(){}
		Port port;
		unsigned int pin;
		bool operator==(Name& other)
		{
			bool same = true;
			same = same && (this->port == other.port);
			same = same && (this->pin == other.pin);
			return same;
		}
	};
	Flow::InPort<bool> inValue;
	Flow::InPort<Direction> inDirection;
	Flow::InPort<Name> inName;
	Flow::OutPort<bool> outValue;
	void run()
	{
		Direction direction;
		if(inName.receive(name) && inDirection.receive(direction))
		{
			ASSERT(name.pin < 8);
			haveName = true;
			SysCtlPeripheralEnable(portPeripheral[(uint8_t)name.port]);
			if(direction == Direction::INPUT) {
				GPIOPinTypeGPIOInput(portBase[(uint8_t)name.port], (1 << name.pin));
			} else if (direction == Direction::OUTPUT) {
				GPIOPinTypeGPIOOutput(portBase[(uint8_t)name.port], (1 << name.pin));
			}

			//###

			if(name.port == Port::J)
			{
				GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
				GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
			}

			//###
		}

		if(haveName)
		{
			bool value;
			if(inValue.receive(value))
			{
				GPIOPinWrite(portBase[(uint8_t)name.port], (1 << name.pin), value ? 0xFF : 0x00);
			}

			{
				int32_t status = GPIOPinRead(portBase[(uint8_t)name.port], (1 << name.pin));
				outValue.send(status == (1 << name.pin));
			}
		}
	}
private:
	bool haveName = false;
	Name name;
	static const unsigned long portBase[(uint8_t)Port::COUNT];
	static const unsigned long portPeripheral[(uint8_t)Port::COUNT];
};

const unsigned long Gpio::portBase[(uint8_t)Port::COUNT] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	GPIO_PORTA_BASE,
	GPIO_PORTB_BASE,
	GPIO_PORTC_BASE,
	GPIO_PORTD_BASE,
	GPIO_PORTE_BASE,
	GPIO_PORTF_BASE,
	GPIO_PORTG_BASE,
	GPIO_PORTH_BASE,
	0,
	GPIO_PORTJ_BASE,
	GPIO_PORTK_BASE,
	GPIO_PORTL_BASE,
	GPIO_PORTM_BASE,
	GPIO_PORTN_BASE,
	0,
	GPIO_PORTP_BASE,
	GPIO_PORTQ_BASE
};

const unsigned long Gpio::portPeripheral[(uint8_t)Port::COUNT] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	SYSCTL_PERIPH_GPIOA,
	SYSCTL_PERIPH_GPIOB,
	SYSCTL_PERIPH_GPIOC,
	SYSCTL_PERIPH_GPIOD,
	SYSCTL_PERIPH_GPIOE,
	SYSCTL_PERIPH_GPIOF,
	SYSCTL_PERIPH_GPIOG,
	SYSCTL_PERIPH_GPIOH,
	0,
	SYSCTL_PERIPH_GPIOJ,
	SYSCTL_PERIPH_GPIOK,
	SYSCTL_PERIPH_GPIOL,
	SYSCTL_PERIPH_GPIOM,
	SYSCTL_PERIPH_GPION,
	0,
	SYSCTL_PERIPH_GPIOP,
	SYSCTL_PERIPH_GPIOQ
};

#endif /* TM4C_COMPONENTS_H_ */