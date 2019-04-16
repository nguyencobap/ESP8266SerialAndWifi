#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <MQ135.h>
#include <DHT.h>
#include <string.h>
SocketIOClient client;
#define QUOTE "\""
const int DHTPIN = 4;
const int DHTTYPE = DHT11;
const int PIN_MQ135 = 17;

const int led1 = 5;
const int led3 = 0;
const int led4 = 2;
const int led5 = 14;
const int led6 = 12;
const int led7 = 13;

MQ135 mq135_sensor = MQ135(PIN_MQ135);

char ssid[] = "PASS 12345678";
char password[] = "toluaday";
extern String RID;
extern String Rfull;
String ON = "ON";
String OFF = "OFF";
int ledState;
int ledNum;
int serialInt;

const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
DynamicJsonDocument  doc(bufferSize);
String stateLight;
WiFiServer wifiServer(80);


String ts, hs, content, ps;
DHT dht(DHTPIN, DHTTYPE);
char host[] = "thmnt.ml";
int port = 80;


void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);

  Serial.begin(115200);
  delay(10);
  pinMode(DHTPIN, INPUT);
  dht.begin();
  delay(10);

  Serial.println("Ket noi vao mang: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  Serial.println();
  while (WiFi.status() != WL_CONNECTED) {
    delay(700);
    Serial.print(".");
  }
  Serial.println(("Da ket noi WiFi"));
  Serial.print(WiFi.localIP());

  if (!client.connect(host, port)) {
    Serial.println("Ket noi den socket server that bai!");
    return;
  }

  if (client.connected()) {
    Serial.println("Ket noi thanh cong den server");
  }

  wifiServer.begin();
}

//void lightControlWifi() {
//  if (WFclient.available()) {
//    serialInt = WFclient.parseInt();
//    Serial.write(serialInt);
//  }
//  if (ledState == 1) {
//    if (ledNum == 1)digitalWrite(led1, HIGH);
//
//    if (ledNum == 3)digitalWrite(led3, HIGH);
//    if (ledNum == 4)digitalWrite(led4, HIGH);
//    if (ledNum == 5)digitalWrite(led5, HIGH);
//    if (ledNum == 6)digitalWrite(led6, HIGH);
//    if (ledNum == 7)digitalWrite(led7, HIGH);
//  } else {
//    if (ledNum == 1)digitalWrite(led1, LOW);
//
//    if (ledNum == 3)digitalWrite(led3, LOW);
//    if (ledNum == 4)digitalWrite(led4, LOW);
//    if (ledNum == 5)digitalWrite(led5, LOW);
//    if (ledNum == 6)digitalWrite(led6, LOW);
//    if (ledNum == 7)digitalWrite(led7, LOW);
//  }
//}

void lightControlSerial() {
  if (Serial.available() > 0) {
    serialInt = Serial.parseInt();
  }
  ledState = serialInt % 10;
  ledNum = serialInt / 10;
  if (ledState == 1) {
    if (ledNum == 1)digitalWrite(led1, HIGH);

    if (ledNum == 3)digitalWrite(led3, HIGH);
    if (ledNum == 4)digitalWrite(led4, HIGH);
    if (ledNum == 5)digitalWrite(led5, HIGH);
    if (ledNum == 6)digitalWrite(led6, HIGH);
    if (ledNum == 7)digitalWrite(led7, HIGH);
  } else {
    if (ledNum == 1)digitalWrite(led1, LOW);

    if (ledNum == 3)digitalWrite(led3, LOW);
    if (ledNum == 4)digitalWrite(led4, LOW);
    if (ledNum == 5)digitalWrite(led5, LOW);
    if (ledNum == 6)digitalWrite(led6, LOW);
    if (ledNum == 7)digitalWrite(led7, LOW);
  }
}


void measureTH() {
  float ppm = mq135_sensor.getPPM();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = mq135_sensor.getCorrectedPPM(t, h);

  ps = String(p);
  ts = String(t);
  hs = String(h);

  Serial.print(ts);
  Serial.print(",");
  Serial.print(hs);
  Serial.print(",");
  Serial.print(ps);
  Serial.println();
}


void loop()
{
  //  digitalWrite(led5, HIGH);
  WiFiClient WFclient = wifiServer.available();
  if (WFclient) {
    while (WFclient.connected()) {
      measureTH();
      WFclient.println(ts + "," + hs + "," + ps);
      content = "{"QUOTE"temp"QUOTE":" + ts + ","QUOTE"humi"QUOTE":" + hs + ","QUOTE"ppm"QUOTE":" + ps + "}";
      client.send("db", content);
      delay(500);

    }
    WFclient.stop();
    Serial.println("Client disconnected");
  } else {
    if (client.monitor()) {
      deserializeJson(doc, Rfull);
      JsonObject obj = doc.as<JsonObject>();
      stateLight = obj["state"].as<String>();;
      Serial.println(stateLight);
    }
    if ( stateLight == "true") {
      digitalWrite(led7, HIGH);
    } if (stateLight == "false") {
      digitalWrite(led7, LOW);
    }

    if (digitalRead(led7) == 1) {
      String statepin7 = "{"QUOTE"ledState"QUOTE":true}";
      Serial.println("ON");
      client.send("toggle", statepin7);
    }
    else {
      String statepin7 = "{"QUOTE"ledState"QUOTE":false}";
      Serial.println("OFF");
      client.send("toggle",statepin7 );
    }

    lightControlSerial();
    measureTH();
    content = "{"QUOTE"temp"QUOTE":" + ts + ","QUOTE"humi"QUOTE":" + hs + ","QUOTE"ppm"QUOTE":" + ps + "}";
    client.send("db", content);
    delay(500);
  }





  //  Serial.println(content);


  if (!client.connected()) {
    Serial.println("ngat ket noi");


    client.reconnect(host, port);
    if (client.connected()) {
      Serial.println("Da ket noi lai voi server");
    }
  }
}
