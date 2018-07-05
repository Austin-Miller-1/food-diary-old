#include <stdio.h> //printing
#include <stdlib.h> //exiting
#include <unistd.h> //usleep
#include "pigpio.h" //library being used

#define SPI_SS 26//25

#define BITS 10
#define BX 8               // Bit position of data bit B9.
#define B0 (BX + BITS - 1) // Bit position of data bit B0.

#define MISO_PIN 13//23
#define ADCS 1
#define BUFFER 250
#define REPEAT_MICROS 5000
#define SAMPLES 2000000  // Number of samples to take,

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


void readADC(int* MISO,  //Pin connected to ADC's data out
int OOL,    //
int bytes,  //Bytes between readings
int bits,   //Bits per reading
char* buf);

rawSPI_t rawSPI =
{
  .clk = 5,
  .mosi = 12,
  .ss_pol = 1,
  .ss_us = 1,
  .clk_pol = 0,
  .clk_pha = 0,
  .clk_us = 1,
};


void readADC(
  int* MISO,  //Pin connected to ADC's data out
  int OOL,    //
  int bytes,  //Bytes between readings
  int bits,   //Bits per reading
  char* buf)
  {
    int currentBit, p;
    uint32_t level;

    p = OOL;
    for(currentBit = 0; currentBit < bits; currentBit++){
      level = rawWaveGetOut(p);
      putBitInBytes(currentBit, buf, level & (1<< MISO_PIN ));
      p--;
    }



  }




int main(int argc, char** args){
  int i, wid, offset;
  char buf[2];
  gpioPulse_t final[2];
  char rx[8];
  int sample;
  int val;
  int cb, botCB, topOOL, reading, now_reading;
  float cbs_per_reading;
  rawWaveInfo_t rwi;
  double start, end;
  int pause;

  pause = 0;

  //Set up library
  if(gpioInitialise()<0){perror("Could not init lib"); exit(EXIT_FAILURE);}

  //Set GPIO as outputs
  gpioSetMode(rawSPI.clk, PI_OUTPUT);
  gpioSetMode(rawSPI.mosi, PI_OUTPUT);
  gpioSetMode(SPI_SS, PI_OUTPUT);

  gpioWaveAddNew(); //Flush old unused wave data

  offset = 0;

/*
MCP3004/8 10-bit ADC 4/8 channels

1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17
SB SD D2 D1 D0 NA NA B9 B8 B7 B6 B5 B4 B3 B2 B1 B0

SB       1
SD       0=differential 1=single
D2/D1/D0 0-7 channel
*/

/*
     Now construct lots of bit banged SPI reads.  Each ADC reading
     will be stored separately.  We need to ensure that the
     buffer is big enough to cope with any reasonable rescehdule.

     In practice make the buffer as big as you can.
  */

  for (i=0; i<BUFFER; i++)
  {
     buf[0] = 0xC0; // Start bit, single ended, channel 0.

     rawWaveAddSPI(&rawSPI, offset, SPI_SS, buf, 2, BX, B0, B0);

     /*
        REPEAT_MICROS must be more than the time taken to
        transmit the SPI message.
     */

     offset += REPEAT_MICROS;
  }

  // Force the same delay after the last reading.

  final[0].gpioOn = 0;
  final[0].gpioOff = 0;
  final[0].usDelay = offset;

  final[1].gpioOn = 0; // Need a dummy to force the final delay.
  final[1].gpioOff = 0;
  final[1].usDelay = 0;

  gpioWaveAddGeneric(2, final);

  wid = gpioWaveCreate(); // Create the wave from added data.

  if (wid < 0)
  {
     fprintf(stderr, "Can't create wave, %d too many?\n", BUFFER);
     return 1;
  }

  /*
     The wave resources are now assigned,  Get the number
     of control blocks (CBs) so we can calculate which reading
     is current when the program is running.
  */

  rwi = rawWaveInfo(wid);

  printf("# cb %d-%d ool %d-%d del=%d ncb=%d nb=%d nt=%d\n",
     rwi.botCB, rwi.topCB, rwi.botOOL, rwi.topOOL, rwi.deleted,
     rwi.numCB,  rwi.numBOOL,  rwi.numTOOL);

  /*
     CBs are allocated from the bottom up.  As the wave is being
     transmitted the current CB will be between botCB and topCB
     inclusive.
  */

  botCB = rwi.botCB;

  /*
     Assume each reading uses the same number of CBs (which is
     true in this particular example).
  */

  cbs_per_reading = (float)rwi.numCB / (float)BUFFER;

  printf("# cbs=%d per read=%.1f base=%d\n",
     rwi.numCB, cbs_per_reading, botCB);

  /*
     OOL are allocated from the top down. There are BITS bits
     for each ADC reading and BUFFER ADC readings.  The readings
     will be stored in topOOL - 1 to topOOL - (BITS * BUFFER).
  */

  topOOL = rwi.topOOL;

  fprintf(stderr, "starting...\n");

  if (pause) time_sleep(pause); // Give time to start a monitor.

  gpioWaveTxSend(wid, PI_WAVE_MODE_REPEAT);

  reading = 0;

  sample = 0;

  start = time_time();

  while (sample<SAMPLES)
  {
     // Which reading is current?

     cb = rawWaveCB() - botCB;
     //printf("RawWAveCb: %d \t BotCb: %d \t \n", rawWaveCB(), botCB);

     now_reading = (float) cb / cbs_per_reading;
     printf("nowReading: %d \t Reading: %d \t Cb: %d\n", now_reading, reading, cb);
     // Loop gettting the fresh readings.

     while (now_reading != reading)
     {
        /*
           Each reading uses BITS OOL.  The position of this readings
           OOL are calculated relative to the waves top OOL.
        */
        readADC(NULL, topOOL - ((reading%BUFFER)*BITS) - 1, 2, BITS, rx);

        //printf("%d", ++sample);

        for (i=0; i<ADCS; i++)
        {
           //  7  6  5  4  3  2  1  0 |  7  6  5  4  3  2  1  0
           // B9 B8 B7 B6 B5 B4 B3 B2 | B1 B0  X  X  X  X  X  X

           val = (rx[i*2]<<2) + (rx[(i*2)+1]>>6);
        // printf(" %d \t %f", val, convert_volt_to_centi(val));
        }

        printf("\n");

        if (++reading >= BUFFER) reading = 0;
     }
     usleep(1000);
  }

  end = time_time();

  printf("# %d samples in %.1f seconds (%.0f/s)\n",
     SAMPLES, end-start, (float)SAMPLES/(end-start));

  fprintf(stderr, "ending...\n");

  if (pause) time_sleep(pause);

  gpioTerminate();

  return 0;
}

//Converts given voltage to the distance between sensor and object in centimeters.
double convert_volt_to_centi(double voltage) {
	//if (voltage < VOLTS_80CM_AWAY || voltage > VOLTS_10CM_AWAY) { printf("Distance is not accurate.\t"); }
	//return VOLT_DIST_CONSTANT / voltage;
  return 1/(voltage*2e-4);
}
