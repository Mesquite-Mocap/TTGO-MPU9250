#include "MPU9250.h"
#include "ttgo.h"
#include <TFT_eSPI.h>

#include <pcf8563.h>
//#include <WiFiManager.h>
#include <WiFi.h> 
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
// #include <WebSocketsClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

MPU9250 mpu;
TFT_eSPI tft = TFT_eSPI(); 
PCF8563_Class rtc; 
//WiFiManager wifiManager;
// WebSocketsClient webSocket;
BLECharacteristic* pCharacteristic;

struct Quat {
    float x;
    float y;
    float z;
    float w;
} quat;
struct Euler {
    float x;
    float y;
    float z;
} euler;
char buff[256];
bool rtcIrq = false;
bool initial = 1;
bool otaStart = false;

uint8_t func_select = 0;
uint8_t omm = 99;
uint8_t xcolon = 0;
uint32_t targetTime = 0;       // for next 1 second timeout
uint32_t colour = 0;
int vref = 1100;

bool pressed = false;
uint32_t pressedTime = 0;
bool charge_indication = false;

uint8_t hh, mm, ss ;
const char* host = "esp32-9";
const char* ssid = "SETUP-AE05";
const char* password = "faucet4039dozed";
String serverIP = "mocap.local";
String mac_address;
WebServer server(80);


#define TP_PIN_PIN          33
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22
#define IMU_INT_PIN         38
#define RTC_INT_PIN         34
#define BATT_ADC_PIN        35
#define VBUS_PIN            36
#define TP_PWR_PIN          25
#define LED_PIN             4
#define CHARGE_PIN          32

//maintain compatability with HM-10
#define BLE_NAME "ESP32" //must match filters name in bluetoothterminal.js- navigator.bluetooth.requestDevice
// BLEUUID  SERVICE_UUID((uint16_t)0x1802); // UART service UUID
// BLEUUID CHARACTERISTIC_UUID ((uint16_t)0x1803);

BLEUUID  SERVICE_UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART service UUID
BLEUUID CHARACTERISTIC_UUID ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");


/*

* Login page

*/

const char* loginIndex =

"<form name='loginForm'>"

    "<table width='20%' bgcolor='A09F9F' align='center'>"

        "<tr>"

            "<td colspan=2>"

                "<center><font size=4><b>ESP32 Login Page</b></font></center>"

                "<br>"

            "</td>"

            "<br>"

            "<br>"

        "</tr>"

        "<tr>"

             "<td>Username:</td>"

             "<td><input type='text' size=25 name='userid'><br></td>"

        "</tr>"

        "<br>"

        "<br>"

        "<tr>"

            "<td>Password:</td>"

            "<td><input type='Password' size=25 name='pwd'><br></td>"

            "<br>"

            "<br>"

        "</tr>"

        "<tr>"

            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"

        "</tr>"

    "</table>"

"</form>"

"<script>"

    "function check(form)"

    "{"

    "if(form.userid.value=='admin' && form.pwd.value=='admin')"

    "{"

    "window.open('/serverIndex')"

    "}"

    "else"

    "{"

    " alert('Error Password or Username')/*displays error message*/"

    "}"

    "}"

"</script>";

/*

* Server Index Page

*/

const char* serverIndex =

"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"

"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"

   "<input type='file' name='update'>"

        "<input type='submit' value='Update'>"

    "</form>"

"<div id='prg'>progress: 0%</div>"

"<script>"

  "$('form').submit(function(e){"

  "e.preventDefault();"

  "var form = $('#upload_form')[0];"

  "var data = new FormData(form);"

  " $.ajax({"

  "url: '/update',"

  "type: 'POST',"

  "data: data,"

  "contentType: false,"

  "processData:false,"

  "xhr: function() {"

  "var xhr = new window.XMLHttpRequest();"

  "xhr.upload.addEventListener('progress', function(evt) {"

  "if (evt.lengthComputable) {"

  "var per = evt.loaded / evt.total;"

  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"

  "}"

  "}, false);"

  "return xhr;"

  "},"

  "success:function(d, s) {"

  "console.log('success!')"

  "},"

  "error: function (a, b, c) {"

  "}"

  "});"

  "});"

