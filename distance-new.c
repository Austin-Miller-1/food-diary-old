
#include <stdio.h>	// For printing
#include <stdlib.h> // For exit
#include <unistd.h> //For usleep
#include "pigpio.h"

#define SPI_CHANNEL 0
#define PIN_BASE 100 //Inputs to mcp chip are on 'fake pins' 100 - 107. We access all pins for entire analog input.

/* The voltage will be given as a 10bit integer. This means that there will be 1024 different values.
 (0-1023). These values correspond to different voltage values, from 0 V to 5 V. Therefore, to
 convert a given voltage interger input to the actual voltage value, we would multiply by the ratio, 5V/1023. */
#define VOLT_TO_BIT_RATIO (5.0/1024)

//These constants are interpreted from manufacturer's data sheet. Will need to text on own since different voltage (5V) is used.
#define VOLTS_10CM_AWAY 3.0	//Expected output voltage from sensor when object is 10 cm away
#define VOLTS_80CM_AWAY 0.4 //Expected output voltage from sensor when object is 80 cm away.

//The voltage and the distance are inversely proportional. That is, Output Voltage = m * 1/distance.
//In order to determine the distance, we need this proportionality constant, which we can define by using either
//of the preexxiting points in this function, (3.0 V, 10CM) or (0.4V, 80CM). The slope of the line made by these two points is found.
#define VOLT_DIST_CONSTANT 29.7

double convert_volt_to_centi(double voltage); //Converts given voltage to the distance between sensor and object in centimeters.

int main(int ac, char** args) {

	//Initializes 'wiringPi' library using its GPIO pin numbering scheme.
	//If init fails, exit.
	//if (wiringPiSetup() == -1) { perror("Could not initialize 'wiringPi' library."); exit(EXIT_FAILURE); }

	//int mcp3008device = mcp3004Setup(PIN_BASE, SPI_CHANNEL);
	//Set up library
	if(gpioInitialise()<0){perror("Could not init lib"); exit(EXIT_FAILURE);}
	//Set up SPI
	unsigned int handle = spiOpen(1,32000,0); //Flags: 000000 0 0 0000 0 0 000 000 00
	if(handle<0){perror("Bad SPI"); exit(EXIT_FAILURE);}

	char buffer[3];
	int chan;
	int bytesRead;
	int voltageBits; double voltage;
	for (;;) {
		//Print all channel data:
		//1) Print voltage bits, interpreted voltage & distance:
		//voltageBits = analogRead(PIN_BASE);
		bytesRead = spiRead(handle, buffer, 2); //Read two bytes from SPI CE1
		printf("bytes read: %d\t", bytesRead);
		printf("byte 1 & byte 2: %d %d \n", (int) buffer[0], (int) buffer[1]);

		//voltageBits = (int) buffer[0]
		//voltage = voltageBits * VOLT_TO_BIT_RATIO;
		//printf("Distance: %f\n", convert_volt_to_centi(voltage));

		/*
		printf("Channel %d:\tDigital Val: %d\tVoltage:%f\tDistance(cm):%f\n", PIN_BASE, voltageBits, voltage, convert_volt_to_centi(voltage));
		for (chan = 1; chan < 8; chan++) {
			printf("Digital value %d: %d \n", PIN_BASE + chan, analogRead(PIN_BASE + chan));
		}
		printf("----------------------\n");
		*/
		//delay(500);
		usleep(500000);
	}

	spiClose(handle);
	//int voltage_bits = analogRead(PIN_BASE);
	//double voltage = voltage_bits * VOLT_TO_BIT_RATIO;
	//double distanceCM = convert_volt_to_centi(voltage);

	return 0;
}

//Converts given voltage to the distance between sensor and object in centimeters.
double convert_volt_to_centi(double voltage) {
	if (voltage < VOLTS_80CM_AWAY || voltage > VOLTS_10CM_AWAY) { printf("Distance is not accurate.\t"); }
	return VOLT_DIST_CONSTANT / voltage;
}
