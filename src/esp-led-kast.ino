#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT Subscribes
const char* SubscribeTo = "esp/led-kast/out";
const char* SubscribeFrom = "esp/led-kast/in";

// WiFi & MQTT Server
const char* ssid = "MySSID";
const char* password = "MyPassWD";
const char* mqtt_server = "192.168.1.10";
const char* mqttUser = "mqttUser";
const char* mqttPassword = "mqttPasswd";

WiFiClient espClient;
PubSubClient pubClient(espClient);
long lastMsg = 0;
char msg[50];
String message("#000000");
String lastMessage("#000000");

// Neopixel Config
#define NeoPIN D4
#define NUM_LEDS 180
int brightness = 150;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_RGB + NEO_KHZ800);

void setup() {
  Serial.begin ( 115200 );

  // ##############
  // NeoPixel start
  Serial.println();
  strip.setBrightness(brightness);
  strip.begin();
  strip.show();
  delay(50);

  // WiFi
  setup_wifi();

  // MQTT
  // connecting to the mqtt server
  pubClient.setServer(mqtt_server, 1883);
  pubClient.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String color("#");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if(i > 0){
      color = color + String((char)payload[i]);
    }
  }

  // finding payload
  if((char)payload[0] == '#'){
    // setting color
     setNeoColor(color);
  }

}

void setup_wifi() {

  // Static IP details...
  IPAddress ip(192, 168, 84, 150);
  IPAddress gateway(192, 168, 84, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);

  wifi_station_set_hostname("EspNeopixelKast");

  delay(10);
  // We start by connecting to a WiFi network

  WiFi.config(ip, gateway, subnet, dns); //If you need Internet Access You should Add DNS also...
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
}

void loop() {
  if (!pubClient.connected()) {
    delay(100);
    reconnect();
  }
  pubClient.loop();

  long now = millis();
  if (now - lastMsg > 60000 || lastMessage.indexOf(message) < 0 ) {

    lastMsg = now;
    char msg[7];
    message.toCharArray(msg, message.length());
      pubClient.publish(SubscribeTo, msg);
      lastMessage = message;
  }
  delay(10);
}

void setNeoColor(String value){
    message = value;
    int number = (int) strtol( &value[1], NULL, 16);

    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

   for(int i=0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color( g, r, b ) );
    }
    strip.show();
}

void reconnect() {
  // Loop until we're reconnected
  while (!pubClient.connected()) {
    // Attempt to connect
    if (pubClient.connect("ESP8266Client", mqttUser, mqttPassword )) {
      // Once connected, publish an announcement...
      pubClient.publish(SubscribeTo, "Restart");
      // ... and resubscribe
      pubClient.subscribe(SubscribeFrom);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
