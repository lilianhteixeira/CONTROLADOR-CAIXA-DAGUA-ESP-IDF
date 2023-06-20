#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_system.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "components/DS18B20/DS18B20.h"
#include "components/DS18B20/ds18x20.h"
#include "components/HC_SR04/HC_SR04.h"
#include "components/MotorControl/MotorControl.h"
#include "components/RelayModule/RelayModule.h"

static const char *TAG_temp = "Temperatura = ";
static const char *TAG_nvl = "Nível da água = ";

// Configuração do pino de dados do sensor DS18B20
const int DS18B20_dataPin = 33;

// Configuração dos pinos do sensor HC-SR04
const int HC_SR04_triggerPin = 19;
const int HC_SR04_echoPin = 18;

// Configuração dos pinos do motor control
const int MotorControl_APin1 = 15;
const int MotorControl_APin2 = 14;

// Configuração do pino do relé
const int RelayModule_relayPin = 16;

// Variáveis globais para armazenar as informações
float desiredTemperature = 50.0;
float desiredTemperature_min = 10.0;
float antTemperature = 0.0;
float currentTemperature = 0.0;
float waterLevel = 0.0;
float waterLevelDefalt = 58.00;

#define MAX_SENSORS 1
static const int RESCAN_INTERVAL = 8;
static const uint32_t LOOP_DELAY_MS = 100;

void temperature_task(void *pvParameter)
{

    ds18x20_addr_t addrs[MAX_SENSORS];
    float temps[MAX_SENSORS];
    size_t sensor_count = 0;

    gpio_set_pull_mode(DS18B20_dataPin, GPIO_PULLUP_ONLY);

    esp_err_t res;
    while (1)
    {
        // Every RESCAN_INTERVAL samples, check to see if the sensors connected
        // to our bus have changed.
        res = ds18x20_scan_devices(DS18B20_dataPin, addrs, MAX_SENSORS, &sensor_count);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG_temp, "Sensors scan error %d (%s)", res, esp_err_to_name(res));
            continue;
        }

        if (!sensor_count)
        {
            ESP_LOGW(TAG_temp, "No sensors detected!");
            continue;
        }

        ESP_LOGI(TAG_temp, "%d sensors detected", sensor_count);

        // If there were more sensors found than we have space to handle,
        // just report the first MAX_SENSORS..
        if (sensor_count > MAX_SENSORS)
            sensor_count = MAX_SENSORS;

        // Do a number of temperature samples, and print the results.
        for (int i = 0; i < RESCAN_INTERVAL; i++)
        {
            // ESP_LOGI(TAG, "Measuring...");

            res = ds18x20_measure_and_read_multi(DS18B20_dataPin, addrs, sensor_count, temps);
            if (res != ESP_OK)
            {
                ESP_LOGE(TAG_temp, "Sensors read error %d (%s)", res, esp_err_to_name(res));
                continue;
            }

            for (int j = 0; j < sensor_count; j++)
            {
              currentTemperature = temps[j];

              ESP_LOGI(TAG_temp, "Temperature = %.3f°C", currentTemperature);
            }

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
        }
    }
}


// Função para atualizar o valor da temperatura
void updateTemperature() {
  currentTemperature = ds18b20_get_temp();
  ESP_LOGE(TAG_temp, "%f", currentTemperature);
}

// Função para atualizar o valor do nível da água
void updateWaterLevel() {
  float distance = HC_SR04_readDistance();
  waterLevel = 80.0 - distance;
}

