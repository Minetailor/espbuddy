#include <SPI.h>
#include <SD.h>
#include "hal/spi_types.h"
#include <Arduino.h>

#ifndef _IO_H
#define _IO_H

#define FSPI_CS_PIN  33
#define FSPI_DC_PIN  35
#define FSPI_RST_PIN 39
#define FSPI_BL_PIN  18
#define FSPI_MOSI 4
#define FSPI_MISO 6
#define FSPI_CLK 2
#define SD_CS 5

#define BUTTON1 40
#define BUTTON2 38
#define BUTTON3 12
#define BUTTON4 34


uint8_t fspi_transfer(uint8_t data);
fs::SDFS acquire_sd();
void release_sd();

#endif