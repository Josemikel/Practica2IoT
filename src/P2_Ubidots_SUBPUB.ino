#include <UbiConstants.h>
#include <UbidotsEsp32Mqtt.h>
#include <UbiTypes.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "DHT.h"

/* CODIGO JOSÉ MIGUEL VÉLEZ */

#define DHTPIN 27   //pin 27 del ttgo     
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBUS-MhREkiB2dovQplRO9Jr77aisaszzZm";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "VELEZ";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "71621728";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL1 = "Temperatura"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "Humedad";
const char *SUBSCRIBE_DEVICE_LABEL = "esp32";   // Replace with the device label to subscribe to
const char *SUBSCRIBE_VARIABLE_LABEL1 = "switch1"; // Replace with the variable label to subscribe to
const char *SUBSCRIBE_VARIABLE_LABEL2 = "switch2";


const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

unsigned long timer;

const uint8_t LED = 12;

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    if ((char)payload[0] == '1')
    {
      tft.fillCircle(40, 170, 19,TFT_DARKGREEN); //rellenos CAMBIAN CON LOS SWITCH
      digitalWrite(LED, HIGH);
    }
    else if ((char)payload[0] == '0')
    {
      tft.fillCircle(40, 170, 19,TFT_BLACK);
      digitalWrite(LED, LOW);
    }
        else if ((char)payload[0] == '3')
    {
      tft.fillCircle(95, 170, 19,TFT_BLACK);
    }
        else if ((char)payload[0] == '4')
    {
      tft.fillCircle(95, 170, 19,TFT_PURPLE);
    }
  }
  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED, OUTPUT); 
  
  tft.init();    //OLED
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.drawString("Sin conexion a", 15, 100, 2);
  tft.drawString(WIFI_SSID, 20, 120, 2);
  
  
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, SUBSCRIBE_VARIABLE_LABEL1); // Insert the device and variable's Labels, respectively
  ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, SUBSCRIBE_VARIABLE_LABEL2); // Insert the device and variable's Labels, respectively

  timer = millis();
  
  Serial.println(F("DHTxx test!")); //sensor
  dht.begin();
  
  tft.init();    //OLED
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Temperatura", 10, 18, 2);
  tft.drawString("Humedad", 10, 88, 2);
  tft.drawString("`C", 63, 40, 4);
  tft.drawString("%", 63, 110, 4);
  
  tft.drawCircle(40, 170, 20,TFT_GREEN);   //CONTORNOS  
  tft.drawCircle(95, 170, 20,TFT_MAGENTA);


 // tft.fillCircle(40, 170, 19,TFT_DARKGREEN); //rellenos CAMBIAN CON LOS SWITCH
 // tft.fillCircle(95, 170, 19,TFT_PURPLE);
  

}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
   if (ubidots.connected())
  {
  tft.setTextColor(TFT_BLUE);
    tft.drawString("Conectado a", 20, 200, 2);
    tft.drawString(WIFI_SSID, 20, 220, 2);
  }
    
  if ((millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    //float value = analogRead(analogPin);
    float h = dht.readHumidity();
    float t = dht.readTemperature();

  
    if (isnan(h) || isnan(t)) { //revisar errores
      Serial.println(F("ERROR"));
      tft.setTextColor(TFT_RED);
      tft.drawString("ERROR", 10, 45, 2);
      tft.drawString("ERROR", 10, 115, 2);
      return;
    }
  
    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.print(F("%  Temperatura: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.println();

    tft.fillRect(9, 39, 53, 30, TFT_BLACK); //borrar valor anterior
    tft.fillRect(9, 109, 53, 30, TFT_BLACK);
  
    tft.setTextColor(TFT_CYAN);
    tft.drawString(String(t,1), 10, 40, 4);
    tft.drawString(String(h,1), 10, 110, 4);
    
    
    ubidots.add(VARIABLE_LABEL1, t); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL2, h);
    
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
}
