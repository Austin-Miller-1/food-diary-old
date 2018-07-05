
#include <Scandit/ScRecognitionContext.h>
#include <Scandit/ScBarcodeScanner.h>
#include "scanBarcode.h"
#include <stdio.h>
#include <stdlib.h>
#define BUFFER 30

int main(int ac, char** av){
  ScRecognitionContext *context = NULL;     //Manages scanner objects & scheduling of the recognition process
  ScBarcodeScanner *scanner = NULL;         //Scans & decodes barcodes in images
  ScImageDescription *imageDescr = NULL;    //Describes dimension and internal memory layout of an image buffer
  ScBarcodeScannerSettings *settings = NULL;//Holds configuration options for barcode scanner

  char upcBuffer[BUFFER];

  if( init_scandit(&context, &scanner, &imageDescr, &settings) < 0){ end_scandit(scanner, settings, context, imageDescr);}
  if( get_upc("dsmall.jpg", upcBuffer, BUFFER, context, scanner, imageDescr) < 0){ end_scandit(scanner, settings, context, imageDescr);}
  printf("\nUPC: %s\n", upcBuffer);
  end_scandit(scanner, settings, context, imageDescr);
}
