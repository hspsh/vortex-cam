#include "commonFwUtils.h"
#include <ArduinoOTA.h>
#include "secrets.h"


void handle_io_pattern(uint8_t pin, led_patterns_type_t target_pattern){
  static uint32_t pattern_counter = 0;
  static uint8_t heartbeat_pattern[] = {1,0,0,1,0,0,0,0,0,0,0,0,0};
  static uint8_t errcon_pattern[] = {1,0,1,0,1,0,1,1,1,0,0,0,0};
  
  switch (target_pattern){    
    case PATTERN_HBEAT:
      digitalWrite(pin, heartbeat_pattern[
        pattern_counter % sizeof(heartbeat_pattern)
      ]);
      break;
    
    case PATTERN_ERR:
      digitalWrite(pin, errcon_pattern[
        pattern_counter % sizeof(errcon_pattern)
      ]);
      break;
    case PATTERN_NONE:
    default:
      digitalWrite(pin,0);
      break;
  }
  pattern_counter++;
}

void begin_hspota(){

  ArduinoOTA.setPassword(ARDUINO_OTA_PASSWD);
  ArduinoOTA.setPort(8266);

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
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
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}