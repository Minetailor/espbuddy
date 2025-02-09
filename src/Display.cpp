#include "Display.h"

#define CONVERT_32BIT(buf, offset) (uint32_t)buf[offset+3]<<24|(uint32_t)buf[offset+2]<<16|(uint32_t)buf[offset+1]<<8|(uint32_t)buf[offset]
#define CONVERT_16BIT(buf, offset) (uint16_t)buf[offset+1]<<8|(uint16_t)buf[offset]

void displayBMP(const char *path, uint16_t x, uint16_t y) {
    //requires SD 
    fs::SDFS sd = acquire_sd();
    
    if (!sd.exists(path)) {
        Serial.print(path);
        Serial.println(" ::Image does not exist");
        release_sd();
        return;
    }
    
    File f = sd.open(path);
    uint8_t buf[DISP_BUFFER]; // this can probably be changed
    uint8_t header[14];
  
    // read header
    f.read(header, 14);
  
    uint32_t size = CONVERT_32BIT(header,2); //(uint32_t)header[5]<<24|(uint32_t)header[4]<<16|(uint32_t)header[3]<<8|(uint32_t)header[2];
    uint32_t offset = CONVERT_32BIT(header, 10);
    
    // for (int i = 0; i < 14; i++) {
    //   Serial.println(header[i]);
    // }
  
    Serial.print("Start Address: ");
    Serial.println(offset);
    Serial.print("Size of BMP: ");
    Serial.println(size);
  
    f.read(buf, 4);
    uint32_t dibsize = CONVERT_32BIT(buf, 0);
    f.read(buf, dibsize-4);
    uint32_t width = CONVERT_32BIT(buf,0);
    uint32_t height = CONVERT_32BIT(buf,4);
    uint16_t depth = CONVERT_16BIT(buf, 10);
    uint32_t compression = CONVERT_32BIT(buf, 12);
    uint32_t raw_size = CONVERT_32BIT(buf, 16);
    uint32_t col_size = CONVERT_32BIT(buf, 28);
  
    uint32_t red_mask = CONVERT_32BIT(buf, 36);
    uint32_t green_mask = CONVERT_32BIT(buf, 40);
    uint32_t blue_mask = CONVERT_32BIT(buf, 44);
    uint32_t alpha_mask = CONVERT_32BIT(buf, 48);
  
    Serial.println(width);
    Serial.println(height);
    Serial.println(depth);
    Serial.println(compression);
    Serial.println(col_size);
  
    //start drawing
    LCD_SetCursor(x, y, x+width,y+height);
    // convert pixel data
    while (f.available()) {
      size_t num_bytes = f.read(buf, 4); // size dependent on bits per pixel
      uint32_t full_col = CONVERT_32BIT(buf,0);
      uint32_t red = ((full_col & red_mask)>>16)>>3;//5
      uint32_t green = ((full_col & green_mask)>>8)>>2;//6
      uint32_t blue = (full_col & blue_mask)>>3;//5
      uint16_t colour = red<<11 | green << 5 | blue;
  
      LCD_WriteData_Word(colour);
      
    }

    release_sd();
}

void displayInit() {
    LCD_Init();
    LCD_Clear(0xffff);
}