#define BLYNK_TEMPLATE_ID "TMPL6pctGS8tS"
#define BLYNK_TEMPLATE_NAME "Smart Home Energy Monitor"
#define BLYNK_AUTH_TOKEN "htWFlIyIxCJnCXRDgLY3A8fkYGLhlDj8"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define POT_PIN   34
#define LOAD1_PIN 18
#define LOAD2_PIN 19

const float VOLTAGE = 230.0;
const float POWER_LIMIT = 2000.0;

float current = 0;
float power = 0;
float energy = 0;

bool load1State = false;
bool load2State = false;
bool alertSent = false;

unsigned long previousMillis = 0;

BlynkTimer timer;

/**********************
   Appliance 1 Control
**********************/
BLYNK_WRITE(V4)
{
  load1State = param.asInt();
  digitalWrite(LOAD1_PIN, load1State);
}

/**********************
   Appliance 2 Control
**********************/
BLYNK_WRITE(V5)
{
  load2State = param.asInt();
  digitalWrite(LOAD2_PIN, load2State);
}

/**********************
   Sensor Processing
**********************/
void sendSensorData()
{
  int adcValue = analogRead(POT_PIN);

  // Simulated Current: 0A - 10A
  current = ((float)adcValue / 4095.0) * 10.0;

  // Power Calculation
  power = VOLTAGE * current;

  // Energy Calculation
  unsigned long currentMillis = millis();

  float elapsedHours =
      (currentMillis - previousMillis) / 3600000.0;

  energy += (power * elapsedHours) / 1000.0;

  previousMillis = currentMillis;

  // Send Data to Blynk
  Blynk.virtualWrite(V0, VOLTAGE);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);

  /**********************
     High Power Logic
  **********************/
  if(power > POWER_LIMIT)
  {
    // Turn OFF Appliance 2
    digitalWrite(LOAD2_PIN, LOW);
    load2State = false;

    Blynk.virtualWrite(V5, 0);

    if(!alertSent)
    {
      Blynk.logEvent(
        "high_power",
        "Power exceeded 2000W. Appliance 2 turned OFF."
      );

      alertSent = true;
    }
  }
  else
  {
    alertSent = false;
  }

  // Serial Monitor Output
  Serial.println("--------------------------------");

  Serial.print("Voltage: ");
  Serial.print(VOLTAGE);
  Serial.println(" V");

  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" A");

  Serial.print("Power: ");
  Serial.print(power);
  Serial.println(" W");

  Serial.print("Energy: ");
  Serial.println(energy,6);
  Serial.println(" kWh");

  Serial.print("Load 1: ");
  Serial.println(load1State ? "ON" : "OFF");

  Serial.print("Load 2: ");
  Serial.println(load2State ? "ON" : "OFF");
}

/**********************
        Setup
**********************/
void setup()
{
  Serial.begin(115200);

  pinMode(POT_PIN, INPUT);

  pinMode(LOAD1_PIN, OUTPUT);
  pinMode(LOAD2_PIN, OUTPUT);

  digitalWrite(LOAD1_PIN, LOW);
  digitalWrite(LOAD2_PIN, LOW);

  previousMillis = millis();

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  timer.setInterval(1000L, sendSensorData);
}

/**********************
        Loop
**********************/
void loop()
{
  Blynk.run();
  timer.run();
}