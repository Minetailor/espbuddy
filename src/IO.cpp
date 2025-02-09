/*

IO

Used to define SPI and GPIO.

*/
// #include <stdint.h>

#include <esp32-hal-spi.h>
#include "IO.h"
SPIClass *fspi;

bool sd_in_use = false;

uint8_t fspi_transfer(uint8_t data) {
  return fspi->transfer(data);
}

fs::SDFS acquire_sd() {
  while (!SD.begin(SD_CS) || sd_in_use) Serial.println("SD Does not exist");
  sd_in_use = true;
  return SD;
}

void release_sd() {
  SD.end();
  sd_in_use = false;
}




void GPIO_Init()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS,1);
  pinMode(FSPI_CS_PIN, OUTPUT);
  digitalWrite(FSPI_CS_PIN,1);
  pinMode(FSPI_RST_PIN, OUTPUT);
  pinMode(FSPI_DC_PIN, OUTPUT);
  pinMode(FSPI_BL_PIN, OUTPUT);

  pinMode(BUTTON1, INPUT_PULLDOWN);
  pinMode(BUTTON2, INPUT_PULLDOWN);
  pinMode(BUTTON3, INPUT_PULLDOWN);
  pinMode(BUTTON4, INPUT_PULLDOWN);
 }

void Config_Init()
{

  GPIO_Init();
  
  //Serial
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  //FSPI
  fspi = new SPIClass(HSPI); // HSPI and FSPI work
  fspi->begin(FSPI_CLK,-1,FSPI_MOSI, -1); // pin 14 cus this might be interferring
  fspi->beginTransaction(SPISettings(3000000 , MSBFIRST, SPI_MODE3));
  }