// Função para controlar o motor até que o cálculo do nível da água seja maior que 3
void controlMotor() {
  ESP_LOGE(TAG_nvl, "%f", waterLevel);
  updateWaterLevel();
  while (waterLevel < waterLevelDefalt) {
    MotorControl_deactivate(MotorControl_APin1, MotorControl_APin2);
    updateWaterLevel();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  MotorControl_activate(MotorControl_APin1, MotorControl_APin2);
}

// Função para acionar ou desativar o relé com base na temperatura desejada
void controlRelay() {
  if (((currentTemperature < desiredTemperature_min) || (antTemperature == desiredTemperature_min)) && (currentTemperature < desiredTemperature)) {
    antTemperature = desiredTemperature_min;
    RelayModule_deactivate(RelayModule_relayPin);
  } else{
    antTemperature = desiredTemperature;
    RelayModule_activate(RelayModule_relayPin);
  }
}

// Função para atualizar a temperatura desejada
esp_err_t updateDesiredTemperature(httpd_req_t *req) {
  char buf[100];
  if (httpd_req_get_hdr_value_str(req, "Content-Length", buf, sizeof(buf)) <= 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing Content-Length header");
    return ESP_FAIL;
  }
  int content_len = atoi(buf);
  if (content_len <= 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid Content-Length value");
    return ESP_FAIL;
  }
  if (httpd_req_recv(req, buf, content_len) <= 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive data");
    return ESP_FAIL;
  }
  buf[content_len] = '\0';
  desiredTemperature = atof(buf);
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

// Função para lidar com as requisições HTTP
esp_err_t http_handler(httpd_req_t *req) {
  if (strcmp(req->uri, "/temperature") == 0) {
    char temperature[10];
    sprintf(temperature, "%.2f", currentTemperature);
    httpd_resp_send(req, temperature, strlen(temperature));
  } else if (strcmp(req->uri, "/waterlevel") == 0) {
    char level[10];
    sprintf(level, "%.2f", waterLevel);
    httpd_resp_send(req, level, strlen(level));
  } else if (strcmp(req->uri, "/desiredtemperature") == 0) {
    if (req->method == HTTP_POST) {
      return updateDesiredTemperature(req);
    } else if (req->method == HTTP_GET) {
      char desiredTemp[10];
      sprintf(desiredTemp, "%.2f", desiredTemperature);
      httpd_resp_send(req, desiredTemp, strlen(desiredTemp));
    }
  } else {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found");
  }
  return ESP_OK;
}

// Função para inicializar o servidor HTTP
httpd_handle_t start_http_server() {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.uri_match_fn = httpd_uri_match_wildcard;
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t temperature_uri = {
        .uri = "/temperature",
        .method = HTTP_GET,
        .handler = http_handler,
    };
    httpd_register_uri_handler(server, &temperature_uri);

    httpd_uri_t waterlevel_uri = {
        .uri = "/waterlevel",
        .method = HTTP_GET,
        .handler = http_handler,
    };
    httpd_register_uri_handler(server, &waterlevel_uri);

    httpd_uri_t desiredtemperature_uri = {
        .uri = "/desiredtemperature",
        .method = HTTP_GET | HTTP_POST,
        .handler = http_handler,
    };
    httpd_register_uri_handler(server, &desiredtemperature_uri);
  }
  return server;
}

// Função para inicializar a conexão Wi-Fi
void wifi_init() {
  // Inicializa o subsistema NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // Se as páginas do NVS não estão livres ou uma nova versão é encontrada, apaga e reinicializa o NVS
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Inicializa o subsistema Wi-Fi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = "GABRIEL",
          .password = "error02468",
          .scan_method = WIFI_FAST_SCAN,
          .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
          .threshold.rssi = -127,
          .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      },
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void) {
  nvs_flash_init();
  
  // Inicializa a conexão Wi-Fi
  //wifi_init();
  // Inicializa o servidor HTTP
  //httpd_handle_t server = start_http_server();
  
  // Inicializa o sensor DS18B20
  xTaskCreate(temperature_task, "temperature_task", configMINIMAL_STACK_SIZE * 32, NULL, 5, NULL);

  // Inicializa o sensor HC-SR04
  HC_SR04_init(HC_SR04_triggerPin, HC_SR04_echoPin);

  // Inicializa o módulo de controle do motor
  MotorControl_init(MotorControl_APin1, MotorControl_APin2);

  // Inicializa o módulo de relé
  RelayModule_init(RelayModule_relayPin);


  while (1) {
    // Atualiza a temperatura
    //updateTemperature();

    // Atualiza o nível da água
    updateWaterLevel();
    ESP_LOGE(TAG_nvl, "%f", waterLevel);

    // Controla o motor caso o nível da água seja baixo
    if (waterLevel < waterLevelDefalt) {
      controlMotor();
    }

    // Controla o relé com base na temperatura desejada
    controlRelay();

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Aguarda 1 segundo antes da próxima iteração
  }

  // Encerra o servidor HTTP
  //httpd_stop(server);

}