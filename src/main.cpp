#include <Arduino.h>
#include "Sensors.h"
#include "calculate.h"
#include <FreeRTOS.h>
#include <Adafruit_NeoPixel.h>
#include <iostream>

using namespace std;
#define LED_PIN   48   
#define NUMPIXELS 1  

int led_colur;
// inlet pressure sensor
#define DOUT_1     18
#define SCK_1      17
// throat pressure sensor
#define DOUT_2     11
#define SCK_2      10
// button calibrate 
#define calibrate_button 13
#define calibrate_sampling 100

#define inlet 0.25 //cm
#define throat 0.1 //cm
#define airdensity 1.225 

volatile bool calibrate = false;
Adafruit_NeoPixel strip(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Sensors sensor(DOUT_2, SCK_2, DOUT_1, SCK_1);
Calculate calculate(inlet, throat, airdensity);
QueueHandle_t queue1; 
QueueHandle_t queue2; 
QueueHandle_t queue3;



 long offset1 = 0;    // ubah: offset sensor 1
 long offset2 = 0;    // ubah: offset sensor 2
 float scale1 = 0.1;// ubah: scale sensor 1 
 float scale2 = 0.1;  // ubah: scale sensor 2 

 

 


void readSensor1(void *pvParameters) {
    while (1) {
        long pressure1 = sensor.getRaw1();
        xQueueSend(queue1, &pressure1, portMAX_DELAY);
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}

void readSensor2(void *pvParameters) {
    while (1) {
        long pressure2 = sensor.getRaw2();
        xQueueSend(queue2, &pressure2, portMAX_DELAY);
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}

void proccessAirflow(void *pvParameters) {
    while (1) {
        long pressure1, pressure2;
        
        if (xQueueReceive(queue1, &pressure1, portMAX_DELAY) == pdPASS) {
            if (xQueueReceive(queue2, &pressure2, portMAX_DELAY) == pdPASS) {
                float airflow = calculate.getAirflow(
                    pressure2, pressure1, offset1, offset2, scale1, scale2);  
                
                // Serial.println("Pressure 1 & 2: ");
                // calculate.showPressures();
                
                Serial.print("Airflow: ");
                Serial.print(airflow,2);
                Serial.println(" ml/s");
                led_colur = map(airflow, 0,300,0,255);
                xQueueSend(queue3, &airflow, portMAX_DELAY);
            }
        }
    }
}

void airVolume(void *pvParameters) {
    float airVolume = 0;  // ubah: pindahkan keluar loop supaya akumulasi
    const float delaytime = 0.15;  // ubah: delaytime harus cocok dengan vTaskDelay (150 ms = 0.15 s)
    while (1) {
        float airflow;
        if (xQueueReceive(queue3, &airflow, portMAX_DELAY) == pdPASS) {
            airVolume += airflow * delaytime;
            Serial.print("Total volume: ");
            Serial.print(airVolume);
            Serial.println(" m3");  // ubah: satuan volume m³
        }
    }
}

void calibrateTask(void *pvParameters) {
  long avg1;
  long avg2;
  long delta;
  bool lastButtonState = HIGH;
  while (1) {
      bool currentState = digitalRead(calibrate_button);
     
      if (lastButtonState == HIGH && currentState == LOW) {
        strip.setPixelColor(0, strip.Color(255, 255, 0)); // Warna kuning saat kalibrasi
        strip.show();
          long sum1 = 0, sum2 = 0;
          const int samples = 100;
          Serial.println("kalibrasi mulai");
          vTaskDelay(1500 / portTICK_PERIOD_MS);
          for (int i = 0; i < samples; i++) {
              sum1 += sensor.getRaw1();
              sum2 += sensor.getRaw2();
              vTaskDelay(5 / portTICK_PERIOD_MS);
          }
          avg1 = sum1 / samples;
          avg2 = sum2 / samples;

          if(avg1>avg2){
            delta = (avg1-avg2);
            offset2 = (delta/2);
            offset1 = -delta/2;
          } else{
            delta = (avg2 - avg1);
            offset1 = (delta/2);
            offset2 = -delta/2;
          }

          // offset1 = sum1 / samples;
          // offset2 = sum2 / samples;

          scale1 = 40000.0f / 16777216.0f;
          scale2 = 40000.0f / 16777216.0f;
          Serial.print("offset1: ");
          Serial.println(offset1);
          Serial.print("offset2: ");
          Serial.println(offset2);
          Serial.println("Kalibrasi selesai");
          strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.show();
          
      }
      lastButtonState = currentState;
      vTaskDelay(50 / portTICK_PERIOD_MS);  
  }
}


void setup() {
    Serial.begin(115200);
    pinMode(DOUT_1, INPUT);
    pinMode(SCK_1, OUTPUT);
    pinMode(DOUT_2, INPUT);
    pinMode(SCK_2, OUTPUT);
    pinMode(calibrate_button, INPUT_PULLUP);
    Serial.println("Setup started!");
    delay(1000);
    queue1 = xQueueCreate(1, sizeof(long));
    queue2 = xQueueCreate(1, sizeof(long));
    queue3 = xQueueCreate(1, sizeof(float));
  

    xTaskCreate(readSensor1, "sensor 1", 1000, NULL, 1, NULL);
    xTaskCreate(readSensor2, "sensor 2", 1000, NULL, 1, NULL);
    xTaskCreate(proccessAirflow, "process airflow", 4096, NULL, 1, NULL);
    xTaskCreate(airVolume, "air volume", 2048, NULL, 1, NULL);
    xTaskCreate(calibrateTask,"calibrate",2048,NULL,1,NULL);

    strip.begin();         
    strip.show(); 

    
  
}

void loop() {
  strip.setPixelColor(0, strip.Color(led_colur, 0, led_colur)); 
  strip.show();
  delay(5);

}
