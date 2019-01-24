#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>
#include <stdio.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BRICKS 18
#define RACK_LENGTH 20

typedef struct brick {
  int x_start;
  int x_end;
  int visible;
} brick;

int is_brick(int x, brick bricks[BRICKS])
{
  for (int i=0; i< BRICKS; i++) {
    if ((bricks[i].x_start <= x+2) && (bricks[i].x_end >= x) && (bricks[i].visible == 1)) {
      bricks[i].visible = 0;
      return 1;
    }
  }
  return 0;
}

int all_bricks_gone(brick bricks[BRICKS]) {
  for (int i=0; i<BRICKS; i++) {
    if (bricks[i].visible == 1) {
      return 0;
    }
  }
  return 1;
}

void setup() {
  int x_start = 64;
  int y_start = 32;
  int x_dir = 1;
  int y_dir = 1;
  int analog_val = 0;
  brick rack;
  int points = 0;
  int game_over = 0;
  int x_angle = 2;
  int y_angle = 2;
  brick bricks[BRICKS];
  int gspeed = 1;
  int ee_address = 0;
  int high_score = 0;
  int store_score = 1;

  Serial.begin(9600);

// read high score
  high_score = EEPROM.read(ee_address);

// init the rack
  rack.x_start = 0;
  rack.x_end = RACK_LENGTH;
  rack.visible = 1;

// init bricks
  for (int i=0; i<BRICKS; i++) {
    bricks[i].visible = 1;
  }
  
  {
    int tmp_start = 1;
    int tmp_end = 6;
    for (int i=0; i<BRICKS; i++) {
      bricks[i].x_start = tmp_start;
      bricks[i].x_end = tmp_end;
      tmp_start += 7;
      tmp_end += 7;
    }
  }


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();  

// set display text settings
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

// The main loop
  while (1) {
      if (game_over == 0) {
    display.clearDisplay();
      }
      for (int i=0; i<128; i++){
        display.drawPixel(i,63,BLACK);
      }

// reset bricks if all gone
  if (all_bricks_gone(bricks)) {
    gspeed++;
    for (int i=0; i<BRICKS; i++) {
      bricks[i].visible = 1;
    }
  }

// read joystick
    analog_val = analogRead(A0);
    rack.x_start = map(analog_val, 0, 1023, 128 - RACK_LENGTH, 0);
    rack.x_end = rack.x_start + RACK_LENGTH;

// print rack
    for (int i=rack.x_start; i<rack.x_end; i++) {
      display.drawPixel(i, 63, WHITE);
    }

// print points
    display.setTextSize(1);
    display.setCursor(0, 0);     // Start at top-left corner
    display.print("Score: ");
    display.print(String(points));
    display.print("  High: ");
    display.print(String(high_score));

// print bricks
    for (int i=0; i<BRICKS; i++) {
      if (bricks[i].visible) {
        for (int j=bricks[i].x_start; j<bricks[i].x_end; j++) {
          display.drawPixel(j,10, WHITE);
        }
      }
    }

// print ball
    for (int x=x_start; x<=x_start+2; x++) {
      for (int y=y_start; y<=y_start+2; y++) {
        display.drawPixel(x, y, WHITE);
      }
    }

// write to display
    display.display();

// calculate wall bounce and detect hits
    if (x_start >= 126) {
      x_dir = -1;
    }
    if (x_start <= 0) {
      x_dir = 1;
    }
    if (y_start >= 62) {
      y_dir = -1;
      if ((rack.x_start <= (x_start+5)) && (rack.x_end > x_start-2)) {
        if (((x_start+1) <= (rack.x_start+6)) || ((x_start+1) >= rack.x_end-6)) {
          y_angle = 1*gspeed;
          x_angle = 2*gspeed;
        }
        else {
          y_angle = 2*gspeed;
          x_angle = 1*gspeed;
        }
      }
      else {
        game_over = 1;
      }
    }
    if (y_start <= 10) {
      y_dir = 1;
      if (is_brick(x_start, bricks)) {
         points++;
      }
    }
    if (game_over == 0) {
      x_start += x_dir*x_angle;
      y_start += y_dir*y_angle;
    }
    else {
      // if ball hits ground game is over
      display.setTextSize(2);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(10, 20);     // Start at top-left corner
      display.println("GAME OVER");
      if ((store_score == 1) && (points > high_score)) {
        display.setTextSize(1);
        display.print("      New high!");
      }
      display.display();
      if ((store_score == 1) && (points > high_score)) {
        store_score = 0;
        EEPROM.write(ee_address, points); 
        Serial.println("store in eeprom");
      }
    }
  }
}
