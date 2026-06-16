#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h> 

// RFID Pins
#define SS_PIN 5      
#define RST_PIN 22    
#define buz 13
#define gled 27 
#define rled 26

MFRC522 mfrc522(SS_PIN, RST_PIN);
TFT_eSPI tft = TFT_eSPI(); 

// Replace these 4 bytes with your specific card's UID!
byte authorizedUID[4] = {0x3E,0xBF,0x99,0x92}; 

void drawStandbyScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 20);
  tft.println("Security System Active");
  tft.setCursor(20, 80);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.println("Please scan your ID card...");
}



void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(buz,OUTPUT);
  pinMode(gled,OUTPUT);
  pinMode(rled,OUTPUT);
  // 1. Initialize the TFT Screen
  tft.init();
  tft.setRotation(1); // 1 = Landscape
  
  // Draw the standby screen
  drawStandbyScreen();

  // 2. Initialize the RFID Reader
  mfrc522.PCD_Init();
  delay(10);
  mfrc522.PCD_DumpVersionToSerial();
}

void yes(){
    digitalWrite(buz,HIGH);
    digitalWrite(gled,HIGH);
    delay(200);
    digitalWrite(buz,LOW);
    delay(100);
    digitalWrite(buz,HIGH);
    delay(200);
    digitalWrite(buz,LOW);
    digitalWrite(gled,LOW);
}
void no(){
    digitalWrite(buz,HIGH);
    digitalWrite(rled,HIGH);
    delay(700);  
    digitalWrite(buz,LOW);
    digitalWrite(rled,LOW);
}

void loop() {
  // Wait until a new card is scanned
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Assume the card matches until proven otherwise
  bool isAuthorized = true; 
  String scannedUID = ""; // We will use this to save the ID as text

  // Compare the scanned card's UID to your authorized UID and build the text string
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    // Check for authorization
    if (i < 4 && mfrc522.uid.uidByte[i] != authorizedUID[i]) {
      isAuthorized = false; // A byte didn't match!
    }
    
    // Add the current byte to our readable string
    scannedUID += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    scannedUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  
  scannedUID.toUpperCase();
  scannedUID.trim(); // Removes the extra space at the beginning

  // Clear the screen to prepare for the message
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(60, 120); // Center-ish of a 480x320 screen

  // Display the result
  if (isAuthorized) {
    tft.setTextSize(4);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("ACCESS GRANTED");
    yes();
    Serial.println("Access Granted");

  } else {
    // Print "ACCESS DENIED"
    tft.setTextSize(4);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("ACCESS DENIED");
    
    // Move the cursor down and print the wrong UID
    tft.setCursor(60, 180); 
    tft.setTextSize(3); // Make the ID font slightly smaller so it fits
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("ID: ");
    tft.println(scannedUID);
    
    // Also print it to the Serial Monitor for easy copying
    Serial.print("Access Denied. Scanned UID: ");
    Serial.println(scannedUID);
    no();
  }

  // Keep the message on screen for 3 seconds so you have time to read it
  delay(1000); 

  // Reset back to the standby screen
  drawStandbyScreen();

  // Tell the RFID reader to stop reading this specific card to prevent spamming
  mfrc522.PICC_HaltA();
}//grey, purple white grey2 black green brown, red brown
