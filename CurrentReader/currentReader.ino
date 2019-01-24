// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int inputPin = A0;
int inputValue = 0;
int j = 0;
float ah = 0.0;
float current = 0;
unsigned long atime = 0;
unsigned long measOldTime = 0;
unsigned long printOldTime = 0;
float currentArray[10] = {0};
int currentArrayIndex = 0;

byte l1[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
};

byte l2[8] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};

byte l3[8] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
};
byte l4[8] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
};
byte l5[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void setup() {
  Serial.begin(115200);
  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);
  lcd.createChar(0, l1);
  lcd.createChar(1, l2);
  lcd.createChar(2, l3);
  lcd.createChar(3, l4);
  lcd.createChar(4, l5);
  pinMode(inputPin, INPUT);
  current = analogRead(inputPin);
  delay(1000);
}

void loop() {
  // check the status of the switch
  
  atime = millis();
  if ((atime - measOldTime) >= 100) {
    //Serial.println(atime-measOldTime);
    measOldTime = atime;
    currentArray[currentArrayIndex] = analogRead(inputPin);
    
    inputValue = 0;
    for (int i=0; i<10; i++) {
      inputValue += currentArray[i];
    }
    inputValue = inputValue/10;
    current = map(inputValue,0,1023,-2000,2000)/100.0;
    
    ah = ah + current/36.0;
    
    currentArrayIndex++;
    if (currentArrayIndex >= 10) {
      currentArrayIndex = 0;
    }
  }

  atime = millis();
  if ((atime - printOldTime) >= 500) {
    printOldTime = atime;
    
    inputValue = 0;
    for (int i=0; i<10; i++) {
      inputValue += currentArray[i];
    }
    inputValue = inputValue/10;
    current = map(inputValue,0,1023,-2000,2000)/100.0;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    j = map(inputValue,0,1023,0,80);
    for (int i=0;i<=j/5;i++) {
      if (inputValue > 0) {    
        if (i == j/5) {
          lcd.setCursor(i, 0);
          lcd.write(byte(j%5));
        }
        else {
          lcd.setCursor(i, 0);
          lcd.write(byte(4));
        }
      }
      else {
        lcd.setCursor(i,0);
      }
    }
    
    if (current > 0.0) {
      lcd.setCursor(1, 1);  
    }
    else {
      lcd.setCursor(0, 1);
    }
    lcd.print(current);
    lcd.setCursor(7, 1);
    lcd.print(ah);
  }

}
