#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#include "LCD_Driver.h"
#include "Display.h"
#include "IO.h"


// Sd2Card card;

#define DEBUG

#define SAVE_PERIOD 60000

void setup() {
  //   if (SD.exists("/pixil-frame-0.bmp")) Serial.println("IT EXISTS");
  // else Serial.println("IT DOESN'T");

  Config_Init();
  displayInit();

  // displayBMP("/pixil-frame-0.bmp",0,0);

  displayBMP("/scenery.bmp",0,0);

  // #ifdef DEBUG
  // while (!Serial) {;}
  // #endif


  // displayBMP("/pixil-frame-0.bmp",0,0);

  
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

typedef struct buttons{
  bool button1down;
  bool button2down;
  bool button3down;
  bool button4down;
} Controls;

#define HUNGER_COOLDOWN 1000*60*5 // can feed every 5 minutes
#define HAPPINESS_COOLDOWN 1000*60*2 // can play every 2 minutes

class Buddy {
  private:
    unsigned long hunger_timer;
    unsigned long happiness_timer;
    signed long hunger_cooldown;
    signed long happiness_cooldown;
  public:
    unsigned long age;
    int hunger; // scale of 1 to 10. 1 is hungry, 10 is full
    int happiness;
    char skin;

    Buddy(unsigned long iage, int ihunger, int ihappiness, char iskin) {
      age = iage;
      hunger = ihunger;
      happiness = ihappiness;
      skin = iskin; // future proofing
      hunger_timer = 0; // technically power off will reset this.
      happiness_timer = 0;
      hunger_cooldown = 0; // max time
      happiness_cooldown = 0;
    };

    Buddy(char iskin) {
      age = 0;
      hunger = 0;
      happiness = 0;
      skin = iskin; // future proofing
      hunger_timer = 0; // technically power off will reset this.
      happiness_timer = 0;
      hunger_cooldown = 0;
      happiness_cooldown = 0;
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

    void addHunger(int h) {
      if (hunger_cooldown <= 0) {
        hunger += h;
        if (hunger > 10) hunger = 10;
        hunger_cooldown = HUNGER_COOLDOWN;
      }
      
    }

    void addHappiness(int h) {
      if (happiness_cooldown <= 0) {
        happiness += h;
        if (happiness > 10) happiness = 10;
        happiness_cooldown = HAPPINESS_COOLDOWN;
      }
      
    }

    void update(unsigned long time_diff) {
      age += time_diff;
      hunger_timer += time_diff;
      happiness_timer += time_diff;

      if (hunger_cooldown >= 0) hunger_cooldown -= time_diff;
      if (happiness_cooldown >= 0) happiness_cooldown -= time_diff;

      if (hunger_timer > 60000*60*2) { //2hours
        hunger_timer = 0;
        if (hunger > 0) {
          hunger -= 1;
        }
      }

      if (happiness_timer > 60000*60*3) { //3hours
        happiness_timer = 0;
        if (happiness > 0) {
          happiness -= 1;
        }
      }
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
    Buddy b = Buddy(0,5,5,'0');
    return b;
  }

  release_sd();
  Buddy o = Buddy(0,0,0,'0');
  return o;

  File f = sd.open("/data/buddy.dat");
  f.read(buf.data, sizeof(buf));
  
  release_sd();
  Buddy b = Buddy(buf.unpack.age, buf.unpack.hunger, buf.unpack.happiness, buf.unpack.skin);
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
  NO_UPDATE,
  STARTUP,
  MAIN_MENU
};

class State {
  public:
    virtual void display() {};
    // void handleInputs();
    virtual Menus update(Controls controls) {
      Serial.println("Running Wrong State");
      return NO_UPDATE;
      };

};


class Main_Menu: public State {
  public:
    Buddy* buddy;
    bool new_instance;
    BuddyAge prev_age;
    BuddyEmotion prev_emo;
    Main_Menu(Buddy* b) {
      buddy = b;
      new_instance = true;
    };

    void display() override {
      BuddyAge s = buddy->getState();
      prev_age = s;
      String age;
      switch (s) {
        case EGG:
          age = "/egg";
          break;
        case BABY:
          age = "/baby";
          break;
      }

      BuddyEmotion e = buddy->getEmotion();
      prev_emo = e;
      String emotion;
      switch (e) {
        case HAPPY:
          emotion = "/happy.bmp";
          break;
        case ANGRY:
          emotion = "/angry.bmp";
          break;
        case SAD:
          emotion = "/sad.bmp";
          break;
        case HUNGRY:
          emotion = "/hungry.bmp";
          break;
        default:
          emotion = "/happy.bmp";
      }
      
      String path = String("/skins/");
      path.concat(String(buddy->skin));
      path.concat(age);
      path.concat(emotion);
      

      displayBMP(path.c_str(),0,40);

    };

    Menus update(Controls controls) override {
      if (new_instance) display();
      new_instance = false;
      if (prev_age != buddy->getState()) display(); // change in buddy
      if (prev_emo != buddy->getEmotion()) display(); // change in buddy

      if (controls.button1down) {
        buddy->addHunger(3);
      }

      if (controls.button2down) {
        buddy->addHappiness(3);
      }

      return NO_UPDATE;
    }


};



Controls get_inputs() {
  Controls controls;
  controls.button1down = digitalRead(BUTTON1);
  controls.button2down = digitalRead(BUTTON2);
  controls.button3down = digitalRead(BUTTON3);
  controls.button4down = digitalRead(BUTTON4);

  if (controls.button1down) {
    Serial.println("Button 1 Down");
  } 
  if (controls.button2down) {
    Serial.println("Button 2 Down");
  }
  if (controls.button3down) {
    Serial.println("Button 3 Down");
  } 
  if (controls.button4down) {
    Serial.println("Button 4 Down");
  } 

  return controls;
}




Buddy b; // GLOBAL currently running buddy

State *s;

Menus menu_select = STARTUP;
Menus prev_menu_select = STARTUP;
unsigned long ptime = 0;
unsigned long time_since_save = 0;
void loop() {
  // delay(1000);
  // calculates delta time
  unsigned long ctime = millis();
  unsigned long dtime = ctime - ptime;
  ptime = ctime;

  time_since_save += dtime;

  if (menu_select != STARTUP) {
    b.update(dtime);
    if (time_since_save > SAVE_PERIOD) {
      save_buddy(b);
      time_since_save = 0;
    }
  } else {
    // // Retreives data from SD if it exists
    b = load_buddy();
    // b = Buddy(0,0,0,'0');
    Serial.print("Buddy Skin");
    Serial.println(b.skin);
    // b = new Buddy('0');
    // save_buddy(b);
    menu_select = MAIN_MENU;
    s = new Main_Menu(&b);
  }

  Controls controls = get_inputs();
  s->update(controls);

  // switch (menu_select) {
  //   case STARTUP:
      
  //   case MAIN_MENU:
      

  //     break;
  //   // default:
  //   //   Serial.println("UNKNOWN STATE");
  // }


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
