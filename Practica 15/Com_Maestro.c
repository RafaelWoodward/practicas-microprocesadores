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

#define CONTRASENA_LEN 4
#define UART_OK     '1' // Byte que se envía si la contraseña es correcta
#define UART_FAIL   '0' // Byte que se envía si la contraseña es incorrecta

const char contrasena_correcta[CONTRASENA_LEN + 1] = "A113"; // Contraseña definida en el código

char buffer_ingresado[CONTRASENA_LEN + 1]; // Lo que el usuario va tecleando
unsigned char indice_buffer = 0; // Posición actual dentro del buffer

// PROTOTIPOS-FUNCIONES A LLAMAR EN EL CODIGO
void Init_UART(void);
void UART_Write(char dato);
bool Verificar_Contrasena(void);
void Reiniciar_Intento(void);

// FUNCIÓN PRINCIPAL
void main(void) {
    LCD lcd_pic1 = { &PORTD, 2, 3, 4, 5, 6, 7 }; // RS=RD2, EN=RD3, D4-D7=RD4-RD7

    InitKeypad();
    Init_UART();
    LCD_Init(lcd_pic1);

    LCD_Clear();
    __delay_ms(5); 
    LCD_putrs("INSERTE CODIGO:");
    LCD_Set_Cursor(1, 0);

    while (1) {

        char tecla = switch_press_scan(); // Espera hasta que se presione una tecla (polling)

        if (tecla == '#') {
            // '#' confirma la contraseña ingresada
            if (indice_buffer == CONTRASENA_LEN) {

                if (Verificar_Contrasena()) {
                    UART_Write(UART_OK);
                    LCD_Clear();
                    __delay_ms(5);
                    LCD_putrs("CORRECTA");
                }
                else {
                    UART_Write(UART_FAIL);
                    LCD_Clear();
                    __delay_ms(5);
                    LCD_putrs("INCORRECTA");
                }

                __delay_ms(2000);
                Reiniciar_Intento();
            }
            // Si no se han ingresado los 4 caracteres, '#' no hace nada todavía
        }
        else if (tecla == '*') {
            // '*' se usa para borrar/reiniciar el intento actual
            Reiniciar_Intento();
        }
        else {
            // Cualquier otra tecla (0-9, A-D) se agrega al buffer si hay espacio
            if (indice_buffer < CONTRASENA_LEN) {
                buffer_ingresado[indice_buffer] = tecla;
                indice_buffer++;

                LCD_putc(tecla); // Se muestra la tecla real ingresada
            }
        }
    }

    return;
}


// FUNCIONES
// Configura el módulo EUSART en modo transmisor, 9600 baudios, 8N1
void Init_UART(void) {

    TRISC6 = 0; // TX como salida
    TRISC7 = 1; // RX como entrada (no se usa en el Maestro, pero debe quedar en alto)

    SPBRG = 51; // Para 9600 baudios @ 8MHz con BRGH = 1
    BRGH = 1; // Baud rate alto (mejor precisión)
    SYNC = 0; // Modo asíncrono
    SPEN = 1; // Habilita el puerto serie (EUSART)
    TXEN = 1; // Habilita transmisión
}

// Envía un solo byte por UART 
void UART_Write(char dato) {
    while (!TXIF); // Espera hasta que el buffer de transmisión esté vacío
    TXREG = dato;
}

// Compara el buffer ingresado contra la contraseña correcta
bool Verificar_Contrasena(void) {

    for (unsigned char i = 0; i < CONTRASENA_LEN; i++) {
        if (buffer_ingresado[i] != contrasena_correcta[i]) {
            return false;
        }
    }
    return true;
}

// Limpia el buffer e índice, y vuelve a mostrar la pantalla de ingreso
void Reiniciar_Intento(void) {

    indice_buffer = 0;
    for (unsigned char i = 0; i < CONTRASENA_LEN; i++) {
        buffer_ingresado[i] = '\0';
    }

    LCD_Clear();
    __delay_ms(5);
    LCD_putrs("INSERTE CODIGO:");
    LCD_Set_Cursor(1, 0);
}
