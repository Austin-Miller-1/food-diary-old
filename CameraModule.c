/*
* Author: Deepankar Malhan
* Date: March 24, 2017
* This module is an API used to interact with the camera module on a Raspberry Pi
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CameraModule.h"

int take_picture(char *picName, CameraFlags cameraFlags) {
  char commandToRun[1024];
  strcpy(commandToRun, "raspistill ");

  if(cameraFlags.hf == 1) {
    strcat(commandToRun, "-hf ");
  }
  if(cameraFlags.vf == 1) {
    strcat(commandToRun, "-vf ");
  }
  //strcat(commandToRun, "--mode 0 "); //Change mode to smaller resolution
  strcat(commandToRun, "-o ");
  strcat(commandToRun, picName);

  int result;
  if((result = system(commandToRun)) == -1){
    perror("Error taking a photo!");
    exit(-1);
  }

  return 0;
}
