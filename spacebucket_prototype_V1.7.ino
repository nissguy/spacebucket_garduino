
//Spacebucket Controller Prototype
const String ver = ("Version 1.7");

//V1.7 Soil moisture sensor setup and functioning correctly.
//Created Watering function, and Plant Stage function to clean up Loop. 
//V1.6 Push button mode selection setup and functioning correctly.
//V1.5 Booleans revised based on plant stage. Working correctly.
//V1.4 Light output boolean conditions working correctly.
//V1.3 Fan ouput boolean conditions working correctly.
//V1.2 RTC working correctly.
//V1.1 DHT22 & Nokia 5110 work correctly.
//Prototype for automated spacebucket.

//Libraries
#include "DHT.h"
#include "Nokia_5110.h"
#include "RTClib.h"
#include "Wire.h"

//Constants
#define DHTPIN 2 //Defines digital pin for DHT comm
#define DHTTYPE DHT22 //Defines DHT type
DHT dht(DHTPIN, DHTTYPE); //Initializes DHT Sensor
RTC_DS1307 RTC;
DateTime currentDateTime;

#define RST 6
#define CE 7
#define DC 5
#define DIN 11
#define CLK 13
const String stage; //sets the stage in the plants life

Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK);

//Variables
static int chk;
static unsigned long startTime; //declares startTime as an unsigned integer
static float hum; //stores humidity value
static float temp; //stores temperature value
static String fan; //Fan relay condition value
static String water; //Watering relay condition value
static String light; //Light relay condition value
static int fanout = 12; //sets fan relay output pin at D12
static int lightout = 9; //sets light relay output pin at D9
static int waterout = 8; //sets water relay output at D8
static int pushbutton = 3; //sets stage change button input pin at D3
static int buttonPushCounter = 0; //counter for the number of button presses
static int buttonState = 0; //current state of the button
static int lastButtonState = 0; //previous state of the button
static char* stageName[5] = {"Seedling", "Vegetating", "Flowering", "Late Flowering"};
static int stageCount = 5;
static int moistin = 10; //Soil moisture sensor input pin D4
static int soilMoisture;
static String soilState;

void setup () //Start of program
{
  Serial.begin (9600); //Initializes serial port, baud rate.
  dht.begin();
  Wire.begin();
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //Pulls date/time from compiler

  
  //Sets pinmodes 
  pinMode(fanout, OUTPUT);
  pinMode(lightout, OUTPUT);
  pinMode(lightout, OUTPUT);
  pinMode(pushbutton, INPUT); //Needs pulldown resistor
  pinMode(moistin, INPUT); //Needs pulldown resistor
  
  lcd.setContrast(60); //sets LCD contrast level

  lcd.print("Initializing");
  lcd.setCursor(0,1); //sets cursor to line 2
  lcd.print("SpaceBucket");
  lcd.setCursor(0,2); //sets cursor to line 3
  lcd.print("Controller");
  lcd.setCursor(0,4); //sets cursor to line 5
  lcd.print(ver);
  delay(5000); //5 sec delay
  lcd.clear();

  if (! RTC.isrunning())
  {
    Serial.println("RTC is NOT running!");
    lcd.println("RTC is NOT running!");
    //Following line sets the RTC to the date & time this sketch was
    //compiled. Comment it out & upload to set the time and date and
    //run RTC!
    }

}
    


