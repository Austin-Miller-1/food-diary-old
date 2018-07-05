/**
 * \file ScanditNativeBarcodeScannerDemo.c
 *
 * \brief ScanditSDK demo application
 *
 * Takes a list of input images and directories (containing images) as argument.
 * The resulting input images are processed in reverse order by the barcode
 * scanner.
 *
 * \date 02.06.2016
 *
 * \copyright Copyright (c) 2016 Scandit AG. All rights reserved.
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <MagickWand/MagickWand.h> //#include <wand/magick-wand.h>

#include <Scandit/ScRecognitionContext.h>
#include <Scandit/ScBarcodeScanner.h>

// Please insert your app key here:
#define SCANDIT_SDK_APP_KEY "ASIJqRYVDCpJGQn1LArwXU1DDRo3BUe3EE8952FB6DcdVmh+lnt2fE4+nTr9cgZLPC5A4EZU6mT7byBK3UqkvnJlRLjCJ0XM9Ry6aCVCcLhIX1L/RkV/++hXLaa/dbW1ilMNyjtP2uVic994M07aqHRfUJBzWjbu5CwM5AFA8mnsYO9+5WDmfRdstUpSdTCyQGhH0LNSfM5dTZzxnUDIwCZmc0LzWF9Cu3yZk2tlfAkaK0UrWF77IR1n+WVveOVYaWo7Xm1jXYwmeX6GNnfi9JIuhXmhRyPfgGL8PMpdYJgcIcyPtJDpK3qfl1MeFRqfQ9vXMFfNrE3JSTPYvpQc3R26+0D8SbQDkQxk+4ro/Qw8KWeH+DrXubgWF5hvwO8fonrAQvSuulUov7TK+IPDG8F4Q5ce1cRHAi7Z+fNyHlvO8FuofJTyiq3PXkwH6s97Letmx6SPJgyaEyezpjyp/aJEX6ZWYf6JRYrG1h3fe2sMuJfScQCg/GTPmCDA+ppSd0wKGVIpVPKY/YNoqoNdM9AIbfe38aK8WOCVSC9oUaVSAAAGoKjK9ZFzpyaotEZIPoY56NyexI5TOME1AnE78TQrkYTXOs12DrVCTGVTgUZxrOC5Wq/76jyoMh7M1WGWImN0ERdDGsXHiVU4TEAfmsdQS+RJGi8s9Gaaz7FAW89V1TOnZZjd2jctGBxZ+/cWHo5bBTrsx187eS7wqBbhgb/q1A7J74oFlbVTFhMNwxzY2WV/g2eUSUrX7hoNZSCbO5wH7i67tjEPEr7k+qVZDn+XGZwyVAqGdoLIYPPnaMBKJ7GCOljkUQfuQb4agskF33k5zsSSUaslZx1sGuHv9CZQkYswBlUvr5dH09GWXbFwARq2A72p9M0ClcCuDJlz3vsXfVj9oEwtJ4UV4ReVqdYHZCxaD/vz4b16l3dpI/5/GtDS3Q8A54bbHkO+cSYL+tprYWa+SC3nnkg3MnU0Fd7J0uQ="

int open_img(char* image_name, uint8_t** data, uint64_t* width, uint64_t* height);
int end_program(ScBarcodeScanner *scanner, ScBarcodeScannerSettings *settings, ScRecognitionContext *context, ScImageDescription *imageDescr, uint8_t *imageData);

int main(int argc, char** args){

  //Set up (check args, define vars)
  if(argc!=2){
    perror("usage: 'barcodeScanner.exe imageName'");
    exit(EXIT_FAILURE);
  }

  //static char* id = "com.fooddiary";
  //printf("Here?");
  printf("Scandit SDK Version: %s\n", SC_VERSION_STRING);

  //printf("MAde it here");
  //int returnCode = 0;
  ScRecognitionContext *context = NULL;     //Manages scanner objects & scheduling of the recognition process
  ScBarcodeScanner *scanner = NULL;         //Scans & decodes barcodes in images
  ScImageDescription *imageDescr = NULL;    //Describes dimension and internal memory layout of an image buffer
  ScBarcodeScannerSettings *settings = NULL;//Holds configuration options for barcode scanner
  uint8_t *imageData = NULL;  //The data of the image itself as an array of 8-bit unsigned ints
  uint64_t imageWidth = -1; //Width of img
  uint64_t imageHeight = -1; //Height of img
  char* imageName = args[1]; //Name of img

  //Create recognition context.
  //Writes some file to /tmp
  //printf("Making context..");
  context = sc_recognition_context_new(SCANDIT_SDK_APP_KEY, "/tmp", NULL);
  if(context == NULL){
    perror("Could not init context.\n");
    end_program(scanner, settings, context, imageDescr, imageData);
    exit(EXIT_FAILURE);
  }
  //printf("Context made");
  //Create image description
  imageDescr = sc_image_description_new();
  if(imageDescr == NULL){
    perror("Could not init image description.\n");
    end_program(scanner, settings, context, imageDescr, imageData);
    exit(EXIT_FAILURE);
  }
  //printf("image description made");
  //Create  barcode scanner settings, which will be passed to barcode scanner
  settings = sc_barcode_scanner_settings_new();
  if(settings == NULL){
    perror("Could not init settings.\n");
    end_program(scanner, settings, context, imageDescr, imageData);
    exit(EXIT_FAILURE);
  }

  //Set settings to allow UPC-A, UPC-E and EAN-13 barcodes
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_EAN13, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_UPCA, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_UPCE, SC_TRUE);

  //Set settings such that barcode does not need to be horizontal or centered
  // By setting the code location constraints to hint and code direction to none, we tell
  // the engine to run the full-image localization for every frame.
  sc_barcode_scanner_settings_set_code_location_constraint_1d(settings, SC_CODE_LOCATION_HINT);
  sc_barcode_scanner_settings_set_code_location_constraint_2d(settings, SC_CODE_LOCATION_HINT);
  sc_barcode_scanner_settings_set_code_direction_hint(settings, SC_CODE_DIRECTION_NONE);
  //const ScBool use_full_image_localization = SC_TRUE;

  //Create barcode scanner
  scanner = sc_barcode_scanner_new_with_settings(context, settings);
  if(scanner == NULL){printf("Could not init scanner. \n");
    end_program(scanner, settings, context, imageDescr, imageData);
    exit(EXIT_FAILURE);
  }

  //Allow Scanner to initialize
  if (!sc_barcode_scanner_wait_for_setup_completed(scanner)) {printf("Barcode scanner setup failed.\n"); end_program(scanner, settings, context, imageDescr, imageData); exit(EXIT_FAILURE);}
  //printf("Scanner initialized");
  //Get image's dimensions, data, info
  if( open_img(imageName, &imageData, &imageWidth, &imageHeight) ==  SC_FALSE){ printf("Failed to load image '%s'.\n", imageName); }
  //printf("Image opended");
  //Set image decription fields
  const uint32_t imageMemorySize = imageWidth * imageHeight * 3; //Memory is 3 bytes per pizel (RGB)
  sc_image_description_set_layout(imageDescr, SC_IMAGE_LAYOUT_RGB_8U);
  sc_image_description_set_width(imageDescr, imageWidth);
  sc_image_description_set_height(imageDescr, imageHeight);
  sc_image_description_set_memory_size(imageDescr, imageMemorySize);

  //Signal to the context that a new sequence of frames starts. This call is mandatory,
  // even if we are only going to process one image. Scanning will fail with
  // SC_RECOGNITION_CONTEXT_STATUS_FRAME_SEQUENCE_NOT_STARTED otherwise.
  sc_recognition_context_start_new_frame_sequence(context);

  //Process image for barcodes
  printf("Got to prescanning\n");
  ScProcessFrameResult result = sc_recognition_context_process_frame(context, imageDescr, imageData);
  if (result.status != SC_RECOGNITION_CONTEXT_STATUS_SUCCESS) {
    printf("Processing frame failed with error %d: '%s'\n", result.status,
    sc_context_status_flag_get_message(result.status));
    end_program(scanner, settings, context, imageDescr, imageData);
    exit(EXIT_FAILURE);
  }
  printf("Got to postscanning\n");
  //Signal that frame sequence is finished
  sc_recognition_context_end_frame_sequence(context);

  //Get barcode scaner object to get list of codes recognized in frame
  ScBarcodeScannerSession *session = sc_barcode_scanner_get_session(scanner);

  //Get codes
  ScBarcodeArray* codes = sc_barcode_scanner_session_get_newly_recognized_codes(session);
  uint32_t numOfCodes = sc_barcode_array_get_size(codes);
  if(numOfCodes == 0){ printf("no barcodes found\n");}
  else{ printf("%d codes found", numOfCodes); }

  //Get data associated with barcode
  const ScBarcode* barcode = sc_barcode_array_get_item_at(codes, 0); //get barcode
  ScByteArray data = sc_barcode_get_data(barcode); //get data
  ScSymbology symbology = sc_barcode_get_symbology(barcode); //get symbology
  const char *symbology_name = sc_symbology_to_string(symbology);

  //Print results of search
  printf("barcode: symbology=%s, data='%s'\n", symbology_name, data.str);

  //End program
  sc_barcode_array_release(codes);
  free(imageData);
  imageData = NULL;
  end_program(scanner, settings, context, imageDescr, imageData);
}

int end_program(ScBarcodeScanner *scanner, ScBarcodeScannerSettings *settings, ScRecognitionContext *context, ScImageDescription *imageDescr, uint8_t *imageData){
  sc_barcode_scanner_release(scanner);
  sc_barcode_scanner_settings_release(settings);
  sc_recognition_context_release(context);
  sc_image_description_release(imageDescr);

  free(imageData);
}

//Open image using MagickWand library
ScBool open_img(char* image_name, uint8_t** data, uint64_t* width, uint64_t* height){
  //Start process
  MagickWandGenesis();  //initialize MagickWand env.
  MagickWand *image = NewMagickWand();  //our "image", works with other MagickWand functions

  //Read image, if failure, return false.
  if(MagickReadImage(image, image_name) == MagickFalse){ DestroyMagickWand(image); MagickWandTerminus(); return SC_FALSE;}

  //Convert image to RGB ("so the pixel data can be accessed directly")
  if(!MagickSetImageFormat(image, "RGB")){perror("Could not format image");}
  if(!MagickSetImageColorspace(image, RGBColorspace)){perror("Could not set colorspace");}
  if(!MagickSetImageType(image, TrueColorType)){perror("Could not set image type");}
  if(!MagickSetImageDepth(image, 8)){perror("Could not set image depth");}

  //Get width & height of image
  *width = MagickGetImageWidth(image);
  *height = MagickGetImageHeight(image);

  //Get image data
  size_t blob_size = 0;
  uint8_t* imageData = MagickGetImageBlob(image, &blob_size);

  printf("image size: %d x %d (%d bytes)\n", (int)*width, (int)*height, (int)blob_size);
  *data = malloc(blob_size);
  memcpy(*data, imageData, blob_size);

  //End Process
  MagickRelinquishMemory(imageData);
  DestroyMagickWand(image); //Free image from memory
  MagickWandTerminus();     //Removes MagickWand env.
  return SC_TRUE;
}
