#include <stdio.h>//Biblioteca de Entrada/Saída do windows
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"

//Declaração das variáveis para controle da matriz 5x5
const uint8_t matriz_pino = 7;//Variável com o valor do pino da matriz
volatile uint8_t valor_matriz = 0;//Variável com o valor que será exibido na matriz

//Declaração das variáveis para controle dos botões da placa
const uint8_t btn_a = 5;//Variável com o valor do pino do botão A da placa
const uint8_t btn_b = 6;//Variável com o valor do pino do botão B da placa
const uint8_t btn_j = 22;//Variável com o valor do pino do botão do joystick

//Declaração das varáveis para controle do led RGB
const uint8_t led_red_pino = 13;//Variável com o valor do pino do led vermelho
const uint8_t led_blue_pino = 12;//Variável com o valor do pino do led azul
const uint8_t led_green_pino = 11;//Variável com o valor do pino do led verde

//struct repeating_timer debounce_timer;

//Declaração da variável para controle do debounce dos botões
static volatile uint32_t ultimo_tempo = 0;//Variável que armazena quando foi a última vez pressionado

//Vetores com os números de 0 a 9 que serão utilizados para manipular a matriz de led
double segundo1 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.1, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0};

double segundo2 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.0, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo3 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo4 [] = { 0.0, 0.1, 0.0, 0.0, 0.0,
                       0.1, 0.1, 0.1, 0.1, 0.1,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.0, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.0, 0.0};

