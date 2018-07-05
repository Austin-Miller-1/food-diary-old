#define SPI_SS 26//25

#define BITS 10
#define BX 8               // Bit position of data bit B9.
#define B0 (BX + BITS - 1) // Bit position of data bit B0.

#define MISO_PIN 13//23
#define ADCS 1
#define BUFFER 250
#define REPEAT_MICROS 5000
#define SAMPLES 1//2000000  // Number of samples to take,

/* The voltage will be given as a 10bit integer. This means that there will be 1024 different values.
 (0-1023). These values correspond to different voltage values, from 0 V to 5 V. Therefore, to
 convert a given voltage interger input to the actual voltage value, we would multiply by the ratio, 5V/1023. */
#define VOLT_TO_BIT_RATIO (5.0/1024)

//These constants are interpreted from manufacturer's data sheet. Will need to text on own since different voltage (5V) is used.
#define VOLTS_10CM_AWAY 3.0	//Expected output voltage from sensor when object is 10 cm away
#define VOLTS_80CM_AWAY 0.4 //Expected output voltage from sensor when object is 80 cm away.

#define VAL_10CM_AWAY 480
#define VAL_30CM_AWAY 210
//#define VAL_45CM_AWAY

//The voltage and the distance are inversely proportional. That is, Output Voltage = m * 1/distance.
//In order to determine the distance, we need this proportionality constant, which we can define by using either
//of the preexxiting points in this function, (3.0 V, 10CM) or (0.4V, 80CM). The slope of the line made by these two points is found.
#define VOLT_DIST_CONSTANT 29.7

double convert_volt_to_centi(double voltage); //Converts given voltage to the distance between sensor and object in centimeters.
int init_gpio();
int init_buffer(int* offset);
double get_distance(int* reading, int offset, int mode);

void readADC(int* MISO,  //Pin connected to ADC's data out
int OOL,    //
int bytes,  //Bytes between readings
int bits,   //Bits per reading
char* buf);
