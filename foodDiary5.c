#include <libusb-1.0/libusb.h> //For connectivity to and communication with USB Scale
#include "usbscale.h"
#include "CameraModule.h"
#include "distanceSensor.h"
#include "pigpio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h> //For usleep
#include <sys/stat.h>
#include <fcntl.h>
//#include <signal.h> //For signal

#define TIME_TO_WAIT 250000
#define LOWER_BND 7.0
#define UPPER_BND 9.0
#define WEBSITE "https://fd-website.herokuapp.com"
#define IMGUR_PATH "/api/getimgurtoken"
#define DATA_PATH "/api/datapath"
#define IMGUR_API "https://api.imgur.com/3/image"
#define BARCODE_IMG_NAME "barcode.jpg"
#define FOOD_IMG_NAME "food.jpg"
#define UPC_FILE_NAME "upc"
#define IMGUR_TOKEN_FILE_NAME "tokenResponse"
#define IMGUR_RESPONSE_FILE_NAME "imgurResponse"
#define SERVER_RESPONSE_FILE_NAME "serverResponse"
#define MAX_TOKEN_SIZE 100
#define SERVER_BUFFER_SIZE 250
#define JSON_BUFFER_SIZE 4750
#define DEFAULT_TOKEN "df135dc4e790bf8b95b17d5fa132514b20f58536"
#define UPC_MAX 13
#define MASS_MINIMUM 6.0 //Due to inconsistent readings from scale, it may detect values between 0 and 6 when nothing is on it

//Get imgur authentification token by sending request to web service
int get_imgur_token(char* token, char* albumId, char* username);

//Places upc code in given string
int process_barcode(char* upcString, char* imageName);

//If errors occur, reset scale
int reset_scale(libusb_device* scale, libusb_device_handle* handle, libusb_device **usbDevs);

//Upload image , (upc is upc for barcode, null otherwise )
void upload_data(char* upc, char* imageName, char* token, double mass, char* albumId);

//End program
void exit_program(libusb_device_handle* handle, libusb_device **usbDevs, char* upc);



