// Actividad: Escribir en la LCD los caracteres del teclado matricial.
 // Cada tecla presionada (0-9, A, B, C, D, *, #) se muestra en la LCD
 // tal cual, llenando la linea 1 y despues la linea 2.
 // Al llegar al final de la linea 2, se vuelve a la posicion (0,0)
 // y se empieza a sobrescribir desde el inicio.

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"
#include "keypad.h"

//=============================================================================
// CONFIGURACIÓN DE BITS DE CONFIGURACIÓN (FUSES)
//=============================================================================
#pragma config FOSC = HS       // Oscillator Selection bits (HS oscillator: cristal 8 MHz)
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
#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (cristal HS = 8 MHz)

#define COLUMNAS_LCD   16       // Cantidad de columnas de la LCD 16x2

// VARIABLES GLOBALES
unsigned char fila_actual = 0;      // 0 = primera linea, 1 = segunda linea
unsigned char columna_actual = 0;   // Posicion dentro de la fila (0 a 15)

// PROTOTIPOS
void Sistema_Init(void);
void Escribir_Caracter(char tecla);


// FUNCION PRINCIPAL
void main(void) {
    Sistema_Init(); // Activa el LCD y el Keypad

    while (1) {
        // switch_press_scan() hace polling hasta que el usuario presione una tecla
        char tecla = switch_press_scan();
        Escribir_Caracter(tecla);
    }
}

// INICIALIZACION DEL SISTEMA LCD y Keypad
void Sistema_Init(void) {
    LCD lcd_local = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd_local);

    InitKeypad();
}

// ESCRIBE LA TECLA PRESIONADA EN LA POSICION ACTUAL DE LA LCD
void Escribir_Caracter(char tecla) {
    LCD_Set_Cursor(fila_actual, columna_actual);
    LCD_putc(tecla);

    columna_actual++;

    if (columna_actual >= COLUMNAS_LCD) {
        columna_actual = 0;
        fila_actual++;

        if (fila_actual > 1) {
            fila_actual = 0;
            LCD_Clear(); // Se llenaron ambas lineas, se reinicia desde el principio
        }
    }
}