"</script>";

// void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
//   const uint8_t* src = (const uint8_t*) mem;
//   Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
//   for(uint32_t i = 0; i < len; i++) {
//     if(i % cols == 0) {
//       Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
//     }
//     Serial.printf("%02X ", *src);
//     src++;
//   }
//   Serial.printf("\n");
// }

// void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
//   switch(type) {
//     case WStype_DISCONNECTED:  //when disconnected
//       Serial.printf("[WSc] Disconnected!\n");
//       break;
//     case WStype_CONNECTED: //when connected
//       Serial.printf("[WSc] Connected to url: %s\n", payload);

//       // send message to server when Connected
//       webSocket.sendTXT("Connected"); //validation that you've connected
//       break;
//     case WStype_TEXT: //when you get a message
//       Serial.printf("[WSc] get text: %s\n", payload);

//       // send message to server
//       // webSocket.sendTXT("message here");
//       break;
//     case WStype_BIN:
//       Serial.printf("[WSc] get binary length: %u\n", length);
//       hexdump(payload, length);

//       // send data to server
//       // webSocket.sendBIN(payload, length);
//       break;
//     case WStype_ERROR:
//     case WStype_FRAGMENT_TEXT_START:
//     case WStype_FRAGMENT_BIN_START:
//     case WStype_FRAGMENT:
//     case WStype_FRAGMENT_FIN:
//       break;
//   }
// }

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        // Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
        {
          Serial.print(value[i]);
        }

        // Serial.println();
        // Serial.println("*********");

        pCharacteristic->setValue(value +"\n"); // must add seperator \n for it to register on BLE terminal
        pCharacteristic->notify();
      }
    }
};

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Serial.println("");
    // Serial.print("Connected to ");
    // Serial.println(ssid);
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());

    // Serial.println(mac_address);
    mac_address = WiFi.macAddress();

    /*use mdns for host name resolution*/

    if (!MDNS.begin(host)) { //http://esp32.local
      // Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }

    // Serial.println("mDNS responder started");

    /*return index page which is stored in serverIndex */

    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", loginIndex);
    });

    server.on("/serverIndex", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });

    /*handling uploading firmware file */

    server.on("/update", HTTP_POST, []() {

      server.sendHeader("Connection", "close");

      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");

      ESP.restart();

    }, []() {

      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {

        Serial.printf("Update: %s\n", upload.filename.c_str());

        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size

          Update.printError(Serial);

        }

      } else if (upload.status == UPLOAD_FILE_WRITE) {

        /* flashing firmware to ESP*/

        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {

          Update.printError(Serial);

        }

      } else if (upload.status == UPLOAD_FILE_END) {

        if (Update.end(true)) { //true to set the size to the current progress

          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);

        } else {

          Update.printError(Serial);

        }
      }
    });

    server.begin();

    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    tft.pushImage(0, 0,  160, 80, ttgo);

    BLEDevice::init(BLE_NAME);
    BLEServer *pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE |
                                          BLECharacteristic::PROPERTY_NOTIFY
                                        );

    pCharacteristic->setCallbacks(new MyCallbacks());
    
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();

    Wire.begin();
    delay(2000);

    if (!mpu.setup(0x69)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // calibrate anytime you want to
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Accel Gyro calibration - 5sec.",  20, tft.height() / 2 );
    Serial.println("Accel Gyro calibration will start in 5sec.");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Leave on the flat plane",  20, tft.height() / 2 );
    Serial.println("Please leave the device still on the flat plane.");
    mpu.verbose(true);
    delay(5000);
    mpu.calibrateAccelGyro();

    tft.fillScreen(TFT_BLACK);
    tft.drawString("Mag calibration in 5sec",  20, tft.height() / 2 );
    Serial.println("Mag calibration will start in 5sec.");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Wave device in eight",  20, tft.height() / 2 );
    Serial.println("Please Wave device in a figure eight until done.");
    delay(5000);
    mpu.calibrateMag();

    // print_calibration();
    mpu.verbose(false);

    // webSocket.begin(serverIP, 3000, "/hub");
    // // event handler
    // webSocket.onEvent(webSocketEvent);
    // // use HTTP Basic Authorization this is optional remove if not needed
    // // webSocket.setAuthorization("user", "Password");

    // // try ever 5000 again if connection has failed
    // webSocket.setReconnectInterval(5000);
    // webSocket.sendTXT(String(millis()).c_str());
}

