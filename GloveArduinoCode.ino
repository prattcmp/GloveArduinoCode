#include <stdarg.h>
/*
  Listener is part of the VCoS NSF project, and is intended to listen for incoming serial bytes. 
 These bytes indicate which motor(s) should vibrate to provide haptic feedback to the user.
 The current interface is as follows:
 [Orientation (0..100 percent of rotation Theta. Motor 0 is 0 degrees. 8 bit unsigned int.)]
 [Intensity (0..100 percent). 8 bit unsigned int.]
 [Duration (1..100 percent of a second) 8-bit unsigned int.]
 These values are sent in binary packets using unsigned integer values, as follows:
 [0xFF][orientation][intensity][duration][checksum - never 255. sender will truncate to 254 rather than send a 255]
 If a new packet is received, it will overwrite the duration of the previous one, terminating it immediately.

 Authors: Greg Link
          Gus Smith <hfs5022@psu.edu>
 */

// Pin 13 has an LED connected
int ledpin = 13;
// MOT0 - 3 MOT1 - 8 MOT2 - 12 MOT3 - 23
int motorpins[6] = {
  3,8,12,23,19,14}; // motor driver pins 0 through 5, ideally spaced 60 degrees apart
char print_buffer[64];
float distance_threshold = 40;

// the setup routine runs once when you press reset:
void setup() {
                   
  Serial.begin(9600); // Serial USB is always 12MBit/sec
  Serial.setTimeout(250);
  pinMode(ledpin, OUTPUT);     
  for(int i = 0; i < 6; i++){
    pinMode(motorpins[i],OUTPUT);
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
    delay(100);               // wait for a second
  }
  digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for a second
  digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
  delay(500);  // wait for a second

  // TEST used to determine which motors are which
//  while(1){
//    for (int i = 0; i < 4; i++){
//      digitalWrite(motorpins[i],HIGH);
//      delay(250);
//      digitalWrite(motorpins[i],LOW);
//    }
//  }
  
}

void pulseLED(int durationms){
  digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(durationms);               // wait for a second
  digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
  delay(durationms);               // wait for a second
}

void p(char *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

uint8_t theta = 0;
uint8_t intensity = 0;
int8_t duration = 0;

void check_bytes(void)
{
  static int8_t received_packet [5] = {
    0,0,0          }; //array for the getc
  static int8_t bytes_seen = 0;
  static bool seenFF = false;
  int8_t num_characters = 5;
  
  // While there's bits to be read...
  while(Serial.available() > 0){  
    
    uint8_t byte = Serial.read();
    //p("R:[%d]@%d\n",byte, bytes_seen);
    if(byte == 0xFF) { // preamble byte
       
      // This happens at the start of a set of numbers, so we prepare to take in a new data set. 
      bytes_seen = 0;
      seenFF = true;
    }
    // If we've started, but not finished, a new set of data...
    if(seenFF && bytes_seen < num_characters) {
      received_packet [bytes_seen] = byte;
      bytes_seen++;
    }
    // If we found the full number of bytes expected.
    if(seenFF && bytes_seen == num_characters) {
      
      
      
      // Generate the checksum to check that our data is correct.
      int checksum_int = (int) received_packet[0] + (int) received_packet[1] + (int) received_packet[2] + (int) received_packet[3];
      uint8_t checksum = checksum_int & 0xFF;
      
      if(checksum == 255) { 
        // Sender will truncate to 254 in this case; so must we.
        checksum = 254; 
      }
      
      
      if(/*checksum == received_packet[4]*/1==1){
        theta = constrain(received_packet[1],0,100);
        intensity = constrain(received_packet[2],0,100);
        duration = constrain(received_packet[3],1,100);
        p("PR: [%d][%d][%d][%d][%d]\n",received_packet[0], received_packet[1], received_packet[2], received_packet[3], received_packet[4]);
        
        
      } 
      else {
        p("ERR: [%d][%d][%d][%d][%d]\n",received_packet[0], received_packet[1], received_packet[2], received_packet[3], received_packet[4]);
      }
      bytes_seen = 0;
      seenFF = false;
      received_packet[0] = 0;
      // Flush the rest of it so we get a fresh packet next time.
      Serial.flush();
    }
  } 
}

// the loop routine runs over and over again forever:
void loop() {
  
  theta = 0;
  intensity = 0;
  duration = 0;
  
  check_bytes(); // scan USB incoming buffer for more bytes. If a complete packet, update the three variables and continue.
  pulseLED(10);
  static float motor_power[6]; // array for percentile power to each motor. Calculated from Theta, then scaled by intensity. For now, a simple range check.
  float target_period_ms = 1000;
  float high_period_ms = target_period_ms * intensity/100.0f;
  
  float min_dist_from_angle = (100.0f/4.0f)/2.0f;
  
  // For each motor...
  for(int m = 0; m < 6; m++){
    // Write a note about this later. basically we need the motors assigned to up, down, left, right, and
    //    not quad 1, 2, 3, 4.
    float current_angle = m * (100.0f/4.0f);
    
    float dist = abs(current_angle - theta);
        
    if ( m == theta || theta == 5/*dist < min_dist_from_angle || (current_angle==0 && abs(100-theta) < min_dist_from_angle) */ ) {
      digitalWrite(motorpins[m], HIGH);
    } else {
      digitalWrite(motorpins[m], LOW);
    }
    
   
  }

  int high_ms = (int)high_period_ms;
  delay(high_ms);
  for(int m = 0; m < 4; m++){
    digitalWrite(motorpins[m],LOW); 
  }
  
  int low_ms = (int)target_period_ms - high_ms;
  //delay(low_ms);
  if(duration > 0){
   duration = (int16_t) duration - (int16_t)(low_ms + high_ms)*10; 
  }
}















