#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Wifi lokal configuration.

const char* ssid = "Greenhouse";
const char* password = "12345678";

// MQTT broker configuration
const char* mqtt_server = "192.168.1.2"; // host 
const char* mqttUser = "test"; // user mqtt : "" dan untuk yg gh user : test )
const char* mqttPass = "test"; // pass mqtt : "" dan untuk yg gh pass : test )

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int relay = 15;
int buz = 0;

// ---------------------------------------- Setup Wifi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // ----------------------------------  setup OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  // -------------------------------------------------
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// ------------------------------ MQTT respon Subscribe dan eksekusi LED
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Nyalakan LED jika value = 1 diterima as first character
  if ((char)payload[0] == '1') {
    digitalWrite(relay, HIGH);   // indikator lampu LED akan nyala karena aktive low 
  } else {
    digitalWrite(relay, LOW);  // Turn the LED off by making the voltage HIGH
  }

}

// ---------------------------- MQTT Subscribe dengan topik 'InTopic' silahkan diganti topik menyesuaikan 
// ---------------------------- otomatis rekonek jika putus koneksi ke MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPass )) {
      Serial.println("connected");
      // pemberitahuan pertama kalo mqtt terkoneksi 
      digitalWrite(buz, HIGH);
      delay (200);
      digitalWrite(buz, LOW);
      delay (250);
      digitalWrite(buz, HIGH);
      delay (100);
      digitalWrite(buz, LOW);
      client.publish("outTopic", "berhasil publish ex1");
     // client.publish("outTopic", "berhasil publish ex1");
      // ... and resubscribe
      client.subscribe("gh/ex1");  // diganti dengan topiknya misal gh/sh1 dst
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ------------------------- setup inisiasi program
void setup() {
  pinMode(relay, OUTPUT);     
  pinMode(buz, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);    // eksekusi fungsi callback atau subscribe
  
}

// -------------------------- program utama
void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
