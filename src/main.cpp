#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#include "LCD_Driver.h"
#include "Display.h"
#include "IO.h"


// Sd2Card card;

#define DEBUG

void setup() {
  //   if (SD.exists("/pixil-frame-0.bmp")) Serial.println("IT EXISTS");
  // else Serial.println("IT DOESN'T");

  Config_Init();
  displayInit();

  #ifdef DEBUG
  while (!Serial) {;}
  #endif


  displayBMP("/pixil-frame-0.bmp",0,0);
  // SPI.transfer(0x36);

  // LCD_SetCursor(120,120,121,120);
  // LCD_WriteData_Word(0xffff);
  // LCD_Clear(0xffff);
  // LCD_Clear(0xff);
  // LCD_SetBacklight(50);

  // Serial.begin(9600);

}
// millis wraps after 49 days
enum BuddyAge {
  EGG,
  BABY,
  TEEN,
  ADULT,
  OLD,
  DEAD
};

enum BuddyEmotion {
  HAPPY,
  SAD,
  ANGRY,
  HUNGRY
};

class Buddy {
  public:
    unsigned long age;
    int hunger; // scale of 1 to 10
    int happiness;
    char skin;

    Buddy(unsigned long iage, int ihunger, int ihappiness, char iskin) {
      age = iage;
      hunger = ihunger;
      happiness = ihappiness;
      skin = iskin; // future proofing
    };

    Buddy(char iskin) {
      Buddy(0,0,0,iskin);
    };

    Buddy() {
      Buddy('0');
    };

    BuddyAge getState() {
      int days = age / 86400000;
      if (age < 30000) { //30secs
        return EGG;
      } else if (days < 2) return BABY;
      else if (days < 10) return TEEN;
      else if (days < 20) return ADULT;
      else if (days < 25) return OLD;
      else return DEAD;
    }

    BuddyEmotion getEmotion() {
      if (happiness >= 5 && hunger >= 5) return HAPPY;
      else if (happiness >= 5) return HUNGRY;
      else if (hunger >= 5) return SAD;
      else return ANGRY;
    }
};

union BuddyStats {
  uint8_t data[16];
  struct {
    unsigned long age;
    int hunger; // scale of 1 to 10
    int happiness;
    char skin;
  } unpack;
};

Buddy load_buddy(){
  fs::SDFS sd = acquire_sd();
  BuddyStats buf;
  // check if file exists

  if (!sd.exists("/data/buddy.dat")) {
    Serial.println("Buddy.dat not found");
    // File f = sd.open("/data/buddy.dat", FILE_WRITE);
    // buf[8] = '0';
    // buf
    // f.write();
    release_sd();
    Buddy b('0');
    return b;
  }

  File f = sd.open("/data/buddy.dat");
  f.read(buf.data, sizeof(buf));
  
  release_sd();
  Buddy b(buf.unpack.age, buf.unpack.hunger, buf.unpack.happiness, buf.unpack.skin);
  return b;

  // File f = sd.open("/data/skin_list.dat");
}

void save_buddy(Buddy bud) {
  fs::SDFS sd = acquire_sd();
  BuddyStats buf;

  File f = sd.open("/data/buddy.dat", FILE_WRITE);
  f.seek(0);
  buf.unpack.age = bud.age;
  buf.unpack.happiness = bud.happiness;
  buf.unpack.hunger = bud.hunger;
  buf.unpack.skin = bud.skin;
  f.write(buf.data, sizeof(buf));
  f.close();
  release_sd();

}

enum Menus {
  STARTUP,
  MAIN_MENU
};

Buddy b; // GLOBAL currently running buddy

Menus menu_select = STARTUP;
void loop() {
  delay(1000);
  

  switch (menu_select) {
    case STARTUP:
      // // Retreives data from SD if it exists
      b = load_buddy();
      // save_buddy(b);
      menu_select = MAIN_MENU;
      break;
    case MAIN_MENU:
      BuddyAge s = b.getState();
      String img;
      switch (s) {
        case EGG:
          img = "/egg.bmp";
          break;
      }
      
      String path = "/skins/";
      path.concat(b.skin);
      path.concat(img);

      displayBMP(path.c_str(),0,40);

      break;
    // default:
    //   Serial.println("UNKNOWN STATE");
  }


    // Serial.print(digitalRead(BUTTON1));
    // Serial.print(digitalRead(BUTTON2));
    // Serial.print(digitalRead(BUTTON3));
    // Serial.println(digitalRead(BUTTON4));

  // if (card.init(SPI_HALF_SPEED, 5)) {Serial.println("Card is present");}

  // // put your main code here, to run repeatedly:
  // Serial.println("Hi");
  // Serial.print("MOSI: "); //11
  // Serial.println(MOSI);
  // Serial.print("MISO: "); // 9
  // Serial.println(MISO);
  // Serial.print("SCK: "); // 7
  // Serial.println(SCK);
  // // Serial.println(SPI.getClockDivider());
  // Serial.print("SS: ");
  // Serial.println(SS);
  // // SD.open()
  

  

  // Serial.print("Size: ");
  // Serial.println(SD.cardSize());
  // colour = colour << 1;
  // // if (colour == 0) colour = 3;
  // // LCD_Clear(colour);
  // // SD.end();
  // delay(1000);
  



}