//Constantly reports the mass on the scale
int main(void)
{
  //User login
  char username[40];
  printf("Enter username: \n");
  scanf ("%[^\n]%*c", username);
  //fgets(username, 40, stdin);
  //scanf("%s\n", username);

  //Imgur setup
  char imgurToken[MAX_TOKEN_SIZE];
  char imgurAlbumId[MAX_TOKEN_SIZE];
  if( get_imgur_token(imgurToken, imgurAlbumId, username) < 0){
    printf("Could not get imgur access token... using default\n");
    memcpy( imgurToken, DEFAULT_TOKEN, strlen( DEFAULT_TOKEN ));
    //exit(1);
  }

  //Set up libusb library to access scale. If failed, maybe do something (try again, exit program, etc) .
  if(libusb_library_setup()<0){;}

  //Set up scale (should only be done once, BUT may need to be done multiple times in case of serious malfunction [note: not the case atm])
  libusb_device* scale = NULL;
  libusb_device_handle* handle = NULL;
  libusb_device **usbDevs = NULL;
  setup_usbscale(&scale, &handle, usbDevs);

  //Set up distance sensor with pigpio library
  double distance;
  int offset;  int reading;
  if( init_gpio() < 0){printf("could not init lib"); exit(EXIT_FAILURE);}

  //More distance sensor initialization
  offset = 0;
  init_buffer(&offset);
  reading = 0;
  get_distance(&reading, offset, 0); //Get first value to get rid of garbage data

  //Set up picture settings
  CameraFlags cameraFlags;
  cameraFlags.hf = -1;
  cameraFlags.vf = -1;

  //Variables for reading scale
  double mass = 0.0;
  double newMass = 0.0;

  //Variables for reading distance sensor
  distance = 0.0;
  //double newDistance = 0.0;

  int photoTaken = 0;       //Has a photo been taken in this iteration BEFORE mass has been found? If so, then the program flow has been decided (yes -> barcode & food, no -> only food)
  char upc[UPC_MAX];        //Will contain UPC code for any barcodes being scanned
  char* imageName = NULL;   //Image name (will be 'food.jpg' or 'barcode.jpg')

  /*Process:
  * Check both distance and mass sensors. If distance enters the specified range before mass changes, take a photo. This photo is understood to be a barcode.
  * If mass changes first, then do not take a photo.
  * Once mass changes, if a photo has not been taken, then take a photo. This photo is understood to be a food.
  * Otherwise, this mass goes to the food associated with the barcode.
  *
  * If barcode -> send to server {upc, mass, mode(is barcode)}
  * If food -> send photo to imgur for storage, send to server {mass, mode(is food)}
  */
  //Keep scanning to infinity
  for(;;){

    //Default values
    photoTaken = 0;

    printf("Scan a barcode or place something on the scale!\n");

    //While no food is on scale, check to see if barcode is scanned.
    while(mass <= MASS_MINIMUM){ //6 grams is our minimum value, since scale may detect when no such weight

        //Get stable distance
        distance = get_distance(&reading, offset, 1);
        //printf("Distance obtained: %f cm\n",distance);

        //If distance sensor is reporting in range value, take photo of barcode
        //Else, check to see if the mass changed first, and take photo of the food on the scale
        if(!photoTaken && distance >= LOWER_BND && distance <= UPPER_BND){
          printf("Object detected %f cm ahead, photo of barcode taken!\n Processing barcode...:\n\n",distance);
          imageName = BARCODE_IMG_NAME;
          take_picture(imageName,cameraFlags);
          if( process_barcode(upc, imageName) == -1){
            printf("No barcode detected!\n");
            photoTaken = 0;
          }
          else{
            printf("\nUPC CODE: %s\n", upc);
            printf("Place the food with this UPC on the scale.");
            photoTaken = 1;
          }
        }
        usleep(10000);  //Slow down check rate for distance, wait small amt of time
        mass = get_scale_mass(scale, handle);
      }

    //Once food has been put on the scale, we need to measure the mass.

    //Get stable mass (wait half a second)
    newMass = -1; //Give impossible first value
    while(newMass != mass){
        mass = newMass;
        usleep(4 * TIME_TO_WAIT); //For stability, wait 1 second before calling scale again
        newMass = get_scale_mass(scale, handle); //Get new mass
    }

    //Once stable mass has been found..
    if(newMass >= mass){

      printf("Mass of food obtained: %f g\n",newMass);

      //If photo has already been taken, then it was of a barcode.
      //Package JSON file with mass and send it to web service.
      if(photoTaken){
          //Create json
        printf("Mass of food associated with barcode found! -> JSON for barcode created\n");
        upload_data(upc, imageName, imgurToken, newMass, imgurAlbumId);
        //create_JSON(JSON, "barcode.jpg",1, mass);
      }
      else{ //If not, then photo must be of food.

      //Take photo of food on scale
      printf("Photo of food taken! -> JSON for food created\n");
      imageName = FOOD_IMG_NAME;
      take_picture(imageName, cameraFlags);
      upload_data(NULL, imageName, imgurToken, newMass, imgurAlbumId);
      }

        //Wait until the mass is 0 before continuing
      printf("\n\n\n");
      printf("Please remove mass\n");
      while(  (mass = get_scale_mass(scale, handle)) >= 6.0){;}
      printf("Mass removed! Allowing for next food!\n");
      printf("\n\n\n");
      }
    }

  //Close necessary devices

  //printf("made it here");
  //return 1;
  exit_program(handle, usbDevs, upc);
  return 1;
}

