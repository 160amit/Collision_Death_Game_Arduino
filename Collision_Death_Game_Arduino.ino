#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin #
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


static const unsigned char PROGMEM HERO[] =
{ 0b00000000, 0b00000000,
  0b01111111, 0b11100000,
  0b00111111, 0b01011000,
  0b00011111, 0b01001100,
  0b00111111, 0b11001100,
  0b01111111, 0b00011100,
  0b01111111, 0b11111110,
  0b00011111, 0b11111110,
  0b00011111, 0b01011110,
  0b01111111, 0b01001110,
  0b01111111, 0b11001100,
  0b00111111, 0b00011100,
  0b00011111, 0b11111000,
  0b00111111, 0b11110000,
  0b01111111, 0b10000000,
  0b00000000, 0b00000000
};
static const unsigned char PROGMEM VILLAN[] =
{ 0b00000000, 0b00000000,
  0b00011111, 0b11110000,
  0b00111111, 0b01001000,
  0b01111111, 0b01101100,
  0b01111111, 0b01101110,
  0b01110111, 0b00001111,
  0b01111011, 0b11011111,
  0b01111011, 0b11011111,
  0b01111011, 0b00001111,
  0b01110111, 0b01101110,
  0b01111111, 0b01101100,
  0b01111111, 0b00001100,
  0b01111111, 0b11001000,
  0b00111111, 0b11110000,
  0b00011111, 0b11100000,
  0b00000000, 0b00000000
};
#define DEBOUNCE_CONSTANT 150
volatile static byte count = 0;
volatile static boolean stateOfVillan[4] = {0, 0, 0, 0};
volatile static boolean stateOfHero[3] = {0, 1, 0};
volatile static unsigned long millisStartRight = millis();
volatile static unsigned long microStartLeft = millis();
volatile static byte xStart = 0;
volatile static byte level = 3;
volatile static uint16_t points = 0;
volatile static uint16_t high_score = 0;

void setup() {
  pinMode(2, INPUT);//push button to pin D2 with pull down resister
  pinMode(3, INPUT);//push button to pin D3 with pull down resister
  attachInterrupt(0, shiftHeroLeft, RISING);
  attachInterrupt(1, shiftHeroRight, RISING);

  //Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // Serial.println(F("SSD1306 allocation failed"));
  }
}

void loop() {
  display.clearDisplay();
  displayBackground();
  villanFrame();
  plotHero();
  display.display();//this cause almost 35 milli second delay
  if (checkCollision()) {
    preResetActivityOnGameOver();
  }
}
//*******************************************************************************************************************

void displayHero(byte locationX, byte locationY) {
  display.drawBitmap(locationX, locationY, HERO, 16, 16, 1);
  //display.drawRect( locationX , locationY , 16 , 16 , SSD1306_WHITE);
}

void plotHero() {
  byte xStart = 106;//HERO x-position
  if (stateOfHero[0]) {
    displayHero(xStart, 3);
  } else if (stateOfHero[1]) {
    displayHero(xStart, 23);
  } else if (stateOfHero[2]) {
    displayHero(xStart, 44);
  }

}

//shift hero to left
void shiftHeroLeft() {
  volatile unsigned long var1 = millis();
  volatile unsigned long var2 = var1 - microStartLeft;
  if (var2 > DEBOUNCE_CONSTANT) {
    microStartLeft = millis();

    if (!stateOfHero[0]) {
      if (!stateOfHero[1]) {
        stateOfHero[1] = 1;
        stateOfHero[0] = 0;
        stateOfHero[2] = 0;
      } else {
        stateOfHero[0] = 1;
        stateOfHero[1] = 0;
        stateOfHero[2] = 0;
      }
    }
  }
}

//shift hero to right
void shiftHeroRight() {
  volatile unsigned long var1 = millis();
  volatile unsigned long var2 = var1 - millisStartRight;
  if (var2 > DEBOUNCE_CONSTANT) {
    millisStartRight = millis();
    if (!stateOfHero[2]) {
      if (!stateOfHero[1]) {
        stateOfHero[1] = 1;
        stateOfHero[0] = 0;
        stateOfHero[2] = 0;
      } else {
        stateOfHero[2] = 1;
        stateOfHero[1] = 0;
        stateOfHero[0] = 0;

      }
    }
  }
}
//*******************************************************************************************************************

void displayVillan(byte locationX, byte locationY) {
  display.drawBitmap(locationX, locationY, VILLAN, 16, 16, 1);
  //display.drawCircle( locationX , locationY , 8 , SSD1306_WHITE);
}

void plotLineOne() {
  displayVillan(xStart, 4);
}

void plotLineTwo() {
  displayVillan(xStart, 23);
}

void plotLineThree() {
  displayVillan(xStart, 44);
}

void villanFrame() {
  if (!stateOfVillan[3]) {
    stateOfVillan[3] = 1;
  }
  changeStateOfVillanAtRow();
}

void changeStateOfVillanAtRow() {

  if (stateOfVillan[3]) {
    xStart += level;
    if (!(stateOfVillan[0] || stateOfVillan[1] || stateOfVillan[2])) {
      while (!(stateOfVillan[0] || stateOfVillan[1]))
      {
        stateOfVillan[0] = randomState();
        stateOfVillan[1] = randomState();
      }
      if (stateOfVillan[0] && stateOfVillan[1]) {
        stateOfVillan[2] = 0;
      } else {
        stateOfVillan[2] = 1;
      }
    }
    if (xStart >= 142) {
      if (count == 10) {
        levelUp();
        count = 0;
      } else {
        count++;
      }
      points++;
      resetVillanArray();
    }
    plotLine();
  }
}
void resetVillanArray() {
  for (byte i = 0; i < 4; i++) {
    stateOfVillan[i] = 0;
  } xStart = 0;
}
//*******************************************************************************************************************

void displayBackground() {
  display.drawRect( 0 , 0 , 128 , 64 , SSD1306_WHITE);
  display.drawRect( 1 , 1 , 126 , 62 , SSD1306_WHITE);
  display.drawLine( 0 , 21, 104 , 21 , SSD1306_WHITE);
  display.drawLine( 0 , 41, 104 , 41 , SSD1306_WHITE);
}

void displayScore() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 10);
  if (high_score < points) {
    high_score = points;
  }
  String string = "HS=" + String(high_score) + "\n  " + "YS=" + String(points);
  display.print(string);
}

void levelUp() {
  level++;
}

void displayGameOver() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 20);
  display.print(F("GAME OVER"));
}

boolean randomState() {
  byte randomState = random(0, 15);
  if (randomState < 6) {
    return 0;
  } else {
    return 1;
  }
}

void plotLine() {
  if (stateOfVillan[3]) {
    if (stateOfVillan[0]) {
      plotLineOne();
    }
    if (stateOfVillan[1]) {
      plotLineTwo();
    }
    if (stateOfVillan[2]) {
      plotLineThree();
    }
  }
}

boolean checkCollision() {
  if (stateOfHero[0] && stateOfVillan[0]) {
    return  xStart > 90 && xStart < 106;
  } else if (stateOfHero[1] && stateOfVillan[1]) {
    return  xStart > 90 && xStart < 106;
  } else if (stateOfHero[2] && stateOfVillan[2]) {
    return  xStart > 90 && xStart < 106;
  } else return 0;
}
void preResetActivityOnGameOver() {
  display.clearDisplay();
  display.display();
  displayGameOver();
  display.display();
  delay(400);
  display.clearDisplay();
  display.display();
  displayScore();
  display.display();
  delay(1500);

  resetVillanArray();
  count = 0;
  points = 0;
  level = 3;

  loop();
}
