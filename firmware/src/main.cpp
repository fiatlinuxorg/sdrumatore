#include <WiFi.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <vector>

#include "common.h"
#include "world.h"
#include "clock.h"
#include "clockview.h"
#include "busview.h"
#include "msgview.h"

#define NUM_VIEWS 3

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

using namespace std;

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

TFT_eSPI tft = TFT_eSPI();  

World world(&tft);
View* views[NUM_VIEWS];
int current_view_idx = 0;

const char *ssid = "<SSID>";
const char *password = "<PASS>";

void setup() {
    Serial.begin(115200);
    
    // Init touch screen
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);

    ts.begin(mySpi);
    ts.setRotation(1);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Init wifi
    Serial.println("\nConnecting");

    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED){
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    // Init views
    views[0] = new ClockView();
    views[1] = new BusView("fiera", "5");
    views[2] = new BusView("fiera", "3");
    //views[2] = new MsgView();
}

void next_frame() {
    views[current_view_idx]->draw(&world);
    views[current_view_idx]->update(&world);
    world.draw();
    world.step();
}

void loop() {
    //Clock clock = Clock::get_instance();
    
    if (ts.tirqTouched() && ts.touched()) {
        TS_Point p = ts.getPoint();

        if (p.x >= 2466) {
            current_view_idx = (current_view_idx + 1) % NUM_VIEWS;
        } else if (p.x <= 1233) {
            current_view_idx = (current_view_idx - 1 + NUM_VIEWS) % NUM_VIEWS;
        }

        world.flush();
        views[current_view_idx]->present(&world);
    }
    next_frame();
}
