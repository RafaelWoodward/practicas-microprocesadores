#include <stdio.h>
#include <stdlib.h>
#include <xc.h>         // Biblioteca principal del compilador XC8
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACIÓN DE BITS DE CONFIGURACIÓN (FUSES)
//=============================================================================

// Selección de oscilador (usar XT si estás usando un cristal de 4 MHz)
#pragma config FOSC = HS        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (disabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection (disabled)
#pragma config WRT = OFF        // Flash Program Memory Write Enable (disabled)
#pragma config CP = OFF         // Flash Program Memory Code Protection (disabled)

//=============================================================================
// DEFINICIONES
//=============================================================================

#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (para __delay_ms y __delay_us)
#define UMBRAL_X_MIN  462
#define UMBRAL_X_MAX  562
#define UMBRAL_Y 100

// Figuritas - patrones de la matriz 5x8
unsigned char pacman[8]      = {0b01110, 0b11110, 0b11100, 0b11000, 0b11100, 0b11110, 0b01110, 0b00000}; // Normal
unsigned char pacman_inv[8]  = {0b01110, 0b01111, 0b00111, 0b00011, 0b00111, 0b01111, 0b01110, 0b00000}; // Normal - Invertido
unsigned char pacman2[8]     = {0b01110, 0b11111, 0b11111, 0b11110, 0b11111, 0b11111, 0b01110, 0b00000}; // Boca cerrada - Interaccion con SW
unsigned char pacman2_inv[8] = {0b01110, 0b11111, 0b11111, 0b01111, 0b11111, 0b11111, 0b01110, 0b00000}; // Boca cerrada invertido - Interaccion con SW
unsigned char fantasma[8]    = {0b01110, 0b10101, 0b00100, 0b11111, 0b11111, 0b11111, 0b10101, 0b10101}; // Obstaculo
unsigned char explosion[8]   = {0b10001, 0b01010, 0b00000, 0b10001, 0b00000, 0b01010, 0b10001, 0b00000}; // Muerte

// Funciones vistas en clase para inicializar la lectura del voltaje en crudo
void ADC_Init() {
    ANSEL  = 0x03;
    ANSELH = 0x00;
    ADCON0 = 0x01;
    ADCON1 = 0x80;
}

// Funcion para leer el ADC crudo
unsigned int ADC_Read(unsigned char channel) {
    __delay_us(5);
    ADCON0 &= 0x83;
    ADCON0 |= channel << 2;
    __delay_ms(2);
    GO_nDONE = 1;
    while(GO_nDONE);
    return (unsigned int)((ADRESH << 8) | ADRESL);
}

// Creamos una funcion para poner de manera aleatoria el fantasma
unsigned char aleatorio   = 0;
unsigned char f_col  = 0; unsigned char f_fila = 0; // Posicion del fantasma

void generar_fantasma(unsigned char pac_col, unsigned char pac_fila) {
    // Repite hasta encontrar una posicion que no este encima de pacman
    do {
        aleatorio  += 7;
        f_col  = aleatorio % 16;
        f_fila = (aleatorio / 16) % 2;
    } while (f_col == pac_col && f_fila == pac_fila);
}