void loop() {
    // Serial.println(pServer->getConnectedCount());

    IMU_Show();
    server.handleClient();  
    // delay(1);
    // webSocket.loop();

    String url = "{\"id\": \"" + mac_address + "\",\"x\":" + quat.x + ",\"y\":" + quat.y + ",\"z\":" + quat.z +  ",\"w\":" + quat.w + "}";
    // String url = String(quat.x) + " " + quat.y + " " + quat.z +  " " + quat.w;
    Serial.println(url);

    // String message = "Hello, world!";
    pCharacteristic->setValue(url.c_str());

    // Send a notification to connected clients
    pCharacteristic->notify();
    // webSocket.sendTXT(url.c_str());

}

void IMU_Show() {
  if (mpu.update()) {
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 10) {
            print_roll_pitch_yaw();
            prev_ms = millis();
        }
        quat.x = mpu.getQuaternionX();
        quat.y = mpu.getQuaternionY();
        quat.z = mpu.getQuaternionZ();
        quat.w = mpu.getQuaternionW();
        euler.x = mpu.getEulerX();
        euler.y = mpu.getEulerY();
        euler.z = mpu.getEulerZ();
    }
}

void print_roll_pitch_yaw() {
    // Serial.print("Yaw, Pitch, Roll: ");
    // Serial.print(mpu.getYaw(), 2);
    // Serial.print(", ");
    // Serial.print(mpu.getPitch(), 2);
    // Serial.print(", ");
    // Serial.println(mpu.getRoll(), 2);
    
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    // tft.drawString(mac_address,  0, 0, 4);
    // snprintf(buff, sizeof(buff), "%s", mac_address);
    tft.drawString(buff, 0, 0);
    snprintf(buff, sizeof(buff), "--  w   x   y   z");
    tft.drawString(buff, 0, 16);
    snprintf(buff, sizeof(buff), "Q %.2f  %.2f  %.2f  %.2f", quat.w, quat.x, quat.y, quat.z);
    tft.drawString(buff, 0, 32);
    snprintf(buff, sizeof(buff), "E %.2f  %.2f  %.2f", euler.x, euler.y, euler.z);
    tft.drawString(buff, 0, 48);
}

void print_calibration() {
    Serial.println("< calibration parameters >");
    Serial.println("accel bias [g]: ");
    Serial.print(mpu.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
    Serial.println();
    Serial.println("gyro bias [deg/s]: ");
    Serial.print(mpu.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.print(", ");
    Serial.print(mpu.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
    Serial.println();
    Serial.println("mag bias [mG]: ");
    Serial.print(mpu.getMagBiasX());
    Serial.print(", ");
    Serial.print(mpu.getMagBiasY());
    Serial.print(", ");
    Serial.print(mpu.getMagBiasZ());
    Serial.println();
    Serial.println("mag scale []: ");
    Serial.print(mpu.getMagScaleX());
    Serial.print(", ");
    Serial.print(mpu.getMagScaleY());
    Serial.print(", ");
    Serial.print(mpu.getMagScaleZ());
    Serial.println();
}
