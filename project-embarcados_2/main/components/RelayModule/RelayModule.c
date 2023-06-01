#include "RelayModule.h"
#include <driver/gpio.h>

static int relayPin;

void RelayModule_init(int pin) {
  relayPin = pin;
  gpio_pad_select_gpio(relayPin);
  gpio_set_direction(relayPin, GPIO_MODE_OUTPUT);
}

void RelayModule_activate() {
  gpio_set_level(relayPin, 1);
}

void RelayModule_deactivate() {
  gpio_set_level(relayPin, 0);
}