void loop()
{
StageSet();

//Get information from RTC & Sensors
DateTime now=RTC.now();
hum = dht.readHumidity(); //Read data and store to hum variable
temp = dht.readTemperature(); //Read data and store to temp variable

soilMoisture = digitalRead(moistin); //Read digital value
if (soilMoisture == LOW){
  soilState = ("MOIST");
  }else{
    soilState = ("DRY");
    Watering();
  }
  
displayinfo();



  

     


// ENVIRONMENTAL CONDITIONS
// Seedlings ideal conditons: 66-70% RH, 20-25C, 18-24hr light
// Vegetation Period ideal conditions: 40-70% RH, 18-24hr light
// Flowering Period ideal conditions: 40-50% RH (55% MAX), 20-26C 22-28C 12/12hr light
// Late Flowering (1-2 wks before harvest) ideal conditions: 30-40% RH, 18-24C 22-28C 12/12hr light

static int seedlingLightStart = 10; //1000h light start
static int seedlingLightStop = 6; //0600h light end
static int vegetationLightStart = 10; //1000 light start
static int vegetationLightStop = 22; //2200h light end

//Temperature & Humidity Control 
// Seedling Stage
if (stage == "Seedling") {
    if (temp >25 || hum >70) { // If temperature or humidity exceed setpoint run fan.
      digitalWrite (fanout, HIGH);
      fan = ("ON1");  
      } else {
      digitalWrite (fanout, LOW);
      fan = ("OFF1");
      }
    if ((now.hour() >= seedlingLightStop && (now.hour() <= seedlingLightStart)))  { 
      digitalWrite (lightout, LOW);
      light = ("OFF1");  
      }else{
      digitalWrite (lightout, HIGH);
      light = ("ON1");}
}

// Vegitating Stage
if (stage == "Vegetating") {
    if (temp >28 || hum >70) { // If temperature or humidity exceed setpoint run fan.
      digitalWrite (fanout, HIGH);
      fan = ("ON2");  
      } else {
      digitalWrite (fanout, LOW);
      fan = ("OFF2");}
      if ((now.hour() >= vegetationLightStop && (now.hour() <=vegetationLightStart))) {
      digitalWrite (lightout, LOW);
      light = ("OFF2");
      }else{
      digitalWrite (lightout, HIGH);
      light = ("ON2");}
}

//Flowering Stage
if (stage == "Flowering") {
    if (temp >26 || hum >50) { // If temperature or humidity exceed setpoint run fan.
      digitalWrite (fanout, HIGH);
      fan = ("ON3");  
      } else {
      digitalWrite (fanout, LOW);
      fan = ("OFF3");}
      if ((now.hour() >= vegetationLightStop && (now.hour() <=vegetationLightStart))) {
      digitalWrite (lightout, LOW);
      light = ("OFF2");
      }else{
      digitalWrite (lightout, HIGH);
      light = ("ON2");}
 }
//Late Flowering Stage
if (stage == "Late Flowering") {
    if (temp >24 || hum >40) { // If temperature or humidity exceed setpoint run fan.
    digitalWrite (fanout, HIGH);
    fan = ("ON4");  
    } else {
    digitalWrite (fanout, LOW);
    fan = ("OFF4");}
    if ((now.hour() >= vegetationLightStop && (now.hour() <=vegetationLightStart))) {
    digitalWrite (lightout, LOW);
    light = ("OFF2");
    }else{
    digitalWrite (lightout, HIGH);
    light = ("ON2");}
}
if (stage == "NONE") {
    fan = ("OFF");
    digitalWrite (fanout, LOW);
    water = ("OFF");
    light = ("OFF");
    digitalWrite (lightout, LOW);
}
}

void Watering()
{
  digitalWrite (waterout, HIGH);
  lcd.print("Watering.");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  digitalWrite (waterout, LOW);
  
  
}

void StageSet()
{
Serial.print("button pushes: ");
  Serial.print(buttonPushCounter);
buttonState = digitalRead(pushbutton);
if (buttonState == HIGH){
  buttonPushCounter++;
  if (buttonPushCounter % stageCount == 0){
    stage = ("NONE");
    lcd.clear();
    lcd.print(stage);
  }
  if (buttonPushCounter % stageCount == 1){
    stage = ("Seedling");
    lcd.clear();
    lcd.print(stage);
  }
  if (buttonPushCounter % stageCount == 2){
    stage = ("Vegetating");
    lcd.clear();
    lcd.print(stage);
  }
  if (buttonPushCounter % stageCount == 3){
    stage = ("Flowering");
    lcd.clear();
    lcd.print(stage);
  }
  if (buttonPushCounter % stageCount == 4){
    stage = ("Late Flowering");
    lcd.clear();
    lcd.print(stage);
  }
  if (buttonPushCounter >4){
    buttonPushCounter = 0;
  }
  
  //delay(400);  
}
}

void displayinfo()
{
  DateTime now=RTC.now();
  
    lcd.setCursor(0, 0);
  lcd.print("Plant Stage: ");
  lcd.setCursor(0, 1);
  lcd.print(stage);
  
  lcd.setCursor(0, 3); //sets cursor to line 4
  lcd.print("Humid: ");
  lcd.print(hum);
  lcd.print(" %");
  lcd.setCursor(0, 4); //sets cursor to line 5
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  lcd.setCursor(0, 5); //sets cursor to line 6
  lcd.print("Soil: ");
  lcd.print(soilState);
  delay(1000);
  lcd.clear();

  lcd.print("Fan: ");
  lcd.print(fan);
  lcd.setCursor(0, 1); //sets cursor to line 2
  lcd.print("Light: ");
  lcd.print(light);
  lcd.setCursor(0, 4); //sets cursor to line 5
  lcd.print(now.year(), DEC);
  lcd.print("/");
  lcd.print(now.month(), DEC);
  lcd.print("/");
  lcd.print(now.day(), DEC);
  lcd.setCursor(0, 5); //sets cursor to line 6
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  lcd.print(now.second(), DEC);
  
  delay(2000); //delay(2000); //miliseconds to delay loop
  lcd.clear();
  
  //Print temp & humidity values to serial monitor
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" - ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print(" - ");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celcius");
  Serial.print("Soil: ");
  Serial.print(soilState);
  Serial.print("Plant Stage: ");
  Serial.print(stage);
  Serial.print("\n");
  Serial.print("Fan: ");
  Serial.print(fan);
  Serial.print(" Light: ");
  Serial.print(light);
  //delay (2500); //Delay .5 sec.
}

