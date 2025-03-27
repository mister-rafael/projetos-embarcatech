#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Configuração dos pinos do LED RGB
const uint BLUE_LED_PIN = 12; // LED azul no GPIO 12
// Configuração dos pinos dos botões como entrada com pull-up
const uint BUTTON_A_PIN = 5; // Botão A no GPIO 5

// Simulação do botão
int botaoSimulado = 0; // Inicialmente não pressionado

QueueHandle_t filaBotao;
QueueHandle_t filaLED;

/**void tarefaLeituraBotao(void *pvParameters);
void tarefaProcessamentoBotao(void *pvParameters);
void tarefaControleLED(void *pvParameters);*/
void setup()
{
    stdio_init_all(); // Inicializa os tipos stdio padrão presentes ligados ao binário

    // Inicialização dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Inicialização dos LEDs
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Inicialmente, desligar o LED RGB
    gpio_put(BLUE_LED_PIN, 0);

    // gpio_put(BLUE_LED_PIN, 1); // Liga o LED azul
}

void tarefaLeituraBotao(void *pvParameters)
{
    int estadoBotao;
    while (1)
    {
        //botaoSimulado = !botaoSimulado;           // Muda de 0 para 1, ou seja, simula o botão sendo pressionado.
        //estadoBotao = botaoSimulado;              // atribui 1 a variável estadoBotao
        estadoBotao = gpio_get(BUTTON_A_PIN) == 0 ? 1 : 0;  // Botão pressionado = 1
        xQueueOverwrite(filaBotao, &estadoBotao); // Envia estado para a fila
        //printf("Botão: %d\n", estadoBotao);       // Mostra o retorno da ação.
        vTaskDelay(pdMS_TO_TICKS(100));          // Alterna a cada 1s
    }
}

void tarefaProcessamentoBotao(void *pvParameters)
{
    int estadoBotao;
    while (1)
    {
        if (xQueueReceive(filaBotao, &estadoBotao, portMAX_DELAY))
        {
            int comandoLED = estadoBotao;
            xQueueOverwrite(filaLED, &comandoLED);
        }
    }
}

void tarefaControleLED(void *pvParameters)
{
    int estadoLED;
    while (1)
    {
        if (xQueueReceive(filaLED, &estadoLED, portMAX_DELAY))
        {
            //printf("LED: %s\n", estadoLED ? "ON" : "OFF");
            gpio_put(BLUE_LED_PIN, estadoLED); // Liga o LED azul
        }
    }
}

int main()
{
    setup();
    filaBotao = xQueueCreate(1, sizeof(int));
    filaLED = xQueueCreate(1, sizeof(int));

    if (filaBotao != NULL && filaLED != NULL)
    {
        xTaskCreate(tarefaLeituraBotao, "LeituraBotao", 256, NULL, 1, NULL);
        xTaskCreate(tarefaProcessamentoBotao, "Processamento", 256, NULL, 2, NULL);
        xTaskCreate(tarefaControleLED, "ControleLED", 256, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1)
        ;
}