int get_imgur_token(char* token, char* albumId, char* username){
  int resultBufSize = MAX_TOKEN_SIZE+300;
  char commandToRun[150];
  char resultBuffer[resultBufSize];

  unlink(IMGUR_TOKEN_FILE_NAME);
  //Send request to server to get imgur authentification key, save response to 'response' file
  snprintf(commandToRun, 150, "echo $(http GET %s/api/getimgurtoken 'username=%s') >> %s", WEBSITE, username, IMGUR_TOKEN_FILE_NAME);
  system(commandToRun);

  //Open response file, read data
  int fd;
  if( (fd = open(IMGUR_TOKEN_FILE_NAME, O_RDONLY, "r"))<0 ){printf("Could not open Imgur Token file\n"); return -1;}
  if( read(fd, resultBuffer, 500) <= 0){
    printf("Could not read token file\n");
    return -1;
  }

  //Get albumId position
  int c; int albumIdStart = 0; int albumIdEnd = 0; int albumIdSize;
  for(c = 0; c < resultBufSize; c++){
    if(resultBuffer[c] == ':'){
      albumIdStart = (c+=2); //Start of token is therefore 2 characters ahead ( "field":"token" )
      while(resultBuffer[++c] != '\"'){;}  //Keep going until the end quote is found
      albumIdEnd = c; //End token
      break;
    }
  }
  printf("album. start: %d, end %d\n", albumIdStart, albumIdEnd);

  albumIdSize = albumIdEnd - albumIdStart;
  memcpy(albumId, resultBuffer+albumIdStart, albumIdSize);
  albumId[albumIdSize] = '\0';

  printf("Imgur album ID: %s\n", albumId);

  //Get token position
  int tokenStart = 0; int tokenEnd = 0; int tokenSize;
  for(c ; c < resultBufSize; c++){
    if(resultBuffer[c] == ':'){ //Assuming there is one field, find the colon
      tokenStart = (c+=2); //Start of token is therefore 2 characters ahead ( "field":"token" )
      while(resultBuffer[++c] != '\"'){;}  //Keep going until the end quote is found
      tokenEnd = c; //End token
      break;
    }
  }
  tokenSize = tokenEnd - tokenStart;

  //Copy token
  memcpy(token, resultBuffer+tokenStart, tokenSize);
  printf("token. start: %d, end %d\n", tokenStart, tokenEnd);
  //resultBuffer[tokenSize] = '\0';
  token[tokenSize] = '\0';

  //Print result for testing
  printf("Imgur Token: %s\n", token);
  return 1;
}



int process_barcode(char* upc, char* imageName){
  char commandToRun[100]; //Will hold the command to shrink image and to process smaller image
  char resultBuffer[50]; //Will hold result to processing barcode

  //Delete previous barcode file
  unlink(UPC_FILE_NAME);

  //Shrink image command
  snprintf(commandToRun, 100, "convert %s -resize 1224x919 small-%s", imageName, imageName);
  system(commandToRun);

  //Process image command, write upc code to file
  //snprintf(commandToRun, 100, "echo $(./BarcodeScanner.exe small-%s) >> upc", imageName);
  snprintf(commandToRun, 100, "./BarcodeScanner.exe small-%s", imageName);
  system(commandToRun);

  //Open file, read data
  int fd;
  if( (fd = open(UPC_FILE_NAME, O_RDONLY, "r"))<0 ){printf("Could not open UPC file\n"); return -1;}
  if( read(fd, resultBuffer, 50) <= 0){
    //resultBuffer[0] = '\0';
    printf("Could not read UPC file\n");
    return -1;
  }

  if(!strcmp(resultBuffer,"NULL")){printf("No UPC detected"); return -1;}
  else{
    //If everything went right, save the upc from the file to the upc string pointer
    snprintf(upc, 50, "%s", resultBuffer);

    //printf("\nUPC CODE: %s\n", upc);
    return 1;
  }
}

