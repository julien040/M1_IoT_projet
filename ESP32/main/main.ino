/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd-lvgl/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft-lvgl/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen - Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>

// Install "PubSubClient" library by Nick O'Leary for MQTT
#include <WiFi.h>
#include <PubSubClient.h>

// *** REPLACE WITH YOUR WIFI CREDENTIALS ***
const char* WIFI_SSID = /*"Livebox-AOAO"*/ "iPhone de Julien";
const char* WIFI_PASSWORD = /*"bSVn5zt2VRfScZsLVNs"*/"test1234";

// MQTT Broker settings
const char* MQTT_BROKER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "ESP32_CYD_Client";

// MQTT Topics
const char* TOPIC_CAR_TIME = "UPPA_M1_IOT/harvest/travelTime/car";
const char* TOPIC_BUS_TIME = "UPPA_M1_IOT/harvest/travelTime/bus";
const char* TOPIC_BUS_DEPARTURES = "UPPA_M1_IOT/harvest/nextBusDepartures";

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Global variables for travel data
String carTime = "-- min";
String busTime = "-- min";
String busDepartures = "Loading...";

// LVGL label pointers for dynamic updates
lv_obj_t *bus_time_label = NULL;
lv_obj_t *car_time_label = NULL;
lv_obj_t *bus_departures_label = NULL;

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMAGE_LIGNE_F
#define LV_ATTRIBUTE_IMAGE_LIGNE_F
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMAGE_LIGNE_F uint8_t Ligne_F_map[] = {
  0x00, 0x00, 0x80, 0xfc, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x60, 0xfc, 0x00, 0x00, 
  0x40, 0xfc, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x60, 0xfc, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x00, 0xf4, 0xe1, 0xf3, 0xe1, 0xf3, 0xe1, 0xf3, 0xe1, 0xf3, 0xe1, 0xf3, 0x20, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x93, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8b, 0xfd, 0x20, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7b, 0xff, 0x9c, 0xff, 0xbd, 0xff, 0x49, 0xfd, 0x20, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xd6, 0xfe, 0x00, 0xf3, 0xa0, 0xf3, 0x80, 0xf3, 0x00, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xd6, 0xfe, 0x20, 0xf3, 0xc0, 0xf3, 0xa0, 0xf3, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xbd, 0xff, 0x94, 0xfe, 0xd5, 0xfe, 0x18, 0xff, 0x42, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0x18, 0xff, 0x02, 0xf4, 0x84, 0xf4, 0x84, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xf7, 0xfe, 0xa0, 0xf3, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0x52, 0xfe, 0xff, 0xff, 0xf7, 0xfe, 0xc0, 0xf3, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0xe0, 0xf3, 0xb4, 0xfe, 0xff, 0xff, 0x59, 0xff, 0xa0, 0xf3, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x20, 0xf4, 0x83, 0xf4, 0xc6, 0xf4, 0xa4, 0xf4, 0x20, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 
  0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x60, 0xf4, 
  0x00, 0x00, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xf4, 0x40, 0xfc, 0x00, 0x00, 
};

const lv_image_dsc_t Ligne_F = {
  {
    LV_COLOR_FORMAT_RGB565,
    LV_IMAGE_HEADER_MAGIC,
    17,
    18
  },
  306 * 2,
  Ligne_F_map,
};

// WiFi connection function/ WiFi connection function with detailed error reporting
void connectWiFi() {
  Serial.println("=== WiFi Connection Attempt ===");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  
  // Check if credentials look valid
  if (String(WIFI_SSID) == "YOUR_WIFI_SSID" || String(WIFI_SSID) == "") {
    Serial.println("ERROR: WiFi SSID not configured!");
    Serial.println("Please replace YOUR_WIFI_SSID with your actual WiFi network name");
    return;
  }
  
  if (String(WIFI_PASSWORD) == "YOUR_WIFI_PASSWORD" || String(WIFI_PASSWORD) == "") {
    Serial.println("ERROR: WiFi PASSWORD not configured!");
    Serial.println("Please replace YOUR_WIFI_PASSWORD with your actual WiFi password");
    return;
  }
  
  // Disconnect if already connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected. Disconnecting...");
    WiFi.disconnect();
    delay(100);
  }
  
  // Start WiFi in station mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Print status every 5 attempts
    if (attempts % 5 == 0) {
      Serial.println();
      Serial.print("Status: ");
      switch(WiFi.status()) {
        case WL_IDLE_STATUS:
          Serial.print("IDLE");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.print("NO SSID AVAILABLE - Check if SSID '");
          Serial.print(WIFI_SSID);
          Serial.print("' is correct and in range");
          break;
        case WL_CONNECT_FAILED:
          Serial.print("CONNECTION FAILED - Check password");
          break;
        case WL_DISCONNECTED:
          Serial.print("DISCONNECTED");
          break;
        default:
          Serial.print(WiFi.status());
          break;
      }
      Serial.println();
      Serial.print("Continuing");
    }
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("=== WiFi Connected Successfully! ===");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("=== WiFi Connection Failed! ===");
    Serial.print("Final status: ");
    
    switch(WiFi.status()) {
      case WL_IDLE_STATUS:
        Serial.println("IDLE - WiFi is in idle status");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("NO SSID AVAILABLE");
        Serial.println("Possible causes:");
        Serial.println("  - SSID name is incorrect (check spelling and case)");
        Serial.println("  - Router is out of range");
        Serial.println("  - Router is turned off");
        Serial.print("  - Searching for: '");
        Serial.print(WIFI_SSID);
        Serial.println("'");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("CONNECTION FAILED");
        Serial.println("Possible causes:");
        Serial.println("  - Incorrect password");
        Serial.println("  - Router security settings incompatible");
        Serial.println("  - MAC address filtering enabled on router");
        break;
      case WL_DISCONNECTED:
        Serial.println("DISCONNECTED");
        Serial.println("Connection was interrupted or rejected");
        break;
      default:
        Serial.print("Unknown status code: ");
        Serial.println(WiFi.status());
        break;
    }
    
    Serial.println("\nTroubleshooting steps:");
    Serial.println("1. Verify SSID and password are correct");
    Serial.println("2. Check if router is 2.4GHz (ESP32 doesn't support 5GHz)");
    Serial.println("3. Move ESP32 closer to the router");
    Serial.println("4. Check if router has available connections");
    Serial.println("5. Try restarting the router");
  }
  Serial.println("==============================");
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message received on topic: ");
  Serial.print(topic);
  Serial.print(" - Message: ");
  Serial.println(message);
  
  // Update the appropriate variable based on topic
  if (strcmp(topic, TOPIC_CAR_TIME) == 0) {
    carTime = message;
    if (car_time_label != NULL) {
      String displayText = "En voiture (" + carTime + ")";
      lv_label_set_text(car_time_label, displayText.c_str());
    }
  } 
  else if (strcmp(topic, TOPIC_BUS_TIME) == 0) {
    busTime = message;
    if (bus_time_label != NULL) {
      String displayText = "En bus (" + busTime + ")";
      lv_label_set_text(bus_time_label, displayText.c_str());
    }
  } 
  else if (strcmp(topic, TOPIC_BUS_DEPARTURES) == 0) {
    busDepartures = message;
    if (bus_departures_label != NULL) {
      lv_label_set_text(bus_departures_label, busDepartures.c_str());
    }
  }
}

