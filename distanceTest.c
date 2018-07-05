#include "distanceSensor.h"
#include "pigpio.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **args){

  int offset;  int reading;
  if( init_gpio() < 0){printf("could not init lib"); exit(EXIT_FAILURE);}

  offset = 0;
  init_buffer(&offset);

  reading = 0;
  get_distance(&reading, offset, 0);

  int i;
  for(i = 0; i < 500; i++){
    get_distance(&reading, offset, 1);
  }

  fprintf(stderr, "ending...\n");

  gpioTerminate();
  return 0;

}
