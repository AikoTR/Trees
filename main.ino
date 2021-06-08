#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Seeed_BMP280.h>

#define RxD 6
#define TxD 7

BMP280 bmp280;

const int motorPin = 2;
const int batteryPin = A2;
const int humidityPin = A1;
const int lightPin = A0;
const int waterPin = 5;
const int baroPin = 0x77;

const int humidityWantedValue = 30;
const int lightWantedValue = 0;
const int waterWantedValue = 0;
const int atmosWantedValue = 0;
const int tempWantedValue = 0;

const float TensionMin = 3.2;
const float TensionMax = 4.2;

float batteryLevel;
int batteryValue;
int batteryMinValue;
int batteryMaxValue;

int humiditySensorValue;
int lightSensorValue;
int waterSensorValue;
float atmosSensorValue;
int tempSensorValue;
int altSensorValue;

int activateByHumidity;
int activateByLight;
int activateByWater;
int activateByAtmos;
int activateByTemp;

SoftwareSerial BTSerial(RxD, TxD);

bool run = false;

void setup()
{
    Serial.begin(9600);
    pinMode(motorPin, OUTPUT);
    pinMode(batteryPin, INPUT);
    pinMode(humidityPin, INPUT);
    pinMode(lightPin, INPUT);
    pinMode(waterPin, INPUT);
    pinMode(baroPin, INPUT);
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    setupBluetooth();
    Serial.println("Enter 'start' to launch the sensors, 'stop' to stop them.");
    if (!bmp280.init())
    {
        Serial.println("Device error !");
    }
}

void loop()
{
    sensors();
    //  action();
    delay(2000);
}

void sensors()
{
    if (Serial.available() > 0)
    {
        if (Serial.read() == ("start", DEC))
        {
            run = true;
        }
        if (Serial.read() == ("stop", DEC))
        {
            run = false;
        }
    }

    if (run)
    {
        batteryLevel = analogRead(batteryPin);

        batteryMinValue = (1023 * TensionMin) / 5;
        batteryMaxValue = (1023 * TensionMax) / 5;

        batteryLevel = ((batteryLevel - batteryMinValue) / (batteryMaxValue - batteryMinValue)) * 100;

        if (batteryLevel > 100)
        {
            batteryLevel = 100;
        }
        else if (batteryLevel < 0)
        {
            batteryLevel = 0;
        }

        batteryValue = batteryLevel;
        Serial.print("Battery Level : ");
        Serial.println(batteryValue);

        humiditySensorValue = analogRead(humidityPin) / 3.3;
        Serial.print("Humidity Sensor Value : ");
        Serial.print(humiditySensorValue);
        Serial.println(" %");
        if (humiditySensorValue <= humidityWantedValue)
        {
            activateByHumidity = 1;
            //    Serial.println("Warning, the humidity is inferior than wanted, watering may enable !");
        }

        lightSensorValue = analogRead(lightPin);
        Serial.print("Light Sensor Value : ");
        Serial.print(lightSensorValue);
        Serial.println(" lux");
        if (lightSensorValue >= lightWantedValue)
        {
            activateByLight = 1;
            //    Serial.println("Warning, the light is superior than wanted, watering may enable !");
        }

        waterSensorValue = digitalRead(waterPin);
        Serial.print("Ground humidity Value : ");
        Serial.println(waterSensorValue);
        if (waterSensorValue <= waterWantedValue)
        {
            activateByWater = 1;
            //    Serial.println("Warning, the ground humidity is inferior than wanted, watering may enable !");
        }

        //atmosSensorValue = getPressure();
        atmosSensorValue = bmp280.getPressure();
        Serial.print("Atmospheric pressure Value : ");
        Serial.print(atmosSensorValue);
        Serial.println(" Pa");
        if (atmosSensorValue == atmosWantedValue)
        {
            activateByAtmos = 1;
            //    Serial.println("Warning, the atmospheric pressure is equal than wanted, watering may enable !");
        }

        tempSensorValue = bmp280.getTemperature();
        Serial.print("Temperature Value : ");
        Serial.print(tempSensorValue);
        Serial.println(" C");
        if (tempSensorValue >= tempWantedValue)
        {
            activateByAtmos = 1;
            //    Serial.println("Warning, the temperature is superior than wanted, watering may enable !");
        }

        bluetoothSend();
    }
}

void action()
{
    if (run)
    {
        if (activateByHumidity & activateByLight & activateByWater & activateByAtmos & activateByTemp)
        {
            Serial.println("Warning, enabling watering !");
            BTSerial.println("OIWater=1");
            digitalWrite(motorPin, 1);
        }
        else
        {
            Serial.println("Warning, disabling watering !");
            BTSerial.println("OIWater=0");
            digitalWrite(motorPin, 0);
        }
    }
}

void bluetoothSend()
{
    if (BTSerial.available())
    {
        Serial.println("Received Bluetooth");

        Serial.println("Sending Bluetooth TemperatureValue");
        BTSerial.println("temp=" + String(tempSensorValue));
        delay(1000);
        Serial.println("Sending Bluetooth HumidityLevelValue");
        BTSerial.println("humidity=" + String(humiditySensorValue));
        delay(1000);
        Serial.println("Sending Bluetooth WaterLevelValue");
        BTSerial.println("water=" + String(waterSensorValue));
        delay(1000);
        Serial.println("Sending Bluetooth BatteryLevelValue");
        BTSerial.println("battery=" + String(batteryValue));
        delay(1000);
        Serial.println("Sending Bluetooth LightLevelValue");
        BTSerial.println("light=" + String(lightSensorValue));
        delay(1000);
        Serial.println("Sending Bluetooth AtmosphereLevelValue");
        BTSerial.println("atmos=" + String(atmosSensorValue));
        delay(1000);
        Serial.println("Sending Bluetooth AltLevelValue");
        BTSerial.println("alt=" + String(altSensorValue));
        delay(1000);
    }
}

void setupBluetooth()
{
    BTSerial.begin(9600);

    BTSerial.print("AT+DEFAULT");
    delay(2000);

    BTSerial.print("AT+ROLEM");
    delay(400);

    BTSerial.print("AT+NAMESeeedBTMaster");
    delay(400);

    BTSerial.print("AT+PIN0000");
    delay(400);

    BTSerial.print("AT+AUTH1");
    delay(400);

    BTSerial.flush();

    if (BTSerial.available())
    {
        Serial.println("Bluetooth ON");
    }
    else
    {
        Serial.println("Error Connexion Bluetooth");
    }
}