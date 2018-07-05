/*
* Author: Deepankar Malhan
* Date: March 24, 2017
* Tests the camera module
*/

#include "CameraModule.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Please enter a name for the photo!\n");
    exit(1);
  }

  CameraFlags cameraFlags;
  cameraFlags.hf = 1;
  cameraFlags.vf = 1;

  int test;
  if((test = take_picture(argv[1], cameraFlags)) == 0) {
    printf("Picture successfully taken!");
  }
  else {
    perror("Photo not taken!");
    exit(1);
  }

  return 0;
}
