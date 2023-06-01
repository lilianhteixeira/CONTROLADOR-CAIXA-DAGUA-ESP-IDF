#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include "DS18B20.h"
#include "DallasOneWire\onewire\onewire.h"
#include "DallasOneWire\drivers\ow_driver.h"

float readTemperature(int gpio_pin)
{
    // Inicializa o driver OneWire
    ow_driver_ptr driver;
    int result = init_driver(&driver, gpio_pin);

    if (result == 0)
    {
        // Inicializa a estrutura owu_struct_t
        owu_struct_t owu;
        owu_init(&owu, driver);

        // Reseta o barramento OneWire
        result = owu_reset(&owu);

        if (result == OW_OK)
        {
            // Envia o comando de conversão de temperatura
            owu_skip(&owu);
            owu_write_byte(&owu, 0x44);

            // Aguarda a conversão
            while (owu_read_byte(&owu, NULL) == 0xFF)
            {
                // Espera até que a conversão seja concluída
            }

            // Reseta o barramento OneWire novamente
            owu_reset(&owu);

            // Seleciona o dispositivo DS18B20
            uint8_t ds18b20_rom_code[8] = {0x28, 0xFF, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55};
            result = owu_select_device(&owu, ds18b20_rom_code);

            if (result == OW_OK)
            {
                // Envia o comando de leitura da temperatura
                owu_write_byte(&owu, 0xBE);

                // Lê os dados de temperatura do DS18B20
                uint8_t data[9];
                for (int i = 0; i < 9; i++)
                {
                    owu_read_byte(&owu, &data[i]);
                }

                // Calcula e retorna a temperatura
                int16_t raw_temp = (data[1] << 8) | data[0];
                float temperature = (float)raw_temp / 16.0;

                // Libera os recursos do driver
                release_driver(&driver);

                return temperature;
            }
        }

        // Libera os recursos do driver em caso de erro
        release_driver(&driver);
    }

    return -273.15f; // Retorna -273.15 °C se o sensor não for encontrado ou houver um erro no driver
}
