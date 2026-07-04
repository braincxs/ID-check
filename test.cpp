#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h> 
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// --- Pin Definitions ---
#define SS_PIN 5      
#define RST_PIN 22    
#define buz 13
#define gled 27 
#define rled 26
#define BUTTON_PIN 12 // Button to toggle modes (ESP32 Boot button)

// --- WiFi & Network Settings ---
#define WIFI_SSID "BEEHIVE1_2.4GHz"
#define WIFI_PASSWORD "@Ybee2226"
const String sheet_url = "https://script.google.com/macros/s/AKfycbwNoYyE0K9XKObMk___9VwN2u4YWFE3D8cTKLQMms4BtEfsBRUfgZTdbGxAYVULgzeP/exec?name=";

// --- Hardware Instances ---
MFRC522 mfrc522(SS_PIN, RST_PIN);
TFT_eSPI tft = TFT_eSPI(); 
MFRC522::MIFARE_Key key;

// --- Application Variables ---
byte authorizedUID[4] = {0x22, 0xF7, 0x3C, 0x02}; 
int blockNum = 2;
byte readBlockData[18];
byte bufferLen = 18;
byte nameToWrite[16] = {"Kennard"}; // Default name to write in Namer mode

int currentMode = 0; // 0 = Access & Attendance, 1 = Card Namer
bool lastButtonState = HIGH;

// --- UI & Helper Functions ---

void drawStandbyScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  if (currentMode == 0) {
    tft.setTextSize(3);
    tft.setCursor(20, 20);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Mode: ATTENDANCE");
    tft.setCursor(20, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println("Please scan your ID card...");
  } else {
    tft.setTextSize(3);
    tft.setCursor(20, 20);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.println("Mode: CARD NAMER");
    tft.setCursor(20, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println("Scan card to write name.");
  }
}

void yes() {
  digitalWrite(buz, HIGH);
  digitalWrite(gled, HIGH);
  delay(200);
  digitalWrite(buz, LOW);
  delay(100);
  digitalWrite(buz, HIGH);
  delay(200);
  digitalWrite(buz, LOW);
  digitalWrite(gled, LOW);
}

void no() {
  digitalWrite(buz, HIGH);
  digitalWrite(rled, HIGH);
  delay(700);  
  digitalWrite(buz, LOW);
  digitalWrite(rled, LOW);
}

// --- RFID Read/Write Functions ---

bool ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  MFRC522::StatusCode status;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Auth failed for Read");
    return false;
  }
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Reading failed");
    return false;
  }
  return true;
}

bool WriteDataToBlock(int blockNum, byte blockData[]) {
  MFRC522::StatusCode status;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Auth failed for Write");
    return false;
  }
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Writing failed");
    return false;
  }
  return true;
}

// --- Network Functions ---

void sendToGoogleSheet(String cardHolderName) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL certificate check
    client.setTimeout(15000); // CRITICAL: Give Google 15 seconds to reply
    
    // Format the name for a URL (replace spaces with web-safe %20)
    cardHolderName.replace(" ", "%20"); 
    String url = sheet_url + cardHolderName;
    
    Serial.println("\n--- Starting Cloud Sync ---");
    Serial.println("URL: " + url);
    
    HTTPClient https;
    // CRITICAL: Force the ESP32 to follow Google's 302 Redirects
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
    
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 100);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println("Syncing to Cloud...");
    
    if (https.begin(client, url)) {
      int httpCode = https.GET();
      Serial.printf("HTTP GET Code: %d\n", httpCode);
      
      if (httpCode > 0) {
        // 200 = OK, 302 = Found/Redirected (Both are successes for Google)
        if (httpCode == HTTP_CODE_OK || httpCode == 302) {
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.println("Sync Successful!");
        } else {
          tft.setTextColor(TFT_ORANGE, TFT_BLACK);
          tft.print("Error Code: ");
          tft.println(httpCode);
        }
      } else {
        Serial.printf("HTTPS Request Failed: %s\n", https.errorToString(httpCode).c_str());
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("Sync Failed.");
      }
      https.end();
    } else {
      Serial.println("Unable to establish HTTPS connection to Google.");
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Connection Error");
    }
  } else {
    Serial.println("WiFi is disconnected. Cannot sync.");
  }
}

// --- Main Setup ---

void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  pinMode(buz, OUTPUT);
  pinMode(gled, OUTPUT);
  pinMode(rled, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize TFT
  tft.init();
  tft.setRotation(1); // Landscape
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println("Connecting to WiFi...");
  
  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi connected.");
  
  // Initialize RFID
  mfrc522.PCD_Init();
  delay(10);
  
  drawStandbyScreen();
}

// --- Main Loop ---

void loop() {
  // 1. Check Mode Button (Always check this first)
  bool btnState = digitalRead(BUTTON_PIN);
  if (btnState == LOW && lastButtonState == HIGH) {
    currentMode = (currentMode == 0) ? 1 : 0; 
    drawStandbyScreen();
    delay(200); 
  }
  lastButtonState = btnState;

  // 2. Wait for RFID Card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return; // Stop here if no card is present
  }

  // 3. Execute Mode Logic
  if (currentMode == 0) {
    // --- MODE 0: ACCESS & ATTENDANCE ---
    bool isAuthorized = true; 
    String scannedUID = ""; 

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (i < 4 && mfrc522.uid.uidByte[i] != authorizedUID[i]) {
        isAuthorized = false; 
      }
      scannedUID += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      scannedUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    if (isAuthorized) {
      bool readSuccess = ReadDataFromBlock(blockNum, readBlockData);
      tft.fillScreen(TFT_BLACK);
      yes();
      
      if (readSuccess) {
        char cleanData[17]; 
        memcpy(cleanData, readBlockData, 16);
        cleanData[16] = '\0'; 
        String holderName = String(cleanData);
        holderName.trim(); 
        
        tft.setCursor(20, 80);
        tft.setTextSize(3);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("ACCESS GRANTED");
        tft.setCursor(20, 140);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println("Hello, " + holderName);
        
        sendToGoogleSheet(holderName);
      } else {
        tft.setCursor(20, 100);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("READ FAILED!");
      }
    } else {
      tft.fillScreen(TFT_BLACK);
      no();
      tft.setCursor(20, 80);
      tft.setTextSize(3);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("ACCESS DENIED");
    }
  } 
  else {
    // --- MODE 1: CARD NAMER ---
    
    // 1. Execute hardware write immediately while card is in stable field
    bool writeSuccess = WriteDataToBlock(blockNum, nameToWrite);
    
    // 2. Update screen AFTER the SPI bus and RFID hardware are done
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 80);
    tft.setTextSize(3);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.println("WRITING...");
    
    if (writeSuccess) {
      yes();
      tft.setCursor(20, 140);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println("SUCCESS!");
    } else {
      no();
      tft.setCursor(20, 140);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("FAIL!");
    }
  }

  // Cleanup
  delay(2000);
  drawStandbyScreen();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}