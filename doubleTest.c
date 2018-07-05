#include <libusb-1.0/libusb.h> //For connectivity to and communication with USB Scale
#include "usbscale.h"
#include "CameraModule.h"
#include "distanceSensor.h"
#include "pigpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **args){

//Set up Scale
  //Set up libusb library to access scale. If failed, maybe do something (try again, exit program, etc) .
    if(libusb_library_setup()<0){;}


    //Set up scale (should only be done once, BUT may need to be done multiple times)
    libusb_device* scale = NULL;
    libusb_device_handle* handle = NULL;
    libusb_device **usbDevs = NULL;
    setup_usbscale(&scale, &handle, usbDevs);
    double mass = 0.0;

//Set up Distance Sensor
  double distance;
  int offset;  int reading;
  if( init_gpio() < 0){printf("could not init lib"); exit(EXIT_FAILURE);}

  offset = 0;
  init_buffer(&offset);
  reading = 0;
  get_distance(&reading, offset, 0);


  //Get Values from both simultaneously
  int i;
  //for(i = 0; i < 5; i++){
  for(;;){
    distance = get_distance(&reading, offset, 1);
    mass = get_scale_mass(scale, handle);
    printf("%f cm, %f g\n", distance, mass);
    //usleep(5000);
  }

  fprintf(stderr, "ending...\n");

  gpioTerminate();
  return 0;

}
