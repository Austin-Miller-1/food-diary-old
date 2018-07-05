/*
* Author: Deepankar Malhan
* Date: March 24, 2017
* Header file defining the custom Camera API defined in CameraModule.c
*/

typedef struct {
  int vf;
  int hf;
} CameraFlags;

int take_picture(char *picName, CameraFlags cameraFlags);