void main(void) {
    ADC_Init();

    ANSELH = 0x00;
    OPTION_REG &= 0x7F; // Activa los pull-ups internos
    WPUB0 = 1;
    TRISB = 0xFF;

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    // Creamos las figuras con la funcion previamente hecha en clase (Practica 6)
    LCD_CreateChar(1, pacman); // normal
    LCD_CreateChar(2, pacman2); // boca cerrada
    LCD_CreateChar(3, fantasma);
    LCD_CreateChar(4, explosion);
    LCD_CreateChar(5, pacman_inv);  // normal 
    LCD_CreateChar(6, pacman2_inv);  // boca cerrada

    // Variables para la posicion de las figuritas
    unsigned char col      = 0; // Pacman - Posicion inicial x
    unsigned char fila     = 0; // Pacman - Posicion inicial y
    unsigned char figura   = 1; // Fig Normal
    unsigned char col_prev = 0;
    unsigned char fila_prev = 0;
    unsigned char dir      = 1; // Direccion de pacman: 1 = derecha, 0 = izquierda

    generar_fantasma(col, fila); // La entrada es la posicion actual del pacman para evitar que el fantasma este en la misma posicion

    LCD_Clear(); 
    LCD_Set_Cursor(f_fila, f_col); LCD_putc(3); // fantasma
    LCD_Set_Cursor(fila, col);     LCD_putc(1); // pacman

    while(1) {
        aleatorio++; // Para que sea diferente en cada ocasion la pos del fantasma

        unsigned int rvx = ADC_Read(0); // Valor crudo obtenido del potenciometro RVx
        unsigned int rvy = ADC_Read(1); // Valor crudo obtenido del potenciometro RVy

        col_prev  = col; fila_prev = fila; // guardamos posicion pasada para limpiarlas

        // Movimiento en X 
        if (rvx > UMBRAL_X_MAX) {
            dir = 1; // va a la derecha
            unsigned int dif = rvx - UMBRAL_X_MAX;
            unsigned char vel;
            if (dif > 400) {
                vel = 2; // 2 columnas por movimiento
            } else {
                vel = 1; // 1 columna por mov
            }
            if (col + vel > 15) {
                // Teletransporte derecha
                col = 0;
                generar_fantasma(col, fila);
                LCD_Clear();
                LCD_Set_Cursor(f_fila, f_col);
                LCD_putc(3);
            } else {
                col += vel;
            }
        } else if (rvx < UMBRAL_X_MIN) {
            dir = 0; // va a la izquierda
            unsigned int dif = UMBRAL_X_MIN - rvx;
            unsigned char vel;
            if (dif > 400) {
                vel = 2;
            } else {
                vel = 1;
            }
            if (col < vel) {
                // Teletransporte izquierda
                col = 15;
                generar_fantasma(col, fila);
                LCD_Clear();
                LCD_Set_Cursor(f_fila, f_col);
                LCD_putc(3); // figura del fantsma
            } else {
                col -= vel;
            }
        }

        // Cambio de fila en Y 
        if      (rvy > 512 + UMBRAL_Y) fila = 0;
        else if (rvy < 512 - UMBRAL_Y) fila = 1;

        // Boton SW: animacion boca, figura segun direccion actual
        unsigned char fig_nueva;
        if (dir == 1) {
            if (RB0 == 0) {
                fig_nueva = 2; // boca cerrada derecha
            } else {
                fig_nueva = 1; // normal derecha
            }
        } else {
            if (RB0 == 0) {
                fig_nueva = 6; // boca cerrada izquierda
            } else {
                fig_nueva = 5; // normal izquierda
            }
        }

        // Colision: detectada ANTES de dibujar
        if (col == f_col && fila == f_fila) {

            // 1. Borra pacman de su posicion anterior
            LCD_Set_Cursor(fila_prev, col_prev);
            LCD_putc(' ');

            // 2. Fantasma queda visible solo en la celda de colision
            LCD_Set_Cursor(f_fila, f_col);
            LCD_putc(3);
            __delay_ms(300);

            // 3. Explosion reemplaza a pacman al llegar a la celda del fantasma
            LCD_Set_Cursor(fila, col);
            LCD_putc(4);
            __delay_ms(800);

            // GAME OVER
            LCD_Clear();
            LCD_Set_Cursor(0, 3);
            LCD_putrs("GAME  OVER");
            LCD_Set_Cursor(1, 2);
            LCD_putrs("Presiona SW");

            // Espera boton para reiniciar
            while(RB0 == 1);
            __delay_ms(50);
            
            // Reinicio
            col    = 0;
            fila   = 0;
            figura = 1;
            dir    = 1; // reinicia direccion a derecha
            generar_fantasma(col, fila);
            LCD_Clear();
            LCD_Set_Cursor(f_fila, f_col); LCD_putc(3); // Fantasma
            LCD_Set_Cursor(fila, col);     LCD_putc(1); // Pacman
            continue; // Se salta el siguiente if para reiniciar al while principal 
        }

        // Dibuja solo si algo cambio 
        if (col != col_prev || fila != fila_prev || fig_nueva != figura) {
            figura = fig_nueva;

            // Borra posicion anterior de pacman
            LCD_Set_Cursor(fila_prev, col_prev);
            LCD_putc(' ');

            // Si cambio de fila, borra residuo en fila nueva
            if (fila != fila_prev) {
                LCD_Set_Cursor(fila, col_prev);
                LCD_putc(' ');
            }

            // Redibuja fantasma
            if (col_prev == f_col && fila_prev == f_fila) {
                LCD_Set_Cursor(f_fila, f_col); LCD_putc(3);
            }

            // Dibuja pacman en nueva posicion
            LCD_Set_Cursor(fila, col);
            LCD_putc(figura);
        }
        __delay_ms(150); // Velocidad del juego
    }
}
