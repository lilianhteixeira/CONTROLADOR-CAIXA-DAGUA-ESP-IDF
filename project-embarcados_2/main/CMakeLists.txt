idf_component_register(SRCS "main.c"
                       "components/DS18B20/DS18B20.c"
                       "components/DS18B20/DallasOneWire/onewire/onewire.c"
                       "components/DS18B20/DallasOneWire/drivers/ow_driver_esp32_usart.c"
                       "components/HC_SR04/HC_SR04.c"
                       "components/MotorControl/MotorControl.c"
                       "components/RelayModule/RelayModule.c"
                       INCLUDE_DIRS ".")