// Connect to MQTT broker
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Generate unique client ID
    String clientId = String(MQTT_CLIENT_ID) + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // Subscribe to topics
      mqttClient.subscribe(TOPIC_CAR_TIME);
      mqttClient.subscribe(TOPIC_BUS_TIME);
      mqttClient.subscribe(TOPIC_BUS_DEPARTURES);
      
      Serial.println("Subscribed to topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    // Constrain values to screen dimensions
    x = constrain(x, 0, SCREEN_WIDTH - 1);
    y = constrain(y, 0, SCREEN_HEIGHT - 1);

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void create_commute_ui(lv_obj_t *parent) {
    // Set black background
    lv_obj_set_style_bg_color(parent, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    
    // Main container
    lv_obj_t *main_cont = lv_obj_create(parent);
    lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(main_cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_style_pad_all(main_cont, 15, 0);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(main_cont, 8, 0);
    
    // Title label
    lv_obj_t *title = lv_label_create(main_cont);
    lv_label_set_text(title, "Pour vous rendre au travail");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_bottom(title, 5, 0);
    
    // Bus option
    lv_obj_t *bus_row = lv_obj_create(main_cont);
    lv_obj_set_size(bus_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(bus_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bus_row, 0, 0);
    lv_obj_set_style_pad_all(bus_row, 3, 0);
    lv_obj_set_flex_flow(bus_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bus_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(bus_row, 6, 0);
    
    // Bus icon - check if image is valid before setting
    lv_obj_t *bus_icon = lv_image_create(bus_row);
    if (Ligne_F.data != NULL) {
      lv_image_set_src(bus_icon, &Ligne_F);
    }
    
    // Bus time text (dynamic)
    bus_time_label = lv_label_create(bus_row);
    String busText = "En bus (" + busTime + ")";
    lv_label_set_text(bus_time_label, busText.c_str());
    lv_obj_set_style_text_color(bus_time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(bus_time_label, &lv_font_montserrat_12, 0);
    
    // RSS icon placeholder (using Unicode symbol)
    lv_obj_t *rss_label = lv_label_create(bus_row);
    lv_label_set_text(rss_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(rss_label, lv_color_make(76, 175, 80), 0); // Green
    
    // Bus departures text (dynamic)
    bus_departures_label = lv_label_create(bus_row);
    lv_label_set_text(bus_departures_label, busDepartures.c_str());
    lv_obj_set_style_text_color(bus_departures_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(bus_departures_label, &lv_font_montserrat_12, 0);
    
    // Car option
    lv_obj_t *car_row = lv_obj_create(main_cont);
    lv_obj_set_size(car_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(car_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(car_row, 0, 0);
    lv_obj_set_style_pad_all(car_row, 3, 0);
    lv_obj_set_flex_flow(car_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(car_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(car_row, 6, 0);
    
    // Car icon (using built-in symbol)
    lv_obj_t *car_icon = lv_label_create(car_row);
    lv_label_set_text(car_icon, LV_SYMBOL_GPS);
    lv_obj_set_style_text_color(car_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(car_icon, &lv_font_montserrat_14, 0);
    
    // Car time text (dynamic)
    car_time_label = lv_label_create(car_row);
    String carText = "En voiture (" + carTime + ")";
    lv_label_set_text(car_time_label, carText.c_str());
    lv_obj_set_style_text_color(car_time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(car_time_label, &lv_font_montserrat_12, 0);
}

void lv_create_main_gui(void) {
  // Create the commute UI
  create_commute_ui(lv_screen_active());
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  delay(1000);  // Give serial time to initialize
  Serial.println(LVGL_Arduino);
  
  // Start LVGL first
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Function to draw the GUI
  lv_create_main_gui();
  
  // Connect to WiFi after GUI is created
  connectWiFi();
  
  // Setup MQTT only if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(512);  // Increase buffer size for larger messages
    connectMQTT();
  }
}

void loop() {
  // Handle MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      connectMQTT();
    }
    mqttClient.loop();
  }
  
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}
