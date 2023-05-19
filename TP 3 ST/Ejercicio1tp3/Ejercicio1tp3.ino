#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>

const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

#define BOTtoken "6235430638:AAGYaEncOpMHo7J3kIw5olG0G2n1nZYe9t4" 
#define CHAT_ID "1838958839"

#define OLED_RESET 4      // Pin de reset para el display OLED
#define SCREEN_WIDTH 128  // Ancho del display OLED
#define SCREEN_HEIGHT 32  // Alto del display OLED5
#define DHTPIN 23   // Pin del sensor DHT11
#define SW1_PIN 34  // Pin del botón SW1
#define SW2_PIN 35  // Pin del botón SW2
#define DHTTYPE DHT11

#define PANTALLA_PRINCIPAL 1    //estados de la maquina
#define ESPERA_CAMBIAR 2
#define PANTALLA_CAMBIAR 3
#define ESPERA_PRINCIPAL 4

int botRequestDelay = 1000; /// intervalo
int displayRequestDelay = 200;
unsigned long lastTimeBotRan; /// ultimo tiempo
unsigned long lastTimeDisplayRan;
float temp;
String tempa;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  int current_screen = 1;
  float temp_umbral = 28;
  bool flagSubir, flagBajar;
  
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void setup() {
  
  Serial.begin(9600);
  dht.begin();
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {   
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot Hola mundo", "");
}



void loop() {

  temp = dht.readTemperature();
 
  switch (current_screen){

    case PANTALLA_PRINCIPAL:
{
   if (millis() > lastTimeDisplayRan + displayRequestDelay) {
    lastTimeDisplayRan = millis();
    temp = dht.readTemperature();
    Serial.print(temp);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(temp);
    display.println(temp_umbral);
    display.display();
   }
    if (digitalRead(SW1_PIN) == LOW && digitalRead(SW2_PIN) == LOW){
      current_screen = ESPERA_CAMBIAR;
    }
}
        break;

    case ESPERA_CAMBIAR: 
  {
    if(digitalRead(SW1_PIN) == HIGH && digitalRead(SW2_PIN) == HIGH)
      {
        current_screen = PANTALLA_CAMBIAR;
      }
  }

        break;

    case PANTALLA_CAMBIAR:
{

  if (millis() > lastTimeDisplayRan + displayRequestDelay) {
    lastTimeDisplayRan = millis();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(temp_umbral);
    display.println("SW1 para -");
    display.println("SW2 para +");
    display.display();
  }
    if(digitalRead(SW2_PIN) == LOW) 
      {
        flagBajar = HIGH;
      }
      if(digitalRead(SW2_PIN) == HIGH && flagBajar == HIGH) 
      {
        flagBajar = LOW;                   
        temp_umbral = temp_umbral -1;
      }    

      if(digitalRead(SW1_PIN) == LOW) 
      {
        flagSubir = HIGH;
      }
      if(digitalRead(SW1_PIN) == HIGH && flagSubir == HIGH)
      {
        flagSubir = LOW;
         temp_umbral = temp_umbral +1;
      }    
      
      if(digitalRead(SW1_PIN) == LOW && digitalRead(SW2_PIN) == LOW)  
      {
        current_screen = ESPERA_PRINCIPAL;
      }
}
     break; 

    case ESPERA_PRINCIPAL:
    {
      if(digitalRead(SW1_PIN) == HIGH && digitalRead(SW2_PIN) == HIGH)
      {
        current_screen = PANTALLA_PRINCIPAL;
      }
    }

      break;
  }  
 

  if (millis() > lastTimeBotRan + botRequestDelay) {
     lastTimeBotRan = millis();
  
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("Veo los msj nuevos");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
   
  }

}

void handleNewMessages(int numNewMessages) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    
    ///fin de verificacion

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/temp") {  
      
      temp = dht.readTemperature();
      tempa = temp;
      Serial.println("Message recieved. Temperature is: " + tempa);
      bot.sendMessage(chat_id, "Temperature is: " + tempa, "");

     
    }
      
      if(temp >= temp_umbral){

    bot.sendMessage(chat_id, "too hot daddy", "");

  }
    
  }


}
