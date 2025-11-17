
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "hardware/pwm.h"
#include "../includes/LoRa-RP2040.h"
#include "../includes/pluviometro.hpp"
#include "pico/binary_info.h"

#define UART_PRINT_ID uart0
#define UART_PRINT_TX 0
#define UART_PRINT_RX 1
#define UART_PRINT_BAUD_RATE 9600

#define LOG_BUFFER_SIZE 512
#define RUN_TIME_BUFFER_SIZE 512

__attribute__((section(".rtos_heap"))) uint8_t ucHeap[configTOTAL_HEAP_SIZE];

void vLoRaSenderTask(void *pvParameters);
void vLoRaConfigTask(void *pvParameters);
void vSystemLogTask(void *pvParameters);
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  uart_config
 *  Description:  Funcao responsavel por realizar as configuracoes de uart
 * =====================================================================================
*/
void uart_config(void);

typedef struct {
    char* print;
} SystemQueues_t;

//semaforos
SemaphoreHandle_t xLoRaInitSemaphore;
//SENSOR PLUVIOMETRO
#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define WAKE_GPIO 28

extern uint slice_num;

int main() {
    stdio_init_all();  
    uart_config();
    inicializa_sensor_pluviometro(SENSOR_HALL_PIN);

    //dados para o Log do sistema
    xLoRaInitSemaphore = xSemaphoreCreateBinary();
    if (xLoRaInitSemaphore == NULL) {
        while(1);
    }

    SystemQueues_t systemQueues;
    char buffer[0xffff];
    systemQueues.print = buffer;

    //Criar as tasks
   // xTaskCreate(vSystemLogTask,"Task Log", 10000, &systemQueues, 5, NULL);
    xTaskCreate(vLoRaConfigTask,"Task LoRaConfig", 256, NULL, 1, NULL);
    xTaskCreate(vLoRaSenderTask,"Task LoRaSender",256,NULL,2,NULL);

    //escalona as tarefas
    vTaskStartScheduler();

    while (1) {
        printf("Falha ao iniciar o scheduler do FreeRTOS.\n");
    }
    return 0;
}

void vLoRaSenderTask(void *pvParameters){
    uint8_t counter = 0;
    const TickType_t wait = pdMS_TO_TICKS(3000);

    if (xSemaphoreTake(xLoRaInitSemaphore, portMAX_DELAY) == pdTRUE) {
        while (true) {

            uart_puts(UART_PRINT_ID, "Received packet \n\r");
            //int packetSize = LoRa.parsePacket();
            /* Formatando mensagem com dados lidos (umidade, temperatura e precipitação) */
    char message[100];
    snprintf(message, sizeof(message),
        "t|%.1f|h|%.1f|r|%.4f",
        30, 60, pwm_get_counter(slice_num) * PRECIPITACAO);

          /* Enviando mensagem formatada via UART */
    uart_puts(UART_PRINT_ID, message);
    uart_puts(UART_PRINT_ID, "\n\r");
        /* Iniciando um novo pacote de transmissao LoRa */
    LoRa.beginPacket();
      /* Adicionando string ao pacote LoRa */
    LoRa.print(message);
            //string response = "eq1_temperature " + std::to_string(pwm_get_counter(slice_num) * PRECIPITACAO);
            // LoRa.beginPacket();
            // LoRa.print(response.c_str());  // Enviar a resposta com a temperatura
            LoRa.endPacket();
            uart_puts(UART_PRINT_ID, "pacote enviado \n\r");
            // uart_puts(UART_PRINT_ID, response.c_str());
            // uart_puts(UART_PRINT_ID, "\n\r");
            // if (packetSize) {
            //     // received a packet
            //     uart_puts(UART_PRINT_ID, "Received packet \n\r");
            //     string request = "";
            //     // read packet
            //     while (LoRa.available()) {
            //         request += (char)LoRa.read();
            //     }

            //     uart_puts(UART_PRINT_ID, "request");
            //     uart_puts(UART_PRINT_ID, "\n\r");
                
            //     if(request == "request_data_from_node_1"){
            //         string response = "eq1_temperature " + std::to_string(pwm_get_counter(slice_num) * PRECIPITACAO);
            //         LoRa.beginPacket();
            //         LoRa.print(response.c_str());  // Enviar a resposta com a temperatura
            //         LoRa.endPacket();
            //         uart_puts(UART_PRINT_ID, response.c_str());
            //         uart_puts(UART_PRINT_ID, "\n\r");
            //     }

            //     // print RSSI of packet
            //     uart_puts(UART_PRINT_ID, "with RSSI \n\r");
            //     uart_puts(UART_PRINT_ID, (char*)LoRa.packetRssi());
            //     uart_puts(UART_PRINT_ID, "\n\r");
            //     uart_puts(UART_PRINT_ID, "pacote enviado\n\r");
            // }
            vTaskDelay(wait);
        }
    }
}

void vSystemLogTask(void *pvParameters) {
    SystemQueues_t* param = (SystemQueues_t*)pvParameters;
    const TickType_t logInterval = pdMS_TO_TICKS(4000);
    while (true) {
        printf("\n================= ESTADO DO SISTEMA =================\n");

        // Log do uso de memória heap
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minHeap = xPortGetMinimumEverFreeHeapSize();
        printf("Uso de Memória:\n");
        printf("Heap Livre: %lu bytes\n", freeHeap);
        printf("Heap Mínimo Já Registrado: %lu bytes\n\n", minHeap);

        // Log do estado das tasks
        printf("Estado das Tasks:\n");
        printf("Task Name\tStatus\tPri\tStack\tTask\tCoreAf#\n");
        vTaskList(param->print);
        printf("%s\n", param->print);

        // Estatísticas de tempo de execução
#if configGENERATE_RUN_TIME_STATS
        printf("Tempo de Execução das Tasks:\n");
        vTaskGetRunTimeStats(param->print);
        printf("%s\n", param->print);
#endif
        // Delay para o próximo log
        vTaskDelay(logInterval);
    }
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  uart_config
 *  Description:  Funcao responsavel por realizar as configuracoes de uart
 * =====================================================================================
*/
void uart_config(void) {
  /* Setando os GPIOs para nivel logico baixo e multiplexando para UART */
  gpio_put(UART_PRINT_TX, 0);
  /* Inicializacao da UART para GPS e configuracao da taxa de baud */
  uart_init(UART_PRINT_ID, UART_PRINT_BAUD_RATE);
  gpio_set_function(UART_PRINT_TX, GPIO_FUNC_UART);
}

void vLoRaConfigTask(void *pvParameters) {
    const TickType_t wait = pdMS_TO_TICKS(1000);
    uart_puts(UART_PRINT_ID, "Iniciando configuracao do LoRa...\n\r");
    while(true){
        if (!LoRa.begin(915E6)) {
            uart_puts(UART_PRINT_ID, "Falha na inicializacao LoRa, tentando novamente\n\r");
            
        }else{
            LoRa.setSyncWord(0xa1);

            uart_puts(UART_PRINT_ID, "Modulo LoRa inicializado com sucesso!\n\r");

            //modulo configurado, agora pode escalonar a task de envio
            xSemaphoreGive(xLoRaInitSemaphore);
            vTaskDelete(NULL);
        }
        vTaskDelay(wait);
    }
}

