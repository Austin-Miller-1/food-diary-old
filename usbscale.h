//#include <libusb-1.0/libusb.h>
#ifndef USBSCALE_H
#define USBSCALE_H


#include <stdint.h>
#define NSCALES 9
#define WEIGHT_COUNT 2
#define WEIGH_REPORT_SIZE 0x06
#define GRAMS_PER_ONCE 28.3





int libusb_library_setup(void); //Initializes libusb library for scale. Must be done only once.

//Set up a usbscale. Pass pointer to libusb_device pointer and pointer to libusb_device_handle pointer, whose values (assumingly NULL) will be replaced.
int setup_usbscale(libusb_device **scale, libusb_device_handle **handle, libusb_device **usbDevs);

//Get scale from device list
int get_usb_scale(libusb_device **usbscale, libusb_device **usbDevs);

libusb_device* find_scale(libusb_device**); //Takes libusb device list (array of pointers) and find first usb device matching device listed in scale.h
uint8_t get_first_endpoint_address(libusb_device* dev); // take device and fetch bEndpointAddress for the first endpoint

//Get the mass that is on the scale by providing scale device and scale handle
double get_scale_mass(libusb_device* scale, libusb_device_handle* handle);

//Check data, determine status, etc
double check_data(unsigned char* dat);

//Properly end the program
int end_scale(libusb_device_handle* handle, libusb_device **usbDevs);

//Prints message through perror, then returns -1. Used for convenience
int default_error(char* message);

/*
uint16_t scales[NSCALES][2] = {
    // Stamps.com Model 510 5LB Scale
    {0x1446, 0x6a73},
    // USPS (Elane) PS311 "XM Elane Elane UParcel 30lb"
    {0x7b7c, 0x0100},
    // Stamps.com Stainless Steel 5 lb. Digital Scale
    {0x2474, 0x0550},
    // Stamps.com Stainless Steel 35 lb. Digital Scale
    {0x2474, 0x3550},
    // Mettler Toledo
    {0x0eb8, 0xf000},
    // SANFORD Dymo 10 lb USB Postal Scale
    {0x6096, 0x0158},
    // Fairbanks Scales SCB-R9000
    {0x0b67, 0x555e},
    // Dymo-CoStar Corp. M25 Digital Postal Scale
    {0x0922, 0x8004},
    // DYMO 1772057 Digital Postal Scale
    {0x0922, 0x8003}
};
*/

#endif
