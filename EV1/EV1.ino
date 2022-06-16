#include <METER.h>
#include <EVController.h>
#include <RTC.h>
#include <RFID.h>

#define LED_R 33

METER meter;
String voltage, current, power, energy;


EVCONTROLLER ev;
uint16_t en;
String value;
String vehicleStatus;
uint16_t charginCurrentViaCP;
uint8_t statusInputs;
uint8_t statusOutputs;
uint16_t timeConnectHH;
uint8_t timeConnectMM;
uint8_t timeConnectSS;
uint16_t timeChargingHH;
uint8_t timeChargingMM; 
uint8_t timeChargingSS;
uint16_t errorStatus;

String lastVehicleStatus;
String lastTimeConnect;
String lastTimeCharging;

RTC rtc;
//String date;
String times;
String temp;
bool success1;

byte date;
byte month; 
uint16_t year;
byte hour;
byte minute; 
byte seconds;

RFID rfid;
String id;
boolean success2;


unsigned long last_time = 0;
unsigned long period = 1000;
String last_state;

void printAllStatusget(String vehicleStatus, uint16_t charginCurrentCP, uint8_t statusInputs, uint8_t statusOutputs,uint16_t timeConnectHH, uint8_t timeConnectMM, uint8_t timeConnectSS 
      ,uint16_t timeChargingHH , uint8_t timeChargingMM, uint8_t timeChargingSS , uint16_t errorStatus);
      
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_R, OUTPUT);
  digitalWrite(LED_R, HIGH);    delay(1000);
  digitalWrite(LED_R, LOW);    delay(1000);
  digitalWrite(LED_R, HIGH);    delay(1000);
  digitalWrite(LED_R, LOW);    delay(1000);
  digitalWrite(LED_R, HIGH);    delay(1000);
  
  Serial.begin(115200);
  Serial.println("Start");

  Serial2.begin(9600);
  // Initial meter modbus slave 1
  meter.init(1, Serial2);
  Serial.println("initial meter OK");
  
  // Initial EV Controller modbus slave 2
  ev.init(2, Serial2); 
  Serial.println("Initial EV controller OK");
  
  // Initial rtc
  if(rtc.init())
  {
    Serial.println("init RTC ok");
  } else
  {
    Serial.println("init RTC fail");
  }

  // initial rfid 
  if(rfid.init())
  {
    Serial.println("RFID Ready");
  }
  else
  {
    Serial.println("RFID Fail");
  }
  Serial.println();

  Serial.println("Setting EV Controller");
  // set en mode 3  address 4000
  if(ev.setEn(3))
  {
    Serial.println("Set mode 3 OK");
  }
  delay(10);
  
  // get mode en address 4000
  if(ev.getEn(en))
  {
    Serial.print("Mode = ");
    Serial.println(en);
  }
  Serial.println();
  delay(10);

  // set coil address 20000 OFF
  if(ev.setCoil20000("OFF"))
  {
    Serial.println("Off coil 20000");
  }
  delay(10);

  // get coil address 20000 
  if(ev.getCoil20000(value))
  {
    Serial.print("value coil 20000 = ");   Serial.println(value);
  }
  Serial.println();
  delay(10);
  
  // get last state
  if(ev.getVehicleStatus(vehicleStatus))
  {
    if(vehicleStatus == "C1")
    {
      delay(10);  
      if(ev.setCoil20000("ON"))
      {
        Serial.println("ON coil 20000");
        lastVehicleStatus = "B2";
      }
         
    }
    else if(vehicleStatus == "B1")
    {
      delay(10);  
      if(ev.setCoil20000("ON"))
      {
        Serial.println("ON coil 20000");
        lastVehicleStatus = "B1";
      }
         
    }     
    else
      lastVehicleStatus = vehicleStatus;
      
    Serial.print("Last state = ");  Serial.println(lastVehicleStatus);
  }
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - last_time > period)
  {
    last_time = millis();
    // get time current
    rtc.get_rtc_separate(date, month, year, hour, minute, seconds);
    Serial.print("Time ");
    Serial.print(date); Serial.print("/");  Serial.print(month); Serial.print("/"); Serial.print(year); Serial.print("   ");
    Serial.print(hour); Serial.print(":");  Serial.print(minute);  Serial.print(":"); Serial.println(seconds);
    
    Serial.print("Last status : "); Serial.println(lastVehicleStatus);
    if(ev.getAllStatus(vehicleStatus, charginCurrentViaCP, statusInputs, statusOutputs, timeConnectHH , timeConnectMM, timeConnectSS
      , timeChargingHH , timeChargingMM, timeChargingSS , errorStatus))
    {
      printAllStatusget(vehicleStatus, charginCurrentViaCP, statusInputs, statusOutputs, timeConnectHH, timeConnectMM, timeConnectSS 
      , timeChargingHH , timeChargingMM, timeChargingSS , errorStatus);
      
      if((lastVehicleStatus == "A1" || lastVehicleStatus == "B2") && vehicleStatus == "B1")
      {
        lastVehicleStatus = "B1";
        delay(10);
        if(ev.setCoil20000("ON"))
        {
           Serial.println("Set coil 20000 ON OK");   
        }            
      } 
      else if(lastVehicleStatus == "B1" && vehicleStatus == "C2")
      {
        lastVehicleStatus = "B2";
        Serial.println("Start charging");
      }
      else if(lastVehicleStatus == "B2" && vehicleStatus == "B2")
      {
        lastVehicleStatus = "C2";
        Serial.println("Finish charging");
      }
      else if(lastVehicleStatus == "C2" && vehicleStatus == "A1")
      {
        lastVehicleStatus = "B2";
        delay(10);
        if(ev.setCoil20000("OFF"))
        {
           Serial.println("Set coil 20000 OFF OK");   
        }    
      }
      else if(lastVehicleStatus == "C2" && vehicleStatus == "B2")
      {
        Serial.println("Keep time ");
        lastTimeConnect  = String(timeConnectHH) + ":" + String(timeConnectMM) + ":" + String(timeConnectSS);
        lastTimeCharging = String(timeChargingHH) + ":" + String(timeChargingMM) + ":" + String(timeChargingSS);
      }
      else if(lastVehicleStatus == "B2" && vehicleStatus == "A1")
      {
        Serial.print("Last time connect "); Serial.println(lastTimeConnect);
        Serial.print("Last time charging"); Serial.println(lastTimeCharging);
      }

     // Keep time
     // Time connect
     if(vehicleStatus == "B1" || vehicleStatus == "B2" 
     || vehicleStatus == "C1" ||  vehicleStatus == "C2")
     {
        Serial.print("Time connect "); 
        Serial.print(timeConnectHH);  Serial.print(":");
        Serial.print(timeConnectMM);  Serial.print(":");
        Serial.println(timeConnectSS);
     }

     // Time charging
     if(vehicleStatus == "C2")
     {
        Serial.print("Time charging "); 
        Serial.print(timeChargingHH);  Serial.print(":");
        Serial.print(timeChargingMM);  Serial.print(":");
        Serial.println(timeChargingSS);
     }

     Serial.println();
     
    }
  }
  
}

