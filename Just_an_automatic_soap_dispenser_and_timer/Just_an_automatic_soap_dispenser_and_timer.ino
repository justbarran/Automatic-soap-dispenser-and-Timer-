/* automatic soap and wash timer 
* by justbarran
* Please credit me
* May 2020
* www.youtube.com/c/justbarran
* 
* 
* Like Share Comment SUBSCRIBE
* This example code is in the public domain.
* Other Codes used:
* https://create.arduino.cc/projecthub/GeneralSpud/passive-buzzer-song-take-on-me-by-a-ha-0f04a8
* http://wayoda.github.io/LedControl/
* http://www.mathertel.de/Arduino/LiquidCrystal_PCF8574.aspx
*/

#define MUSIC 1 // enable music via the buzzer 0=OFF and 1=ON

#include <LedControl.h>   //By Eberhard Fahle
#include <LiquidCrystal_PCF8574.h> //By Matthias Hertel
#include <Wire.h>
#include <Servo.h>

#define lcd_addr  0x3f      //LCD i2c ADDRESS
#define voltage_const 1.5 //I am using a R1=22k and R2=10k for 16V -> 5V VOLTAGE devider
                            // (R1+R2)/R2
#define voltage_low   7.0   //Low voltage value                   
                         
#define dispense_time_ms  3000 //How long to wait to get more soap
#define soap_time_s     3      //soap up time
#define wash_time_s     20     //Washing hands time

#define red_led_1     A3      // Red LED pin
#define yellow_led_1  A2      //Yellow LED pin (I changed this to a blue LED)
#define yellow_led_2  A1      //Second Yellow LED Pin 
#define green_led_1   A0      //Green LED pin
#define battery_pin   A6      //Battery voltage sensor pin

#define pir_pin 2            // Not used - use if you want to us a IR distance sensor
#define buzzer_pin 3         //Buzzer pin
#define echo_pin 4           // Ultrasound echo pin
#define trig_pin 5          // Ultrasound trig pin
 
#define servo_timer 8       // Servo pin for analog display
#define servo_soap 9        //Servo pin for dispenser
#define servo_soap_angle 220    //My Servo wasnt accurate. Ajust servo angles
#define servo_wash_angle 130   //Ajust servo angles

#define max_DIN 11    //MAX din pin
#define max_LOAD 10   //max load pin
#define max_CLK 13    //max clock pin

LiquidCrystal_PCF8574 lcd(lcd_addr); // set the LCD address to 0x27 for a 16 chars and 2 line display

LedControl lc=LedControl(max_DIN,max_CLK,max_LOAD,1);
// DIN pin
// CLK pin
// CS pin
// 1 as we are only using 1 MAX7219

Servo mySoap; 
Servo myTimer;

//Music notes
const int c = 261;
const int d = 294;
const int e = 329;
const int f = 349;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 455;
const int b = 466;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659;
const int fH = 698;
const int fSH = 740;
const int gH = 784;
const int gSH = 830;
const int aH = 880;

unsigned int notes[74] = {//Star Wars
a,a,a,f,cH,a,f,cH,a,0,
eH,eH,eH,fH,cH,gS,f,cH,a,0,
aH,a,a,aH,gSH,gH,fSH,fH,fSH,0,
aS,dSH,dH,cSH,cH,b,cH,0,
f,gS,f,a,cH,a,cH,eH,0,
aH,a,a,aH,gSH,gH,fSH,fH,fSH,0,
aS,dSH,dH,cSH,cH,b,cH,0,
f,gS,f,cH,a,f,cH,a,0};

unsigned int notes_time[74] = {
500,500,500,350,150,500,350,150,650,500,
500,500,500,350,150,500,350,150,650,500,
500,300,150,500,325,175,125,125,250,325,
250,500,325,175,125,125,250,350,
250,500,350,125,500,375,125,650,500,
500,300,150,500,325,175,125,125,250,325,
250,500,325,175,125,125,250,350,
250,500,375,125,500,375,125,650,};

// defines variables
long duration;
int distance;
unsigned int count = 0;
unsigned int song_count = 0;

long second_timer_last = 0;
long second_timer_now = 0;

long dispense_time_last = 0;
long dispense_time_now = 0;

long soap_timer_last = 0;
long soap_timer_now = 0;

long wash_timer_last = 0;
long wash_timer_now = 0;

boolean flag_dispense = 0;
boolean flag_soap = 0;
boolean flag_wash = 0;
float v_battery = 0; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Starts the serial communication
  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("* JUST BARRAN  *");
  delay(3000);
  lcd.setCursor(0, 1);
  lcd.print("LIKE & SUBSCRIBE");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SOAP DISPENSER ");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("AND WASH TIMER");
  
  pinMode(red_led_1,OUTPUT);
  pinMode(yellow_led_1,OUTPUT);
  pinMode(yellow_led_2,OUTPUT);
  pinMode(green_led_1,OUTPUT);
  pinMode(buzzer_pin,OUTPUT);
  pinMode(trig_pin,OUTPUT);
  pinMode(echo_pin,INPUT);
  pinMode(pir_pin,INPUT);
  
  lc.shutdown(0,false);// turn off power saving, enables display
  lc.setIntensity(0,8);// sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  mySoap.attach(servo_soap);  // attaches the servo on pin 9 to the servo object  
  mySoap.write(0);   // Sets Servo to initially 0 degrees 
  delay(1000);
     mySoap.detach();  
  myTimer.attach(servo_timer);  // attaches the servo on pin 9 to the servo object  
  myTimer.write(0);   // Sets Servo to initially 0 degrees 
  delay(1000);  
  myTimer.detach();
  beep(1000);
  lcd.clear();
  lcd.setBacklight(0);
}

