#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <DHT.h>
#include <string.h>
SocketIOClient client;
#define QUOTE "\""
const int DHTPIN = 2;
const int DHTTYPE = DHT11;

char ssid[] = "hayday";
char password[] = "Van@#The1981";

extern String RID;
extern String Rfull;

WiFiServer wifiServer(80);

String ts, hs, content;
DHT dht(DHTPIN, DHTTYPE);
char host[] = "192.168.100.12";
int port = 80;
void setup()
{

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


void measureTH() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //  ts = String(random(0, 100));
  //  hs = String(random(0, 100));

  ts = String(t);
  hs = String(h);

  Serial.print(ts);
  Serial.print(",");
  Serial.print(hs);
  Serial.println();
}


void loop()
{

  WiFiClient WFclient = wifiServer.available();
  if (WFclient) {
    while (WFclient.connected()) {
      measureTH();
      WFclient.println(ts + "," + hs);
      content = "{"QUOTE"temp"QUOTE":" + ts + ","QUOTE"humi"QUOTE":" + hs + "}";
      client.send("db", content);
      delay(5000);
      while (WFclient.available()) {
        char c = WFclient.read();
        Serial.write(c);
      }
    }
    WFclient.stop();
    Serial.println("Client disconnected");
  } else {
    measureTH();
    content = "{"QUOTE"temp"QUOTE":" + ts + ","QUOTE"humi"QUOTE":" + hs + "}";
    client.send("db", content);
    delay(5000);
  }

  if(client.monitor()){
      Serial.println(RID);
      Serial.println(Rfull);
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
