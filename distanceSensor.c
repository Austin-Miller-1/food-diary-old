#include "distanceSensor.h"
#include "pigpio.h" //library being used
#include <stdio.h> //printing
#include <stdlib.h> //exiting
#include <unistd.h> //usleep

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

int init_gpio(){
  if(gpioInitialise()<0){perror("Could not init lib"); return -1;}

  //Set GPIO as outputs
  gpioSetMode(rawSPI.clk, PI_OUTPUT);
  gpioSetMode(rawSPI.mosi, PI_OUTPUT);
  gpioSetMode(SPI_SS, PI_OUTPUT);

  gpioWaveAddNew(); //Flush old unused wave data

  return 1;
}

int init_buffer(int* offset){
  char buf[2];
  int i;
  for (i=0; i<BUFFER; i++)
  {
     buf[0] = 0xC0; // Start bit, single ended, channel 0.

     rawWaveAddSPI(&rawSPI, *offset, SPI_SS, buf, 2, BX, B0, B0);

     /*
        REPEAT_MICROS must be more than the time taken to
        transmit the SPI message.
     */

     *offset = *offset + REPEAT_MICROS;
  }
  return 1;
}


//Converts given voltage to the distance between sensor and object in centimeters.
double convert_volt_to_centi(double voltage) {
	//if (voltage < VOLTS_80CM_AWAY || voltage > VOLTS_10CM_AWAY) { printf("Distance is not accurate.\t"); }
	//return VOLT_DIST_CONSTANT / voltage;
  return 1/(voltage*2e-4);
}

double get_distance(int* readingg, int offset, int mode){
  static rawWaveInfo_t rwi;
  static int wid;
  static gpioPulse_t final[2];
  if(!mode){
  //printf("got here");
  //int r = 0;
  //int* reading = &r;
  //gpioPulse_t final[2];
  final[0].gpioOn = 0;
  final[0].gpioOff = 0;
  final[0].usDelay = offset;

  final[1].gpioOn = 0; // Need a dummy to force the final delay.
  final[1].gpioOff = 0;
  final[1].usDelay = 0;

  gpioWaveAddGeneric(2, final);

   wid = gpioWaveCreate();
   rwi = rawWaveInfo(wid);
   gpioWaveTxSend(wid, PI_WAVE_MODE_REPEAT);
 }
 int r = 0;
 int* reading = &r;
  int botCB = rwi.botCB;
  int cbs_per_reading = (float)rwi.numCB / (float)BUFFER;
  int topOOL = rwi.topOOL;



  int pause = 0;
  int now_reading = 0;
  int cb = 0;
  if (pause) time_sleep(pause); // Give time to start a monitor.

  int gotDistance = 0;
  char rx[8];
  int val;
  while (!gotDistance)
  {
     // Which reading is current?
     //printf("RawWAveCb: %d \t BotCb: %d \t \n", rawWaveCB(), botCB);
     cb = rawWaveCB() - botCB;

     now_reading = (float) cb / cbs_per_reading;
     //printf("nowReading: %d \t Reading: %d \t Cb: %d\n", now_reading, *reading, cb);
     // Loop gettting the fresh readings.

     while (now_reading != *reading)
     {
       //printf("nowReading: %d \t Reading: %d \t Cb: %d\n", now_reading, *reading, cb);
        readADC(NULL, topOOL - (((*reading)%BUFFER)*BITS) - 1, 2, BITS, rx);
        gotDistance = 1;

		val = (rx[0]<<2) + (rx[1]>>6);
        //printf(" %d \t %f", val, convert_volt_to_centi(val));


        //printf("\n");

        *reading = *reading+1;
        if (*reading >= BUFFER) (*reading) = 0;
        return convert_volt_to_centi(val);
     }
     usleep(1000);
  }
	return 0;
}