void upload_data(char* upc, char* imageName, char* token, double mass, char* albumId){
  char commandToRun[150];
  //char token[MAX_TOKEN_SIZE];
  //char base64[4750];
  //char imgurJSON[MAX_TOKEN_SIZE+100];
  //char serverJSON[SERVER_BUFFER_SIZE];
  //If food...
    //1. request server for imgur token
    //2. contact imgur and upload image as base64
    //3. contact server again

    if( upc == NULL){
      //int newImageNameLen = strlen(imageName) + 1;
      //char newImageName[newImageNameLen];

      //Get token (if did not already)
      //get_imgur_token(token);


      //Convet img to base64
      //snprintf(newImageName, newImageNameLen, "e%s", imageName);
      //snprintf(commandToRun, 150, "./base64 -e %s %s", imageName, newImageName);

      //int result;
      //if((result = system(commandToRun)) == -1){
      //  perror("Error encoding image!");
      //}

      //Open and read img into token buffer
      //int fd;
      //if( (fd = open(newImageName, O_RDONLY)) < 0){perror("could not open base64 file"); exit(0);};

      /*
      int bytesRead = read(fd, base64, 4749);
      if(bytesRead == 0){perror("could not read file");}
      base64[bytesRead] = '\0';
      //snprintf(buffer, 4750,"{\"imageName\":\"%s\",\"mode\":\"%s\",\"mass\":\"%f\",\"imageData\":\"data:image/jpeg;base64,%s\"}", newImageName, mode, mass, base64);
      snprintf(imgurJSON, 5000,"{\"image\":\"%s\",\"title"\:\"%s\"}\0", base64, imageName);
      */


      //Make header for imgur
      char header[MAX_TOKEN_SIZE+100];
      snprintf(header, MAX_TOKEN_SIZE+100, "Authorization: Bearer %s", token);

      //Send request to imgur API & save response
      commandToRun[0] = '\0'; //Reset command buffer
      snprintf(commandToRun, 1000, "$(curl -X POST -H '%s' %s -F image=@%s -F title=food -F album=%s ) >> imgurResponse", header, IMGUR_API, imageName, albumId);
      //system(commandToRun);

      /*
      //Read response & get imgur id
      int fd;
      if( (fd = open(IMGUR_RESPONSE_FILE_NAME, O_RDONLY, "r"))<0 ){printf("Could not open Imgur Token file\n"); return -1;}
      if( read(fd, resultBuffer, 500) <= 0){
        printf("Could not read UPC file\n");
        return;
      }

      //Get token position
      int c; int tokenStart; int tokenEnd; int tokenSize;
      for(c = 0; c < resultBufSize; c++){
        if(resultBuffer[c] == ':'){ //Assuming there is one field, find the colon
          tokenStart = (c+=2); //Start of token is therefore 2 characters ahead ( "field":"token" )
          while(resultBuffer[++c] != '\"'){;}  //Keep going until the end quote is found
          tokenEnd = c-1; //End token
        }
      }
      tokenSize = tokenEnd - tokenStart;

      resultBuffer[tokenSize] = '/0';
      //Delete response so that it does not append to itself each time
      unlink(IMGUR_RESPONSE_FILE_NAME);
      */


      printf("Message regarding imgur:  %s ", commandToRun);

      //Make json for server message
    //  snprintf(serverJSON, SERVER_BUFFER_SIZE, "mode=food mass=", mass);

      //Make command for server message
      //snprintf(commandToRun, 1000, "$(curl -X POST -H '%s' -d '%s' %s) >> imgurResponse", header, imgurJSON, IMGUR_API);
      snprintf(commandToRun, 150, "echo $(http GET %s/api/sendData 'photoid:%s' 'mode:food' 'mass:%f' 'username:%s') >> %s", WEBSITE, "someid", mass, username, SERVER_RESPONSE_FILE_NAME);

      //Send request to server to get imgur authentification key, save response to 'response' file

      //system(commandToRun);

        //"{\"imageName\":\"%s\",\"mode\":\"%s\",\"mass\":\"%f\",\"imageData\":\"data:image/jpeg;base64,%s\"}", newImageName, mode, mass, base64);
    }
    else{

      //Send message to server
      //snprintf(serverJSON, SERVER_BUFFER_SIZE, "{\"mode\":\"barcode\",\"mass\":\"%f\"}", mass, upc);
      snprintf(commandToRun, 150, "echo $(http GET %s/api/sendData 'upc:%s' 'mode:barcode' 'mass:%f' 'username:%s') >> %s", WEBSITE, upc, mass, username, SERVER_RESPONSE_FILE_NAME);


    }



  return;
}



void exit_program(libusb_device_handle* handle, libusb_device **usbDevs, char* upc){
  //if(upc != NULL){free(upc);}
  gpioTerminate();
  end_scale(handle, usbDevs);
  exit(0);
}

/*
int reset_scale(libusb_device* scale, libusb_device_handle* handle, libusb_device **usbDevs){
  end_scale(handle, usbDevs);
  setup_usbscale(&scale, &handle, usbDevs);
  printf("Scale has been reset..\n");
}
*/
