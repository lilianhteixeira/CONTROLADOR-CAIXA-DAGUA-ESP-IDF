idf_component_register(SRCS "main.c"
                       "components/DS18B20/DS18B20.c"
                       "components/DS18B20/ds18x20.c"
                       "components/onewire/onewire.c"
                       "components/HC_SR04/HC_SR04.c"
                       "components/MotorControl/MotorControl.c"
                       "components/RelayModule/RelayModule.c"
                       INCLUDE_DIRS ".")
