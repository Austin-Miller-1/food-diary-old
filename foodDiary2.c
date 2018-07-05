#include <libusb-1.0/libusb.h> //For connectivity to and communication with USB Scale
#include "usbscale.h"
#include "CameraModule.h"
#include "distanceSensor.h"
#include "pigpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h> //For usleep
#include <b64/cencode.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TIME_TO_WAIT 250000
#define LOWER_BND 7.0
#define UPPER_BND 9.0
//void get_avg_mass_and_dist(*double mass, *double distance, libusb_device* scale, libusb_device_handle* handle);

/*
{
  "data":"image/jpeg",
  "imageName":"__",
  "mode":"barcode/food",
  "mass":"mass",
  "base64": ...
}

*/
void create_JSON(char* JSONBuffer, char* imageName, int isBarcode, double mass);

void exit_program(libusb_device_handle* handle, libusb_device **usbDevs);

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

  //Set up distance sensor
  double distance;
  int offset;  int reading;
  if( init_gpio() < 0){printf("could not init lib"); exit(EXIT_FAILURE);}

  offset = 0;
  init_buffer(&offset);
  reading = 0;
  get_distance(&reading, offset, 0);

  //Set up picture settings
  CameraFlags cameraFlags;
  cameraFlags.hf = -1;
  cameraFlags.vf = -1;

  //JSON BUFFER
  char JSON[5000];

  //Read from scale and sensor
  double mass = 0.0;
  double newMass = 0.0;

  distance = 0.0;
  double newDistance = 0.0;

  int photoTaken = 0; int isBarcode = 0;
  //char* JSON = NULL;
//    newMass = get_scale_mass(scale, handle);


  //  printf("%lf\n", newMass);

    //Get distance



//For some number of foods (may be infinitely many), determine the nutritional values
int fd = 0;
int maxfood = 5;
for(fd = 0; fd < 5; fd++){

  //Default values
  photoTaken = 0;
  JSON[0] = '\0';
  printf("Scan a barcode or place something on the scale!\n");

  //While no food is on scale, check to see if barcode is scanned.
  while(mass == 0.0){

      //Get stable distance
      distance = get_distance(&reading, offset, 1);
      //printf("Distance obtained: %f cm\n",distance);

      //If distance sensor is reporting in range value, take photo of barcode
      //Else, check to see if the mass changed first, and take photo of the food on the scale
      if(!photoTaken && distance >= LOWER_BND && distance <= UPPER_BND){
        printf("Object detected %f cm ahead, photo of barcode taken! Put the food on scale.\n",distance);
        take_picture("barcode.jpg",cameraFlags);
        //isBarcode = 1;
        photoTaken = 1;
        //exit_program(handle, usbDevs);
      }

      usleep(10000); //Slow down check rate for distance
      mass = get_scale_mass(scale, handle);
    }

  //Once food has been put on the scale, we need to measure the mass,
  //and pack it in the correct JSON file (is image a barcode or a food?)

  //Get stable mass (wait half a second)
  double newMass = -1;
  //double temp;
  while(newMass != mass){
      mass = newMass;
      usleep(4 * TIME_TO_WAIT);
      newMass = get_scale_mass(scale, handle);

  }
  printf("Mass of food obtained: %f g\n",newMass);

  //If photo has already been taken, then it was of a barcode.
  //Package JSON file with mass and send it to web service.
  if(photoTaken){
      //Create json
    printf("Mass of food associated with barcode found! -> JSON for barcode created\n");
    create_JSON(JSON, "barcode.jpg",1, mass);
  }
  else{ //If not, then photo must be of food.

  //Take photo of food on scale
  printf("Photo of food taken! -> JSON for food created\n");
  take_picture("food.jpg", cameraFlags);

  //Create json
            //isBarcode = 0;
            //photoTaken = 1;
  create_JSON(JSON, "food.jpg", 0, mass);
  }


  printf("\nThis is the JSON file: %s\n", JSON);
    //Create connection, Send data

    //End connection

    //Create connection, Send image

    //Wait until the mass is 0 before continuing
    printf("\n\n\n");
    printf("Please remove mass\n");
    while(  (mass = get_scale_mass(scale, handle)) != 0.0){;}
    printf("Mass removed! Allowing for next food!\n");
    printf("\n\n\n");
  }

  //Close necessary devices

  //printf("made it here");
  //return 1;
  return end_scale(handle, usbDevs);
}


void create_JSON(char* buffer, char* imageName, int isBarcode, double mass){

/*
  {
    "data":"image/jpeg",
    "imageName":"__",
    "mode":"barcode/food",
    "mass":"mass",
    "base64": ...
  }
  */
  char* mode;
  if(isBarcode){mode = "barcode";}
  else{ mode = "food";}

  //Convet img to base64
  char commandToRun[1000];
  char newImageName[100];
  snprintf(newImageName, "e%s", imageName);
  snprintf(commandToRun, "./base64 -e %s %s", imageName, newImageName);

  //Open and read img into buffer

  char base64[4750];
  int fd;
  if( (fd = open(newImageName, O_RDONLY)) < 0){perror("could not open base64 file"); exit(0);};

  int bytesRead = read(fd, base64, 4749);
  if(bytesRead == 0){perror("could not read file");}
  base64[bytesRead] = '\0';

  snprintf(buffer, 1000,
    "{
      \"data\":\"image/jpeg\",
      \"imageName\":\"%s\",
      \"mode\":\"%s\",
      \"mass\":\"%f\",
      \"base64\":\"%s\"
    }",
    newImageName, mode, mass, base64);

}

//void image_to_base64(char* imageName, char* buffer){



//}


void exit_program(libusb_device_handle* handle, libusb_device **usbDevs){
  gpioTerminate();
  end_scale(handle, usbDevs);
}

/*
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
*/
