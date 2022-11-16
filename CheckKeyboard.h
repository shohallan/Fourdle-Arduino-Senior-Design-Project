#ifndef CheckKeyboard_h
#define CheckKeyboard_h

// Used for I2C protocol with keyboard
#include <Wire.h>

char checkKeyboard(uint8_t Address){
  Wire.requestFrom(Address, (size_t)1); // Request an input from the keyboard
  while (Wire.available()) {
    char key = Wire.read();
    if (key != 0) { // If a key on the keyboard is pressed...
    // And the key is an uppercase alphabetical...
      if(key >= 'A' && key <= 'Z')
        return(key); // return normally
        
    // And the key is a lowercase alphabetical...
      else if(key >= 'a' && key <= 'z')
        return(toupper(key)); // return the uppercase version
    
    // And the key is a backspace, comma, period, space, or enter...
      else if(key == 0x08 || key == ',' || key == '.' || key == ' ' || key == 0x0D)
        return(key); // return normally
    
    // Otherwise return null
      else return(NULL);
    }
    else return(NULL); // Return null if a key isn't pressed
  }
}

#endif