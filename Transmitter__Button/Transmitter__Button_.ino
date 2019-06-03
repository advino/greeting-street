//libraries

#include <SPI.h>
#include <RH_RF69.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(9,8,5,A0,A1,A2);

//frequency setup
#define RF69_FREQ 915.0

#define RFM69_CS 4
#define RFM69_INT 3
#define RFM69_RST 2
#define LED 6
#define PRESENTLED 7
#define BUTTON 10

int buttonState = 0;

boolean userReady = false;
boolean otherUser = false;
boolean messageSent = false;

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;

void setup() {
  //  set serial
  Serial.begin(115200);

  lcd.begin(16,2);
  lcd.print("Hello, press the");
  lcd.setCursor(0,1);
  lcd.print("button to begin!");
  delay(2000);
  lcd.clear();
  //pin assignments
  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  pinMode(PRESENTLED, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(RFM69_RST, LOW);

  //Print for confirming transmitter is on
  Serial.println("Button Transmitter Test");
  Serial.println();

  //manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  //notify via serial if initialize fails
  if (!rf69.init()) {
    Serial.println("RFM69 failed to initialize");
    while (1);
  }

  //notify if initilization succeeds
  Serial.println("RFM69 initialized");

  //notify if frequency is not set
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("RFM69 failed to set frequency");
  }

  rf69.setTxPower(20, true);

  pinMode(LED, OUTPUT);


  Serial.println("RFM69 radio @");
  Serial.println((int)RF69_FREQ);
  Serial.println(" MHz");


}

void loop() {
  
  char radiopacket[1] = "X";
  buttonState = digitalRead(BUTTON);
//  Serial.println(switchState);


  //  controls print message
  if (buttonState == LOW) {

    if (messageSent == false) {
      //    itoa(packetnum++, radiopacket+13, 10);
      Serial.println("Sending "); Serial.println(radiopacket);
      userReady = true;

      //    send message
      rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
      rf69.waitPacketSent();

      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
      messageSent = true;
      lcd.print("Message Sent"); 
      lcd.setCursor(0,1);
      lcd.print("Wating for reply");
    }

    }

  
  //  controls receiving messages and responses

  if (rf69.available()) {
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]");
      Serial.println((char*)buf);
      Serial.println("RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);

      if (strstr((char *)buf, "Y")) {
        digitalWrite(PRESENTLED, HIGH);
        otherUser = true;
        lcd.clear();
        lcd.print("Incoming Message");
      }
    }



  }

  if (userReady && otherUser == true) {
    lcd.clear();
    lcd.print("Give the person");
    lcd.setCursor(0,1);
    lcd.print("a high-five!");
    delay(3000);
    userReady = false;
    otherUser = false;
    messageSent = false;
    digitalWrite(PRESENTLED, LOW);
    delay(2000);
    lcd.clear();
  }
}