double segundo5 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.0, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo6 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.0, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo7 [] = { 0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.1, 0.0, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo8 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo9 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.0, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

double segundo0 [] = { 0.0, 0.1, 0.1, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.0, 0.1, 0.0,
                       0.0, 0.1, 0.1, 0.1, 0.0};

//Protótipo das funções do projeto (A explicação de cada função estará logo acima dela, mais abaixo no código)
void iniciar_pinos();
bool alternar_led(struct repeating_timer *t);
void gpio_irq_handler(uint gpio, uint32_t events);
void numero_escolhido(uint8_t escolha, PIO pio, uint sm);
void animacao_matriz(double *frame, PIO pio, uint sm, uint8_t red, uint8_t green, uint8_t blue);

//Função principal
int main(){
    //Inicializando os pinos e a biblioteca stdio
    iniciar_pinos();
    stdio_init_all();

    //Configurações da matriz de led
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2818b_program);
    ws2818b_program_init(pio, sm, offset, matriz_pino, 800000);

    //Configurando os botões para gerar as interrupções
    gpio_set_irq_enabled_with_callback(btn_a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(btn_b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(btn_j, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Configurações para acionamento do led vermelho
    struct repeating_timer timer;//Declaração de uma variável do tipo struct repeating_timer
    //Criação de um temporizador para chamar a função alternar_led a cada 100 milissegundos
    add_repeating_timer_ms(-100, alternar_led, NULL, &timer);

    //Loop principal do código para manipulação da matriz e desligamento dos leds azul e verde
    while (true)
    {
        numero_escolhido(valor_matriz, pio, sm);//Aqui chamamos a função selece
        gpio_put(led_blue_pino, false);
        gpio_put(led_green_pino, false);
    }
    return 0;
}

//Função para inicializar os pinos da placa
void iniciar_pinos(){
    //Pinos dos botões
    gpio_init(btn_a);
    gpio_init(btn_b);
    gpio_init(btn_j);
    gpio_set_dir(btn_a, GPIO_IN);
    gpio_set_dir(btn_b, GPIO_IN);
    gpio_set_dir(btn_j, GPIO_IN);
    gpio_pull_up(btn_a);
    gpio_pull_up(btn_b);
    gpio_pull_up(btn_j);

    //Pinos do led RGB
    gpio_init(led_red_pino);
    gpio_init(led_blue_pino);
    gpio_init(led_green_pino);
    gpio_set_dir(led_red_pino, GPIO_OUT);
    gpio_set_dir(led_blue_pino, GPIO_OUT);
    gpio_set_dir(led_green_pino, GPIO_OUT);
}

//Função chamada pelo temporizador que a cada 100 milissegundos altera o led resultado em piscar 5 vezes por segundo
bool alternar_led(struct repeating_timer *t){
    static bool estado_led = false;/*Declração de uma variável do tipo bool, além de está com static para
    manter seu valor entre as chamadas da função*/
    estado_led = !estado_led;/*Alterando o valor da variável para o seu oposto (se for true vai virar false,
    e se for false vai virar true)*/
    gpio_put(led_red_pino, estado_led);//Alteração do estado do led
    return true;//A função sempre retornará true caso contrário, deixará de funcionar.
}

//Função de interrupção do código
void gpio_irq_handler(uint gpio, uint32_t events){
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());/*Declaração de uma variável para armazenar
    o tempo desde do início do sistema*/
    
    /*Neste IF está sendo validado se o tempo_atual menos o ultimo_tempo é maior que 200 milissegundos.
    Caso seja verdade, entrará no IF. Esse método é utilizado para conter o debounce dos botões.*/
    if(tempo_atual - ultimo_tempo > 200000){
        ultimo_tempo = tempo_atual;//Aqui passamos o tempo_atual para o ultimo_tempo
        if(gpio == 5){//Validação para saber se foi o botão 5 que foi pressionado
            gpio_put(led_blue_pino, true);//Liga o led azul para feedback visual
            if(valor_matriz != 0){//Aqui realizo outra validação para sabe se ainda pode decrementar a variável valor_matriz
                valor_matriz--;
            }
            printf("BTN_A\nNumero: %d\n", valor_matriz);//Printf utilizado para monitoramento na porta serial
        }
        else if(gpio == 6){//Validação para saber se foi o botão 6 que foi pressionado
            gpio_put(led_green_pino, true);
            if(valor_matriz != 9){//Aqui realizo outra validação para sabe se ainda pode incrementar a variável valor_matriz
                valor_matriz++;
            }
            printf("BTN_B\nNumero: %d\n", valor_matriz);
        }
        else if(gpio == 22){//Validação para saber se foi o botão 22 que foi pressionado
            printf("BTN_J\n");
            reset_usb_boot(0, 0);//Usando o botão para entrada mais fácil no modo BOOTSEL
        }
    }
}

//Função que controla qual número será exibido na matriz
void numero_escolhido(uint8_t escolha, PIO pio, uint sm){
    //Aqui, com base no número que estiver na variável ESCOLHA será o valor impresso na matriz
    if(escolha == 1){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo1, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 2){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo2, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 3){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo3, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 4){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo4, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 5){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo5, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 6){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo6, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 7){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo7, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 8){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo8, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 9){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo9, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
    else if(escolha == 0){
        for(uint8_t i = 0; i < 1; i++){
            animacao_matriz(segundo0, pio, sm, 0, 0, 200);
            sleep_ms(500);
        }
    }
}

//Função responsável por acionar os leds da matriz.
void animacao_matriz(double *frame, PIO pio, uint sm, uint8_t red, uint8_t green, uint8_t blue){
    //FOR utilizado para percorre todos os 25 led da matriz
    for(uint8_t i = 0; i < 25; i++){
        uint8_t BLUE = 0, RED = 0, GREEN = 0;

        /*Aqui o código pega os valores que está sendo passando para a função e passa para varíáveis locais,
        com o calculo sendo a multiplicação das variáveis externas da função com o vetor vetor global*/
        BLUE = (uint8_t) (blue * frame[i]);
        RED = (uint8_t) (red * frame[i]);
        GREEN = (uint8_t) (green * frame[i]);

        //Aqui serão ligados os leds da matriz
        pio_sm_put_blocking(pio, sm, GREEN);
        pio_sm_put_blocking(pio, sm, RED);
        pio_sm_put_blocking(pio, sm, BLUE);
    }
}