void loop() {
  
    second_timer_now = millis();
    if((second_timer_now - second_timer_last) >= 1000)
    {
      count ++;
      second_timer_last = second_timer_now;
      int analogInput = analogRead(battery_pin);
      float vout = (analogInput*5.0)/1024.0;
      v_battery = vout*voltage_const;
      if(v_battery < voltage_low)
      {
        beep(100);
      }
    }  
    if(flag_dispense == 0){
        digitalWrite(trig_pin, LOW);
        delayMicroseconds(2);
        digitalWrite(trig_pin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig_pin, LOW);
        duration = pulseIn(echo_pin, HIGH);
        distance= duration*0.034/2;      
        if(distance == 5){ //Check distance is less than 10cm   
          beep(50);
          dispense();
          flag_dispense = 1;
          dispense_time_last = millis();
        }
      }

      if(flag_dispense == 1)
      {
        dispense_time_now = millis();
        if((dispense_time_now-dispense_time_last)>= dispense_time_ms)
        {
          flag_dispense = 0;
        }
        if(flag_soap == 0 && flag_wash == 0)
        {
           flag_soap = 1;
           count = 0;
           song_count = 0;
        }
      }    
         if((flag_wash == 1 || flag_soap == 1)&& MUSIC)
      {
        song(notes[song_count],notes_time[song_count]);
        song_count ++;
      }
      
      if(soapTimer(count,flag_soap) == 0)
      {
        flag_soap = 0;
        flag_dispense = 0;
        flag_wash =  1;
        count = 0;
      }
      
      if(washTimer(count,flag_wash) == 0)
      {
        flag_wash =  0;
        flag_soap = 0;
        flag_dispense = 0;
      }
      delay(10);
}

//Funtion to dispense soap
void dispense(){
  mySoap.attach(servo_soap);  // attaches the servo on pin 9 to the servo object
  delay(10);
  mySoap.write(180);   // Sets Servo to initially 0 degrees
  delay(1000);
  mySoap.write(0);   // Sets Servo to initially 0 degrees 
  delay(1000);
  mySoap.detach();  // attaches the servo on pin 9 to the servo object  
}

//Funtion to Beep sound
void beep(unsigned int beepTime){
   tone(buzzer_pin,c,beepTime);
   delay(beepTime);
   noTone(buzzer_pin); 
}

//Funtion to set 7 seg display value
void Seg7(byte value){
      byte tens = value/10; 
      byte ones = value%10;
      lc.setDigit(0,0,ones,false);
      lc.setDigit(0,1,tens,false);
}

//Funtion to do first soap timer 
int soapTimer(unsigned int timer , boolean state){
  if(state == 1)
  {
    lcd.setBacklight(255);
    myTimer.attach(servo_timer);
    if(timer <= soap_time_s)
    {
      digitalWrite(red_led_1,HIGH);
      unsigned int soap_time = soap_time_s - timer;
      lcd.setCursor(3, 0);
      lcd.print("SOAP HANDS");      
      lcd.setCursor(6, 1);
      lcd.print("SEC LEFT");
      lcd.setCursor(3, 1);
      if(soap_time <10)
      {
        lcd.print(" ");
      }
      lcd.print(soap_time);
      Seg7(soap_time);
      unsigned int soap_servo = map(timer, 0, soap_time_s,servo_soap_angle, servo_wash_angle);
      myTimer.write(soap_servo);
      return 1;
    }
    else
    {
      beep(500);
      digitalWrite(red_led_1,LOW);
      lc.clearDisplay(0);
      return 0;
    }
  }
  return -1;
}

//Funtion to do wash timer 
int washTimer(unsigned int timer , boolean state){
  if(state == 1)
  {
    if(timer <= wash_time_s)
    {
      myTimer.attach(servo_timer);
      lcd.setBacklight(255);
      digitalWrite(red_led_1,HIGH);
      digitalWrite(yellow_led_1,HIGH);
      unsigned int wash_time = wash_time_s - timer;
      lcd.setCursor(3, 0);
      lcd.print("WASH HANDS");      
      lcd.setCursor(6, 1);
      lcd.print("SEC LEFT");
      lcd.setCursor(3, 1);
      if(wash_time <10)
      {
        lcd.print(" ");
      }
      lcd.print(wash_time);
      Seg7(wash_time);
      unsigned int wash_servo = map(timer, 0, wash_time_s, servo_wash_angle, 0);
      myTimer.write(wash_servo);
      if(timer > wash_time_s/2)
      {
        digitalWrite(yellow_led_2,HIGH);
      }
      if(timer == wash_time_s)
      {
        digitalWrite(green_led_1,HIGH);
      }
      return 1;
    }
    else
    {
      myTimer.detach();
      lcd.setCursor(0, 0);
      lcd.print("   ALL DONE   ");
      lcd.setCursor(0, 1);
      lcd.print("   GOOD JOB   ");
      beep(2000);
      digitalWrite(red_led_1,LOW);
      digitalWrite(yellow_led_1,LOW);
      digitalWrite(yellow_led_2,LOW);
      digitalWrite(green_led_1,LOW);
      lc.clearDisplay(0);
      lcd.clear();
      lcd.setBacklight(0);
      return 0;
    }
  }
  return -1;
}

//Funtion to play a music note 
void song(int note, int duration)
{
  if(note > 0)
  {
    tone(buzzer_pin, note, duration);
  }
  delay(duration);
  noTone(buzzer_pin); 
  delay(50);
}
