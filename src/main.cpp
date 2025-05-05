#include <Arduino.h>
#include "Sensors.h"
#include <FreeRTOS.h>

#define LED_PIN    48
#define DOUT_1     18
#define SCK_1      17
#define DOUT_2     11
#define SCK_2      10
#define BUTTON_PIN 9

volatile bool calibrate = false;
Sensors sensor(100, DOUT_1, SCK_1, DOUT_2, SCK_2);
QueueHandle_t queue1; 
QueueHandle_t queue2;

// void IRAM_ATTR buttonPress() {
//     calibrate = true;
// }


void readSensor1(void *pvParameters){
  while (1)
  {
    long pressure1 = sensor.getRaw1();
    xQueueSend(queue1, &pressure1, portMAX_DELAY); 
    vTaskDelay(120 / portTICK_PERIOD_MS);
  }
  
}

void readSensor2(void *pvParameters){
  while (1)
  {
    long pressure2 = sensor.getRaw2();
    xQueueSend(queue2, &pressure2, portMAX_DELAY); 
    vTaskDelay(120 / portTICK_PERIOD_MS);
  }
  
}

void calculateData(void *pvParameters){
  while(1){
    long pressure1, pressure2;
    if (xQueueReceive(queue1, &pressure1, portMAX_DELAY) == pdPASS) { 
      if (xQueueReceive(queue2, &pressure2, portMAX_DELAY) == pdPASS) { 
          long sum = pressure2 - pressure1;
          while (sum >= 100) {  
            sum /= 10;
        }
          Serial.print("Pressure 1: ");
          Serial.println(pressure1);
          Serial.print("Pressure 2: ");
          Serial.println(pressure2);
          Serial.print("Sum: ");
          Serial.println(sum);
      }
  }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
    Serial.begin(115200);
    pinMode(DOUT_1, INPUT);
    pinMode(SCK_1, OUTPUT);
    pinMode(DOUT_2, INPUT);
    pinMode(SCK_2, OUTPUT);
    Serial.println("Setup started!");

    queue1 = xQueueCreate(1, sizeof(long)); 
    queue2 = xQueueCreate(1, sizeof(long)); 

    xTaskCreate(readSensor1,"sensor 1",1000, NULL, 1, NULL);
    xTaskCreate(readSensor2,"sensor 2",1000, NULL, 1, NULL);
    xTaskCreate(calculateData,"kalkulasi",1000, NULL, 1, NULL);
}

void loop() {
    // if (calibrate) {
    //     sensor1.setOffsets(DOUT_1, SCK_1, DOUT_2, SCK_2);
    //     Serial.println("Calibration done.");
    //     calibrate = false;
    // }

}