void printAllStatusget(String vehicleStatus, uint16_t charginCurrentCP, uint8_t statusInputs, uint8_t statusOutputs,uint16_t timeConnectHH, uint8_t timeConnectMM, uint8_t timeConnectSS 
      ,uint16_t timeChargingHH , uint8_t timeChargingMM, uint8_t timeChargingSS , uint16_t errorStatus){
  // address 24000 get vehicle status A1, A2, B1, B2, C1, C2
    Serial.print("Vehicle status = ");  Serial.println(vehicleStatus);
    //Serial.println();
/*
    // address 24003 get charging current specification via signal CP 
    Serial.print("Charging current specification via signal CP = ");  Serial.println(charginCurrentViaCP);
    Serial.println();

    // addres 24004 get status of digital inputs 
    Serial.print("Status of digital inpus = ");  Serial.println(statusInputs, BIN);
    // Bit 0 is OUT
    if((statusInputs&0b00000001) == 0b00000001)
      Serial.print("EN = 1 ");
    else
      Serial.print("EN = 0 ");

    // Bit 1 is OUT
    if((statusInputs&0b00000010) == 0b00000010)
      Serial.print("XR = 1 ");
    else
      Serial.print("XR = 0 ");

    // Bit 2 is OUT
    if((statusInputs&0b00000100) == 0b00000100)
      Serial.print("ML = 1 ");
    else
      Serial.print("ML = 0 ");

    // Bit 3 is CON
    if((statusInputs&0b00001000) == 0b00001000)
      Serial.print("CCR = 1 ");
    else
      Serial.print("CCR = 0 ");

    // Bit 4 is IN
    if((statusInputs&0b00010000) == 0b00010000)
      Serial.print("IN = 1 ");
    else
      Serial.print("IN = 0 ");

    Serial.println();
    Serial.println();
    
    // Address 24005 status of digital outputs
    Serial.print("Status of digital outputs = ");  Serial.println(statusOutputs, BIN);

     // Bit 0 is OUT
    if((statusOutputs&0b00000001) == 0b00000001)
      Serial.print("OUT = 1 ");
    else
      Serial.print("OUT = 0 ");

    // Bit 1 is OUT
    if((statusOutputs&0b00000010) == 0b00000010)
      Serial.print("ERR = 1 ");
    else
      Serial.print("ERR = 0 ");

    // Bit 2 is OUT
    if((statusOutputs&0b00000100) == 0b00000100)
      Serial.print("CHG = 1 ");
    else
      Serial.print("CHG = 0 ");

    // Bit 3 is CON
    if((statusOutputs&0b00001000) == 0b00001000)
      Serial.print("CON = 1 ");
    else
      Serial.print("CON = 0 ");

    Serial.println();
    Serial.println();
    
    // Address 24019 - 24020 time connect hhhh, mm ,ss
    Serial.print("Time connect Hours = ");
    Serial.println(timeConnectHH);
    Serial.print("Time connect Minutes = ");
    Serial.println(timeConnectMM);
    Serial.print("Time connect Second = ");
    Serial.println(timeConnectSS);
    Serial.println();
    
    // Address 24017 - 24018 time connect hhhh, mm ,ss
    Serial.print("Time charging Hours = ");
    Serial.println(timeChargingHH);
    Serial.print("Time charging Minutes = ");
    Serial.println(timeChargingMM);
    Serial.print("Time charging Second = ");
    Serial.println(timeChargingSS);
    Serial.println();
    
    // Address 24025 Error status
    Serial.print("Error status = ");  Serial.println(errorStatus, BIN);
    // Bit 2 rejection of 13 A cable
    if((errorStatus&0x0002) == 0x0002)
      Serial.println("rejection of 13A cable ");
    // Bit 3 invalid PP value
    if((errorStatus&0x0004) == 0x0004)
      Serial.println("invalid PP value ");
    // Bit 4 invalid CP value
    if((errorStatus&0x0008) == 0x0008)
      Serial.println("invalid CP value ");
    // Bit 5 status F due to no charging station availability
    if((errorStatus&0x0010) == 0x0010)
      Serial.println("status F due to no charging station availability ");
    // Bit 6 locking
    if((errorStatus&0x0020) == 0x0020)
      Serial.println("locking");
    // Bit 7 locking
    if((errorStatus&0x0040) == 0x0040)
      Serial.println("unlocking");
    // Bit 8 LD unavailable during locking
    if((errorStatus&0x0080) == 0x0080)
      Serial.println("LD unavailable during locking");
    // Bit 11 status D, vehicle rejected
    if((errorStatus&0x0400) == 0x0400)
      Serial.println("status D, vehicle rejected");
    // Bit 12 charging contactor error
    if((errorStatus&0x0800) == 0x0800)
     Serial.println("charging contactor error");
    // Bit 13 no diode in the Control Pilot circuit in the vehicle
    if((errorStatus&0x1000) == 0x1000)
      Serial.println("no diode in the Control Pilot circuit in the vehicle");
    // Bit 15 EV-RCM residual current detection triggered
    if((errorStatus&0x4000) == 0x4000)
      Serial.println("EV-RCM residual current detection triggered");
    // Bit 16 EV-RCM selftest error
    if((errorStatus&0x8000) == 0x8000)
      Serial.println("EV-RCM selftest error");
  */
}
