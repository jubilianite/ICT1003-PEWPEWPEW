

/*************************************************************************
                              *Toy Gun*
 *This are the codes that allows user to play around with the amount of 
 *ammos they would like to have*
 *************************************************************************/
#define BLACK           0x00
#define BLUE            0xE0
#define RED             0x03
#define GREEN           0x1C
#define DGREEN          0x0C
#define YELLOW          0x1F
#define WHITE           0xFF
#define ALPHA           0xFE
#define BROWN           0x32


#include "BMA250.h"       // For interfacing with the accel. sensor
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>             // For using I2C communication
#include "Adafruit_DRV2605.h" // For interfacing with the DRV2605 chip
#include <Wireling.h>
#include <STBLE.h>

//Debug output adds extra flash and memory requirements!
#ifndef BLE_DEBUG
#define BLE_DEBUG true
#endif

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0


TinyScreen display = TinyScreen(0);
#define buttonPin A1 // corresponds to Port 0. Similarly, Port 1 = A1, Port 2 = A2, Port 3 = A3.

// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;
int x, y, z;
double temp;
Adafruit_DRV2605 drv;   // The variable used to interface with the DRV2605 chip
uint8_t effect = 1;     // The global variable used to keep track of Waveform effects

// Make Serial Monitor compatible for all TinyCircuits processors
#if defined(ARDUINO_ARCH_AVR)
  #define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
  #define SerialMonitorInterface SerialUSB
#endif

void setup() 
{
  // Initialize and power wireling
  BLEsetup();
  Wireling.begin();
  Wire.begin();                         // Begin I2C Communication
  display.begin();
  display.setFlip(1);
//  SerialMonitorInterface.begin(9600); // Begin Serial Communication and set Baud Rate
SerialMonitorInterface.print("Initializing BMA...");
  // Set up the BMA250 acccelerometer sensor
  accel_sensor.begin(BMA250_range_2g, BMA250_update_time_64ms);

  //The port is the number on the Adapter board where the sensor is attached
  Wireling.selectPort(2);

  drv.begin();

  drv.selectLibrary(1);

  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
  drv.useLRA();

  pinMode(buttonPin, INPUT); // The button is an input 
}

int SECONDS = 9;
char R[1] = {'reload'};
void loopPart1()
{
  accel_sensor.read();//This function gets new data from the acccelerometer

  // Get the acceleration values from the sensor and store them into global variables
  // (Makes reading the rest of the program easier)
  x = accel_sensor.X;
  y = accel_sensor.Y;
  z = accel_sensor.Z;
  temp = ((accel_sensor.rawTemp * 0.5) + 24.0);

  // If the BMA250 is not found, nor connected correctly, these values will be produced
  // by the sensor 
  if (x == -1 && y == -1 && z == -1) 
  {
    // Print error message to Serial Monitor
    SerialMonitorInterface.print("ERROR! NO BMA250 DETECTED!");
  }
}

void loopPart2() 
{
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
}
// Print the DRV effect number and then play the effect
void loop() 
{
  aci_loop();
  display.clearScreen();
  display.setFont(liberationSans_12ptFontInfo);
  display.fontColor(BLACK,BLACK);
  for(int i=0;i<5; i++)
  {
    display.setCursor(0,i*12);
    display.print(" ");
  }
  display.setFont(liberationSans_22ptFontInfo);
  display.setCursor(40,8);
  display.fontColor(GREEN,BLACK);
  if (SECONDS > -1) 
  {
    display.print(SECONDS);
  }

  static int buttonState = 0;
  int prevButtonState = buttonState;
  buttonState = digitalRead(buttonPin);   // Poll Sensor Data
  if (prevButtonState != buttonState) // no need to alert the user if button state remains unchanged
  {
    if (buttonState == LOW)
    {      
     
      SECONDS--;
      if (SECONDS > 0)
      {
      char* signal = "1";
      int signal_length = 0;
       for (int i = 0; signal[i] != '\0'; ++i) 
       {
        ++signal_length;
       }
        lib_aci_send_data((uint8_t)0, (uint8_t*)signal, (uint8_t)signal_length);
      }

                  
      // Play the effect
      drv.go();
      // Set the effect to play
      drv.setWaveform(100000, effect);  // Set effect
      drv.setWaveform(100, 0);       // End waveform
      effect++;
      if (effect > 117) effect = 1;
      if (SECONDS <0)
        {
          SECONDS = NULL ;
                
         }
        if (SECONDS == NULL && buttonState == LOW)
            {
               
              display.setFont(liberationSans_14ptFontInfo);
              
              display.setCursor(13,30);
              display.fontColor(BLUE,BLACK);
              display.print("RELOAD!");
                char* signal = "2";
      int signal_length = 0;
       for (int i = 0; signal[i] != '\0'; ++i) 
       {
        ++signal_length;
       }
        lib_aci_send_data((uint8_t)0, (uint8_t*)signal, (uint8_t)signal_length);
              }  
    
    }
       

    
  }
  else
  {
    if (x > 300 || x < -400 || y > 200 || y < -300 || z > 300 || z < -400 )
    {
      SECONDS = 9;
      char* signal = "3";
      int signal_length = 0;
       for (int i = 0; signal[i] != '\0'; ++i) 
       {
        ++signal_length;
       }
        lib_aci_send_data((uint8_t)0, (uint8_t*)signal, (uint8_t)signal_length);

    }
      // button not currently being pressed
  }
    loopPart1();
    loopPart2();
    
}





  
