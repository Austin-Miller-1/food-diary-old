#include <libusb-1.0/libusb.h> //For connectivity to and communication with USB Scale
#include "usbscale.h"
#include "CameraModule.h"
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

  //Set up picture settings
  CameraFlags cameraFlags;
  cameraFlags.hf = 1;
  cameraFlags.vf = 1;

  //Read from scale and sensor
  double mass = 0.0;
  double newMass = 0.0;

  double distance = 0.0;
  double newDistance = 0.0;

  int photoTaken = 0; int isBarcode = 0;
//  while(mass >= 0.0){
    //Get mass and distance
    //get_avg_mass_and_dist(&newMass, &newDistance);
//    newMass = get_scale_mass(scale, handle);


  //  printf("%lf\n", newMass);

    //Get distance
//While the scale is returning a valid value..
while(mass >= 0.0){
  //While the mass is unchanged. We keep testing until the mass increases and is stable, then send data, then repeat the process!
  while(mass == 0.0){

      //If distance sensor is reporting in range value, take photo of barcode
      //Else, check to see if the mass changed first, and take photo of the food on the scale
      if(!photoTaken && newDistance >= LOWER_BND && newDistance <= UPPER_BND){
        printf("Photo of barcode taken! (only photo)\n");
        //take_picture("barcode",cameraFlags);
        isBarcode = 1;
        photoTaken = 1;
      }
      else{
        //If the mass has changed, then we need to either send the mass with barcode or take photo of food and send that phoot with mass (one pair or the other)
        if(mass!=0.0){
          //Get stable mass (wait half a second)


          //If a photo of the barcode has already been taken, we just need to send mass. Otherwise, get photo and send mass AND photo
          if(photoTaken){
            //Create json
          }
          else{
            //Take photo of food on scale
            //take_picture("food", cameraFlags);

            //Create json

            isBarcode = 0;
            photoTaken = 1;
          }

        }
      }
    }

  //Create connection, Send data

  //End connection

  //Create connection, Send image

  //Wait until the mass is 0 before continuing (note: this does not allow zeroing out & using plates or bowls)
  while(  (mass = get_scale_mass(scale, handle)) != 0.0){printf("Please remove mass");}

}
    //Update values

  }
  
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
