  /*
    Ultrasonic sensor Pins:
        VCC: +5VDC
        Trig : Trigger (INPUT) - Pin7
        Echo: Echo (OUTPUT) - Pin 8
        GND: GND
 */
 
#include <elapsedMillis.h>
#include <virtuabotixRTC.h>
#include <NewPing.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <HX711.h>

#define sensorAir A0
#define MAX_DISTANCE 11

//HX711 scale;
#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN  6
#define LOADCELL_SCK_PIN  10

HX711 scale;

//Servo
Servo myservo;

elapsedMillis OutputRTCInterval;
elapsedMillis MakananInterval;
elapsedMillis MinumanInterval;
elapsedMillis BeratMknInterval;
elapsedMillis DispenserInterval;

unsigned long timeberatMkn      = 2000;  
unsigned long timerlcdInterval  = 1000;  //3000
unsigned long timerMakanan      = 2000;  //3000
unsigned long timerMinuman      = 1700;  //3000
unsigned long TimerDispenser    = 2000;

// Creation of the Real Time Clock Object
virtuabotixRTC myRTC(3, 4, 2); // clk ,dat,rst

char buff[11];

int trigPin = 8;    // Trigger
int echoPin = 7;    // Echo
int jarak;
int levelAir;
int buzzPin = 13;     //buzzer
int pos = 0;
unsigned int airValue = 0;

//minuman
int pompa = 9;                // Pin untuk RelayPompa
int sensorPir = 5;              // Pin data Sensor Pir
int state = LOW;             // Status Keadaan sensor Pir
int valPir = 0;                 // Variable untuk status Sensor Pir

// Mnegatur LCD 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

//inisiasi library ultrasonik
NewPing sonar(trigPin, echoPin, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
  //Serial Port begin
  Serial.begin (9600);
 
  // Uncomment kodingan ini jika ingin menatur waktu dengan format seperti berikut: 
  // Detik, Menit, Jam, Hari dalam seminggu, Hari dalam sebulan, Bulan, Tahun
  // myRTC.setDS1302Time(01, 18, 17, 1, 26, 06, 2023);

  // Set faktor kalibrasi dan offset pada HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  
  myservo.attach(12);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pompa, OUTPUT);      // initalize pompa as an output
  pinMode(sensorPir, INPUT);    // initialize sensor as an input

  lcd.begin();
  lcd.backlight();

  // set the cursor to the top left
  lcd.setCursor(0, 0);
  // Print a message to the lcd.
  lcd.print("Memulai");
  for (int j = 0; j < 5; j++) {
    lcd.print(".");
    delay(1000);
  }
  lcd.clear();

  lcd.setCursor(6, 0);
  lcd.print("PROYEK SK");
  lcd.setCursor(4, 1);
  lcd.print("Kelompok 5");
  delay(2000);

  lcd.clear();
}
 
void loop() {
  myRTC.updateTime();      
  myservo.write(pos);
  
  int jam = myRTC.hours;
  int menit = myRTC.minutes;
  int detik = myRTC.seconds;
   
  rtctoLCD(jam, menit);
  minuman(menit,detik);
  makanan(jam, menit, detik);
  
  // Delay so the program doesn't print non-stop
  delay(100);
}


void minuman(int waktuBuzzer, int detikWL){
  
  if(MinumanInterval >= timerMinuman)
  {
    calculateDist( waktuBuzzer);
//    delay(50);
    MinumanInterval = 0;
  }

  if(DispenserInterval >= TimerDispenser)
  {
    airValue = analogRead(sensorAir);
    int mapping = map(airValue, 0 , 1023, 1,5);
    DetectPir(mapping, detikWL);

    DispenserInterval = 0;
  }
  
  delay(100);
  
}
void kalkulasiBeratMakanan(){
    // Baca berat pada HX711
    int weight = scale.get_units();
  
    // Tampilkan berat pada serial monitor
    Serial.print("Berat= ");
    Serial.print(weight);
    Serial.println(" gram");
  
    // Tampilkan berat pada LCD
    lcd.setCursor(9, 1);
    lcd.print("MKN: ");
    lcd.setCursor(14, 1);
    lcd.print("  ");
    lcd.setCursor(14, 1);
    lcd.print(weight);
    
    // Tunggu sejenak
}

void makanan(int jam, int menit, int detik){

  if(BeratMknInterval >= timeberatMkn){

    kalkulasiBeratMakanan();
    BeratMknInterval = 0;
    
  }
  
  if(MakananInterval >= timerMakanan){
    
    if (jam == 17 && menit == 31 && detik <= 5)
    {
      myservo.write(180);
      delay(2000);//delay selama 2 detik
    }
    else if(jam == 8 && menit == 0 && detik <= 5)
    {
      myservo.write(180);
      delay(2000);//delay selama 2 detik
    }
    else if(jam == 2 && menit== 2 && detik <= 5)
    {
      myservo.write(180);
      delay(2000);//delay selama 2 detik
    }

    MakananInterval = 0;
    
  }
  delay(100);
}
  
void calculateDist(int menit_b){
  jarak = sonar.ping_cm();
  levelAir = map(jarak, 10, 0, 0, 100);
  digitalWrite(buzzPin, LOW);
  
  Serial.print("Ukuran air: ");
  Serial.print(jarak);
  Serial.println("%");

//  delay(50);

  //LCD
  lcd.setCursor(0, 1);
  lcd.print("air= ");
  lcd.setCursor(4, 1);
  lcd.print("   %");
  lcd.setCursor(4, 1);
  lcd.print(levelAir);

 
  if(jarak > 30){
    Serial.println("Air mau Habiss");
    //delay(50);
    if(menit_b%3 == 0){
        digitalWrite(buzzPin, HIGH);
        delay(20);
    }
  }
}

void rtctoLCD(int jam, int menit){
  
    if(OutputRTCInterval >= timerlcdInterval){
      sprintf(buff, "%d:%d", jam, menit);

      // Tampilan LCD
//      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("jam= ");
      lcd.setCursor(5, 0);
      lcd.print(buff);
      
      OutputRTCInterval=0;
    }
}

void DetectPir(int mapsensor, int detikPir){
  //nilai Pir 
  valPir = digitalRead(sensorPir);
  Serial.print("Nilai : ");
  Serial.println(valPir);

  Serial.print("Sensor Air: ");
  Serial.println(mapsensor);
  
  if (valPir == HIGH) 
  {           // check if the sensor is HIGH
    digitalWrite(pompa, LOW);   // turn LED ON
//    delay(100);                // delay 100 milliseconds 
    
    if (state == LOW)
    {
      Serial.println("Motion detected!"); 
//      if(mapsensor <= 2 && detikPir%10 == 0){
//        for(int i = 0; i <= 20; i++)
//        {
//        digitalWrite(buzzPin, HIGH);
//        delay(150);
//        }
//      }

      if(mapsensor >= 5){
        digitalWrite(pompa, HIGH);
      }
      state = HIGH;       // update variable state to HIGH 
    }
    
  } 
  else 
  {
      digitalWrite(pompa, HIGH); // turn LED OFF
      
      if (state == HIGH)
      {
        Serial.println("Motion stopped!");
        state = LOW;       // update variable state to LOW
      }
  }
}
