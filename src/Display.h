#include "LCD_Driver.h"
#include <SD.h>
#include "IO.h"


#define DISP_BUFFER 200

void displayBMP(const char *path, uint16_t x, uint16_t y);
void displayInit();