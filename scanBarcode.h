
#define SCANDIT_SDK_APP_KEY "AVe5FwAVGhZxQJEE+QRFV0EBEY6rBvsRMm8fN8d37icseDk6O3BLQYgsK8ccdZtdbGc5X9lpEzeXXbz9gEUPD+xL0exdVvCLcADk3Rpq44LJBowdz0TLteu9X+e6NYkTJogyKo9Kha+g/CLHfDDKOfAtzg7pbxb/dWSnoxOOH4te7msmfSxyfbqP1jAAonQISIzRbEb1aR7S/+5MkmMlye+ezpAFARckJ/RstTVy9m/SXvrVJ9Mn4uXyDIJ88twkXwuDyYbvyLa1NEBeV44xntGv4Dx/WzWfXFEBYfj0yLM3nCp3Nu0aJzFkZuEBiptUEJHYYoQWYOFdmMUSiXX4Sx0En5b1pIpDyn9VTcxVpmEdXu6K4T+xdE5i5UMzWpkUWK0CZFkVD8PkYrw+/OX1ktByBDW8ZPC8eroFp8oLmctaU/N6hlaLLkNA0r7jQC+jBCn5TDm043Qllfu7c+RViQiSJbeAwf7SskIn3/cy0dhITn2ZwBf70RJ9bbzLUpqYTdUPuRvvRAaPm/gt7HRNDdenynvLfMJf6g0EBxXtOzPpml6Fha6ch9D6BiMT3a0WvGcNbnqH+dRCpzezeaK7NcOWZOovBaLnJNBuI9e38XJfINqEPwxTA3Q0N//n5UNlILbqWpriaQoeJVVXVZ3msm+myrQ087tzkaiExdP7r5HvqIeV8AQ63dJq9JRScKYVHJEJ0kA+vUmSzanFbskgaphPJX1HJWl5YQMm7c3crYy+bbrogYzP3TrSkWlHwOhLeDRQTyHPwe7DATKK9FaKW+UoNx+QzA=="

//initialize scandit usage (send in pointer pointers to scandit structs)
int init_scandit(ScRecognitionContext **contextR, ScBarcodeScanner **scannerR, ScImageDescription **imageDescrR, ScBarcodeScannerSettings **settingsR);

//Open barcode given pathname & upc buffer
int get_upc(char* image_name, char* upcBuffer, int bufferSize, ScRecognitionContext *context, ScBarcodeScanner *scanner, ScImageDescription *imageDescr);

//Open image given pathname & buffer
int open_img(char* image_name, uint8_t** data, uint64_t* width, uint64_t* height);

//End library & struct usage
int end_scandit(ScBarcodeScanner *scanner, ScBarcodeScannerSettings *settings, ScRecognitionContext *context, ScImageDescription *imageDescr);
