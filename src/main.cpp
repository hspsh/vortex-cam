#include <Arduino.h>
#include <WiFi.h>

#define USE_ASYNCMQTTCLIENT
#define HOMIELIB_VERBOSE

extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <LeifHomieLib.h>

#include <ESPAsyncWebServer.h>

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"



const char *mqttHost = "192.168.88.170";

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
#define FLASH_GPIO_NUM 4

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void flashOnForNSeconds(int seconds)
{
  digitalWrite(FLASH_GPIO_NUM, HIGH);
  delay(seconds * 1000);
  digitalWrite(FLASH_GPIO_NUM, LOW);
}

HomieDevice homie;
// We'll need a place to save pointers to our created properties so that we can access them again later.
HomieProperty *pPropBuzzer = NULL;
HomieProperty *pPropTray = NULL;

#define CAMERA_MODEL_AI_THINKER
#include "camera_pinout.h"

camera_fb_t *fb = NULL;
size_t _jpg_buf_len = 0;
uint8_t *_jpg_buf = NULL;

void startCameraServer()
{
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    
    esp_err_t res = ESP_OK;

    digitalWrite(FLASH_GPIO_NUM, HIGH);
    fb = esp_camera_fb_get();
    digitalWrite(FLASH_GPIO_NUM, LOW);
    if (!fb)
    {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    else
    {
      Serial.println("Camera capture success");
      if (fb->width > 400)
      {
        if (fb->format != PIXFORMAT_JPEG)
        {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted)
          {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        }
        else
        {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    request->send_P(200, "image/jpg", _jpg_buf, _jpg_buf_len);
    Serial.print("Sent packet of length ");
    Serial.println(_jpg_buf_len);
    esp_camera_fb_return(fb);
  });

  // Start server
  server.begin();
}

#define DEVICE_ID "40"
#define DEVICE_NAME "hsp-vortex-cam"

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  Serial.begin(115200);
  Serial.setDebugOutput(false);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  HomieLibRegisterDebugPrintCallback([](const char *szText)
                                     { Serial.printf("%s", szText); });

  {
    HomieNode *pNode = homie.NewNode();

    pNode->strID = "properties";
    pNode->strFriendlyName = "Properties";
    //		pNode->strType="customtype";

    HomieProperty *pProp;

    pPropBuzzer = pProp = pNode->NewProperty();
    pProp->strFriendlyName = "Annoying Buzzer";
    pProp->strID = "buzzer";
    pProp->SetRetained(true);
    pProp->SetSettable(true);
    pProp->datatype = homieBool;
    pProp->SetBool(false);
    pProp->strFormat = "";
    pProp->AddCallback([](HomieProperty *pSource)
                       {
			//this property is settable. We'll print it into the console whenever it's updated.
			//you can set it from MQTT Explorer by publishing a number between 0-100 to homie/examplehomiedev/nodeid1/dimmer
			//but remember to check the *retain* box.
			Serial.printf("%s is now %s\n",pSource->strFriendlyName.c_str(),pSource->GetValue().c_str()); 
      digitalWrite(16, strcmp(pSource->GetValue().c_str(), "true") == 0 ? HIGH : LOW);
    });

    pProp = pNode->NewProperty();
    pProp->strFriendlyName = "Flash LED";
    pProp->strID = "flash";
    pProp->SetRetained(true);
    pProp->SetSettable(true);
    pProp->datatype = homieBool;
    pProp->SetBool(false);
    pProp->strFormat = "";
    pProp->AddCallback([](HomieProperty *pSource)
                       {
			//this property is settable. We'll print it into the console whenever it's updated.
			//you can set it from MQTT Explorer by publishing a number between 0-100 to homie/examplehomiedev/nodeid1/dimmer
			//but remember to check the *retain* box.
			Serial.printf("%s is now %s\n",pSource->strFriendlyName.c_str(),pSource->GetValue().c_str()); 
      digitalWrite(4, strcmp(pSource->GetValue().c_str(), "true") == 0 ? HIGH : LOW);
    });

    pPropTray = pProp = pNode->NewProperty();
    pProp->strFriendlyName = "Tray detected";
    pProp->strID = "tray";
    pProp->SetRetained(true);
    pProp->datatype=homieInt;
		pProp->strFormat="0:3";
		pProp->SetValue("0");
  }

  {
    // HomieNode *pRes = homie.NewNode();
    // HomieProperty *pProp;

    // pRes->strID = "resources";
    // pRes->strFriendlyName = "Resources";

    // pRes->NewProperty();
    // pProp->strFriendlyName = "Photo from Camera";
    // pProp->strID = "photo";
    // pProp->datatype=homieString;
		// pProp->SetValue("photo");
  }

  homie.strFriendlyName = "Vortex Cam";
  homie.strID = "vortex-cam-1";
  homie.strID.toLowerCase();

  homie.strMqttServerIP = mqttHost;
  homie.Init();
  startCameraServer();
}

void loop()
{
  homie.Loop();
  if (homie.IsConnected())
  {
  }
  delay(100);
}