#include "HC_SR04.h"
#include <driver/gpio.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static int triggerPin;
static int echoPin;

void HC_SR04_init(int trigger, int echo) {
  triggerPin = trigger;
  echoPin = echo;

  gpio_pad_select_gpio(triggerPin);
  gpio_pad_select_gpio(echoPin);
  gpio_set_direction(triggerPin, GPIO_MODE_OUTPUT);
  gpio_set_direction(echoPin, GPIO_MODE_INPUT);
}

float HC_SR04_readDistance() {
  gpio_set_level(triggerPin, 1);
  vTaskDelay(0.01 / portTICK_PERIOD_MS);
  gpio_set_level(triggerPin, 0);

  while (gpio_get_level(echoPin) == 0)
    ;
  int64_t start = esp_timer_get_time();
  while (gpio_get_level(echoPin) == 1)
    ;
  int64_t end = esp_timer_get_time();

  float distance = (end - start) * 0.0343 / 2.0;
  return distance;
}
