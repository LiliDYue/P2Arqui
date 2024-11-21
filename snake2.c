#include "ripes_system.h"
#include <stdio.h>

#define led_matrix_height LED_MATRIX_0_HEIGHT
#define led_matrix_width LED_MATRIX_0_WIDTH

// Se almacena la direccion inicio de la matriz
volatile unsigned int *led_base = (volatile unsigned int *)LED_MATRIX_0_BASE;

// Se almacena la direccion de los D-Pads
volatile unsigned int *d_pad_up = (volatile unsigned int *)D_PAD_0_UP;
volatile unsigned int *d_pad_do = (volatile unsigned int *)D_PAD_0_DOWN;
volatile unsigned int *d_pad_le = (volatile unsigned int *)D_PAD_0_LEFT;
volatile unsigned int *d_pad_ri = (volatile unsigned int *)D_PAD_0_RIGHT;

volatile unsigned int *snake_body[(led_matrix_height*2) * led_matrix_width*2];
volatile unsigned int *snake_head = 0;
volatile unsigned int *apple = 0;

void srand(unsigned int seed);
int rand();
void wait(int ciclos);
void initSnake();
void clearLedMatrix();
void generateApple();
int movement(int d_pad, int direction, int oposite);
void move(int up, int down, int right, int left);
void moveUp();
void moveDown();
void moveLeft();
void moveRight();
void updateSnake();
int isAppleEaten();
void eatApple();
int isSnakeBody(int x, int y);
int isOutsideBoard(int x, int y);

unsigned int next = 1;
int in_game = 1;
int last_move = 0;
int score = 2; //score funciona como el tama?o de la serpiente
int x_coord = 0;
int y_coord = 0;

void main()
{
    clearLedMatrix();
    initSnake();
    generateApple();

    while (in_game)
    {
        move(*d_pad_up, *d_pad_do, *d_pad_ri, *d_pad_le);
        eatApple();
        printf("\nsnake_head in (%d, %d)\n", x_coord, y_coord);
        printf("\nscore: %d", score);
        wait(4000);
    }
    


    printf("Game Over");
}

// verifica si se come una manzana, se llama en el ciclo while
void eatApple() {
    if (isAppleEaten()) { //verifica si estamos arriba de una manzana
         // aumenta la longitud de la serpiente solo si no ha alcanzado el tama?o m?ximo
        if (score < led_matrix_height * led_matrix_width) {
            score++; //score funciona como el tama?o de la serpiente
            score++;
        }
        generateApple(); // genera una nueva manzana despues de ser comida
    }
}

int isAppleEaten() { //cuando la cabeza de la serpiente toca la manzana
    return snake_head == apple;
}

void generateApple(){ //genera la manzana
    srand(next); //inicializa la semilla para generar numeros aleatorios
    int y_rand, x_rand;

    do {
        // genera coordenadas aleatorias para la manzana
        y_rand = rand() % led_matrix_height;
        x_rand = rand() % led_matrix_width;
        //verifica que no aparezcan en el cuerpo de la serpiente o fuera del tablero
    } while (isSnakeBody(x_rand, y_rand) || isOutsideBoard(x_rand, y_rand));

    // asigna la posici?n de la manzana y la pinta de verde
    apple = led_base + (led_matrix_width * y_rand) + x_rand;
    *apple = 0x00FF00;
    next += 1; // se incrementa para la siguiente vez que se genere una manzana aleatoria
}

//verifica si las coordenadas de la manzana se encuentran en el cuerpo de la serpiente
int isSnakeBody(int x, int y) {
    for (int i = 0; i < score; i++) { //recorre el cuerpo de la serpiente y si se encuentra coincidencia retorna 1
        if (snake_body[i] == led_base + (led_matrix_width * y) + x) {
            return 1;
        }
    }
    return 0; // si no encuentra coincidencia
}

// Verifica si las coordenadas (x, y) est?n fuera del tablero
int isOutsideBoard(int x, int y) {
    return x < 0 || x >= led_matrix_width || y < 0 || y >= led_matrix_height;
}

