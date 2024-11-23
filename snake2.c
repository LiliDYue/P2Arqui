#include "ripes_system.h"
#include <stdio.h>

#define led_matrix_height LED_MATRIX_0_HEIGHT
#define led_matrix_width LED_MATRIX_0_WIDTH

volatile unsigned int *led_base = (volatile unsigned int *)LED_MATRIX_0_BASE;

volatile unsigned int *d_pad_up = (volatile unsigned int *)D_PAD_0_UP;
volatile unsigned int *d_pad_do = (volatile unsigned int *)D_PAD_0_DOWN;
volatile unsigned int *d_pad_le = (volatile unsigned int *)D_PAD_0_LEFT;
volatile unsigned int *d_pad_ri = (volatile unsigned int *)D_PAD_0_RIGHT;

volatile unsigned int *snake_body[(led_matrix_height * 2) * (led_matrix_width * 2)];
volatile unsigned int *snake_head = 0;
volatile unsigned int *apple;

unsigned int next = 1;
int in_game = 1;
int last_move = 0;
int tamanio = 1;
int x_coord = 0;
int y_coord = 0;
int apple_x = 0;
int apple_y = 0;


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
void drawBlock(int x, int y, int color);
void clearBlock(int x, int y);
void srand(unsigned int seed);
int rand();
void wait(int ciclos);


void main() {
    clearLedMatrix();
    initSnake();
    generateApple();

    while (in_game) {
        move(*d_pad_up, *d_pad_do, *d_pad_ri, *d_pad_le);
        eatApple();
        wait(4000);
    }
}



void initSnake() {
    snake_head = (volatile unsigned int *)led_base;
    drawBlock(x_coord, y_coord, 0xFF0000);
}

void clearLedMatrix() {
    for (int i = 0; i < led_matrix_height; i++) {
        for (int j = 0; j < led_matrix_width; j++) {
            *(led_base + (led_matrix_width * i) + j) = 0x000000;
        }
    }
}


void generateApple() {
    srand(next);

    do {
        apple_y = rand() % (led_matrix_height - 1); // Garantizar espacio para 2x2
        apple_x = rand() % (led_matrix_width - 1);  // Garantizar espacio para 2x2
    } while (isSnakeBody(apple_x, apple_y) || isSnakeBody(apple_x + 1, apple_y) ||
             isSnakeBody(apple_x, apple_y + 1) || isSnakeBody(apple_x + 1, apple_y + 1) ||
             isOutsideBoard(apple_x + 1, apple_y + 1));

    // Dibujar el área de la manzana (2x2)
    drawBlock(apple_x, apple_y, 0x00FF00);
}



int movement(int d_pad, int direction, int oposite) //no permite movimientos a la direccion contraria
{
    if ((d_pad == 1) || (last_move == direction)) //si se presiona un boton del d pad
    {
        if ((last_move != oposite) || (tamanio == 1))// verifica la direccion y no permite un movimietno al lado contrario
        {
            return 1;
        }
    }
    return 0;
}


void move(int up, int down, int right, int left) {
    if (tamanio) {
        int tail_x = (snake_body[tamanio - 1] - led_base) % led_matrix_width;
        int tail_y = (snake_body[tamanio - 1] - led_base) / led_matrix_width;
        clearBlock(tail_x, tail_y);
    }

    if (movement(up, 1, 2)) {
        moveUp();
        last_move = 1;
    }

    if (movement(down, 2, 1)) {
        moveDown();
        last_move = 2;
    }

    if (movement(left, 3, 4)) {
        moveLeft();
        last_move = 3;
    }

    if (movement(right, 4, 3)) {
        moveRight();
        last_move = 4;
    }
}


void moveUp() {
    y_coord -= 1;
    if (inBoard()) {
        updateSnake();
        *snake_head = 0xFF0000;
        
    }
}

void moveDown() {
    y_coord += 1;
    if (inBoard()) {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}

void moveLeft() {
    x_coord -= 1;
    if (inBoard()) {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}

void moveRight() {
    x_coord += 1;
    if (inBoard()) {
        updateSnake();
        *snake_head = 0xFF0000;
    }
}


void updateSnake() {
    if (tamanio) {
        for (int i = tamanio - 1; i > 0; i--) {
            snake_body[i] = snake_body[i - 1];
        }
    }
    snake_body[0] = snake_head;

    snake_head = led_base + (led_matrix_width * y_coord) + x_coord;

    for (int i = 1; i < tamanio; i++) {
        if (snake_head == snake_body[i]) {
            in_game = 0;
            return;
        }
    }

    drawBlock(x_coord, y_coord, 0xFF0000);

    if (tamanio) {
        int tail_x = (snake_body[tamanio - 1] - led_base) % led_matrix_width;
        int tail_y = (snake_body[tamanio - 1] - led_base) / led_matrix_width;

        // Verificar si la cola está en el área de la manzana
        if (!((tail_x >= apple_x && tail_x < apple_x + 2) && 
              (tail_y >= apple_y && tail_y < apple_y + 2))) {
            clearBlock(tail_x, tail_y);
        }

        snake_body[tamanio - 1] = 0;
    }

    for (int i = 1; i < tamanio; i++) {
        int body_x = (snake_body[i] - led_base) % led_matrix_width;
        int body_y = (snake_body[i] - led_base) / led_matrix_width;
        drawBlock(body_x, body_y, 0xFF0000);
    }
}



int isAppleEaten() {
    return (snake_head == led_base + (apple_y * led_matrix_width) + apple_x ||
            snake_head == led_base + (apple_y * led_matrix_width) + (apple_x + 1) ||
            snake_head == led_base + ((apple_y + 1) * led_matrix_width) + apple_x ||
            snake_head == led_base + ((apple_y + 1) * led_matrix_width) + (apple_x + 1));
}


void eatApple() {
    if (isAppleEaten()) {
        if (tamanio < led_matrix_height * led_matrix_width) {
            tamanio+=2;
        }
        generateApple();
    }
}


int isSnakeBody(int x, int y) {
    for (int i = 0; i < tamanio; i++) {
        if (snake_body[i] == led_base + (led_matrix_width * y) + x) {
            return 1;
        }
    }
    return 0;
}


int isOutsideBoard(int x, int y) {
    return x < 0 || x >= led_matrix_width || y < 0 || y >= led_matrix_height;
}


void drawBlock(int x, int y, int color) {
    if (!isOutsideBoard(x, y) && !isOutsideBoard(x + 1, y + 1)) {
        *(led_base + (led_matrix_width * y) + x) = color;
        *(led_base + (led_matrix_width * y) + (x + 1)) = color;
        *(led_base + (led_matrix_width * (y + 1)) + x) = color;
        *(led_base + (led_matrix_width * (y + 1)) + (x + 1)) = color;
    }
}

void clearBlock(int x, int y) {
    // Verificar si el bloque no es parte del área de la manzana
    if (!((x >= apple_x && x < apple_x + 2) && 
          (y >= apple_y && y < apple_y + 2))) {
        drawBlock(x, y, 0x000000); // Limpia solo si no es parte de la manzana
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
    for (int i = 0; i < ciclos; i++);
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
