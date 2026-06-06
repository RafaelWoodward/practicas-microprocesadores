// Microcontroladores
// Verano 2026
// Práctica 2 - Reto: Palabra REAL en matriz LED 8x8
// Emilio Gonzaez y Rafael Woodward

#include <xc.h>         // Biblioteca principal del compilador XC8

//=============================================================================
// CONFIGURACI N DE BITS DE CONFIGURACI N (FUSES)
//=============================================================================

// Selecci n de oscilador (usar XT si est s usando un cristal de 4 MHz)
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

// Definir el patron de las filas
unsigned char patrones[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
unsigned char letraR[8] = {0x0, 0xFF, 0xFF, 0x33, 0x73, 0xDF, 0x9F, 0x0};
unsigned char letraE[8] = {0xFF, 0xFF, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB};
unsigned char letraA[8] = {0xFC, 0xFC, 0x33, 0x33, 0x33, 0x33, 0xFC, 0xFC};
unsigned char letraL[8] = {0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0};

void main(void) {
    TRISB = 0; // salida para las filas
    TRISD = 0; // salida para las columnas
    PORTD = 0; // resetear las filas
    PORTB = 0; //`resetear las columnas
    
    while(1){
        // Mostrar X
        for(unsigned int t = 0; t < 100; t++)
        {
            for(char i = 0; i < 8; i++)
            {
                PORTB = (1 << i);
                PORTD = ~patrones[i];
                __delay_ms(5);
            }
        } 
        // Mostrar R
        for(unsigned int t = 0; t < 20; t++)
        {
            for(char i = 0; i < 8; i++)
            {
                PORTB = (1 << i);
                PORTD = ~letraR[i];
                __delay_ms(5);
            }
        }
        // Mostrar E
        for(unsigned int t = 0; t < 20; t++)
        {
            for(char i = 0; i < 8; i++)
            {
                PORTB = (1 << i);
                PORTD = ~letraE[i];
                __delay_ms(5);
            }
        }
        // Mostrar A
        for(unsigned int t = 0; t < 20; t++)
        {
            for(char i = 0; i < 8; i++)
            {
                PORTB = (1 << i);
                PORTD = ~letraA[i];
                __delay_ms(5);
            }
        }
        // Mostrar L
        for(unsigned int t = 0; t < 20; t++)
        {
            for(char i = 0; i < 8; i++)
            {
                PORTB = (1 << i);
                PORTD = ~letraL[i];
                __delay_ms(5);
            }
        }
    }
}