//actualiza los leds del tablero para representar a la serpiente
void updateSnake()
{
    if (score) { //verifica que la serpiente exista
        for (int i = score - 1; i > 0; i--) {
            // Guarda la posici?n actual de la cabeza como el siguiente segmento del cuerpo
            snake_body[i] = snake_body[i - 1]; // guarda cada pedazo de la serpiente
        }
    }
    snake_body[0] = snake_head; //establece que el primer pedazo de serpiente es la cabeza

    // actualiza la cabeza a la nueva posici?n
    snake_head = led_base + (led_matrix_width * y_coord) + x_coord;

    //verifica si la cabeza de la serpiente colisiona con su propio cuerpo
    for (int i = 1; i < score; i++) {
        if (snake_head == snake_body[i]) {
            in_game = 0; // si la serpiente se ha tocado a s? misma, el juego termina
            return;
        }
    }

    // enciende la cabeza de la serpiente
    *snake_head = 0xFF0000;

    // si la longitud de la serpiente es mayor que el puntaje, apaga el ?ltimo led
    if (score) {
        *snake_body[score - 1] = 0x000000; // apaga el led
        snake_body[score - 1] = 0; // elimina la referencia al led apagado del cuerpo
    }

    // enciende los leds de cada pedazo de la serpiente
    for (int i = 1; i < score && i < score; i++) {
        *snake_body[i] = 0xFF0000; // color rojo
    }
}


int inBoard() //verifica si la serpiente esta dentro del tablero con las coordenadas de su cabeza
{
    if (((x_coord >= 0) && (x_coord <= (led_matrix_width - 1))) && (y_coord >= 0) && (y_coord <= (led_matrix_height - 1)))
    {
        return 1;
    }
    in_game = 0;
    return 0;
}

void initSnake() //inicializa la serpiente con un solo led
{
    snake_head = (volatile unsigned int *)led_base;
    *(int *)snake_head = 0xFF0000;
}

int movement(int d_pad, int direction, int oposite) //no permite movimientos a la direccion contraria
{
    if ((d_pad == 1) || (last_move == direction)) //si se presiona un boton del d pad
    {
        if ((last_move != oposite) || (score == 1))// verifica la direccion y no permite un movimietno al lado contrario
        {
            return 1;
        }
    }
    return 0;
}

void move(int up, int down, int right, int left) //mueve a la serpiente
{
    //antes de mover la serpiente, apaga el ?ltimo segmento del cuerpo
    if (score) {
        *snake_body[score - 1] = 0x000000;
    }


    if (movement(up, 1, 2)) //movimiento arriba
    {
        moveUp();
        last_move = 1;
    }

    if (movement(down, 2, 1)) //movimiento abajo
    {
        moveDown();
        last_move = 2;
    }

    if (movement(left, 3, 4)) //movimiento izquierda
    {
        moveLeft();
        last_move = 3;
    }

    if (movement(right, 4, 3)) //movimiento derecha
    {
        moveRight();
        last_move = 4;
    }
}

void moveUp() //mueve la serpiente
{
    y_coord -= 1;//modifica la coordenada necesaria
    if (inBoard()) //verifica que este dentro del tablero
    {
        updateSnake();//actualiza los leds
        *snake_head = 0xFF0000;//enciende el led de la nueva cabeza
    }
}

void moveDown()
{
    y_coord += 1;//solo cambia coordenada
    if (inBoard())
    {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}

void moveLeft()
{
    x_coord -= 1;//solo cambia coordenada
    if (inBoard())
    {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}

void moveRight()
{
    x_coord += 1;//solo cambia coordenada
    if (inBoard())
    {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}

void clearLedMatrix() //limpia tablero recorriendolo y apagando todo
{
    for (int i = 0x0; i < led_matrix_height; i++)
    {
        for (int j = 0x0; j < led_matrix_width; j++)
        {
            *(led_base + (led_matrix_width * i) + j) = 0x000000;
        }
    }
}

void srand(unsigned int seed) //inicializacion numeros aleatorios
{
    next = seed;
}

int rand() // trae un numero aleatorio
{
    next = next * 91035733245 + 12945;
    return (unsigned int)(next / 4329736) % 3268;
}

void wait(int ciclos) //wait 
{
    for (int i = 0; i < ciclos; i++)
    {
    }
}