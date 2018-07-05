#include <libusb-1.0/libusb.h> //For connectivity to and communication with USB Scale
#include "usbscale.h"
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h> //For usleep

#define TOTAL_NUM_OF_VALS 3
#define TIME_TO_WAIT 250000
void get_avg_mass_and_dist(*double mass, *double distance, libusb_device* scale, libusb_device_handle* handle);

//Constantly reports the mass on the scale
int main(void)
{

  //Set up libusb library to access scale. If failed, maybe do something (try again, exit program, etc) .
  if(libusb_library_setup()<0){;}


  //Set up scale (should only be done once, BUT may need to be done multiple times)
  libusb_device* scale = NULL;
  libusb_device_handle* handle = NULL;
  libusb_device **usbDevs = NULL;
  setup_usbscale(&scale, &handle, usbDevs);


  //Read from scale and sensor
  double mass = 0.0;
  double newMass = 0.0;

  double distance = 0.0;
  double newDistance = 0.0;

  while(mass >= 0.0){
    //Get mass and distance
    //get_avg_mass_and_dist(&newMass, &newDistance);
    newMass = get_scale_mass(scale, handle);


    printf("%lf\n", newMass);

    //Get distance



    //



    //Update values
    mass = newMass; distance = newDistance;
  }

  //Read from distance sensor


  //Close necessary devices

  //printf("made it here");
  //return 1;
  return end_scale(handle, usbDevs);
}

void get_avg_mass_and_dist(*double mass, *double distance, libusb_device* scale, libusb_device_handle* handle){
  int massFound = 0; int distanceFound = 0;

  int mass1 = 0; int mass2 = 0;
  int distance1 =0; int distance2 = 0;

  mass1 = get_scale_mass(scale,handle);
  //distance1 =

  while(!massFound || !distanceFound){
    usleep(TIME_TO_WAIT);
    if(!massFound){
      if( (mass2 = get_scale_mass(scale,handle)) != mass1 ){ mass1 = mass2;}
      else{ massFound = 1; &mass = mass2; }
    }

    //Do the same for distance


  }

}
