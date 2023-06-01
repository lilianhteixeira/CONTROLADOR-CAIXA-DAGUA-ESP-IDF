#include "MotorControl.h"
#include <driver/gpio.h>

void MotorControl_init(int pin1, int pin2){
  gpio_pad_select_gpio(pin1);
  gpio_pad_select_gpio(pin2);
  gpio_set_direction(pin1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pin2, GPIO_MODE_OUTPUT);
}

void MotorControl_activate(int pin1, int pin2){
  gpio_set_level(pin1, 1);
  gpio_set_level(pin2, 0);
}

void MotorControl_deactivate(int pin1, int pin2){
  gpio_set_level(pin1, 0);
  gpio_set_level(pin2, 0);
}
