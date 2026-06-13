#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACION DE FUSES
//=============================================================================
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

//=============================================================================
// DEFINICIONES
//=============================================================================
#define _XTAL_FREQ 8000000

// VARIABLE GLOBAL DE MODO
// Guarda en cual de los 3 estados:
//   0 = Voltaje 
//   1 = Porcentaje
//   2 = Valor ADC 
// "volatile" porque la ISR (Interrupt Service Routine) puede cambiarla en cualquier momento.
volatile unsigned char modo = 0;

//=============================================================================
// FUNCIONES ADC
//=============================================================================
void ADC_Init() {
    ANSEL = 0x01; // Solo RA0 como entrada analogica, el resto digitales
    ADCON0 = 0x81; // Canal AN0, frecuencia de muestreo Fosc/32, ADC habilitado
    ADCON1 = 0x80; // Resultado "right justified": los 2 bits en ADRESH, los 8 bits en ADRESL
}

unsigned int ADC_Read() {
    __delay_us(5);      // Espera a que el capacitor interno de muestreo se cargue
    GO_nDONE = 1;       // Inicia la conversion analogica a digital
    while(GO_nDONE);    // Espera a que termine (el bit se pone en 0 al terminar)
    return((ADRESH << 8) + ADRESL); // Une los 10 bits: ADRESH[1:0] + ADRESL[7:0]
}

void main(void) {
    ADC_Init();

    // PORTB como entradas con pull-up internos para el boton en RB0
    ANSELH = 0;
    OPTION_REG = OPTION_REG & 0b01111111; // Activar pull-ups internos de PORTB
    TRISB = 0xFF;

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    char buffer[10];

    // CONFIGURACION DE INTERRUPCIONES
    GIE = 1;  // Habilitar interrupciones globales
    INTE = 1;  // Habilitar interrupcion externa por RB0
    INTEDG = 0;  // Flanco de bajada (boton conectado a GND)

    while(1) {
        unsigned long adc = ADC_Read();

        LCD_Clear();

        if (modo == 0) {
            // MODO 0
            // Convierte la lectura ADC (0-1023) a voltaje (0.0 -> 5.0 V)
            // Se evita float multiplicando por 50000 para trabajar con enteros por optmizacion.

            unsigned int volt = (adc * 50000) / 1023;
            unsigned int part_int = volt / 10000; // digitos antes del punto
            unsigned int part_dec = volt % 10000; // digitos despues del punto

            LCD_Set_Cursor(0, 0);
            LCD_putrs("Voltaje:");
            LCD_Set_Cursor(1, 0);
            sprintf(buffer, "%u.%04u V", part_int, part_dec);
            LCD_putrs(buffer);

        } else if (modo == 1) {
            // MODO 1
            // Convierte la lectura ADC (0-1023) a porcentaje (0.00 - 100.00 %)
            // Multiplicamos por 10000 para mantener 2 decimales como entero.
            unsigned int pct      = (adc * 10000) / 1023;
            unsigned int part_ent = pct / 100; // parte entera del porcentaje
            unsigned int part_dec = pct % 100; // parte decimal del porcentaje

            LCD_Set_Cursor(0, 0);
            LCD_putrs("Porc.:");
            LCD_Set_Cursor(1, 0);
            sprintf(buffer, "%u.%02u %%", part_ent, part_dec);
            // %% en sprintf produce un solo % en la pantalla
            LCD_putrs(buffer);

        } else {
            // MODO 2
            LCD_Set_Cursor(0, 0);
            LCD_putrs("ADC:");
            LCD_Set_Cursor(1, 0);
            sprintf(buffer, "%u", adc);
            LCD_putrs(buffer);
        }
        __delay_ms(200);
    }
}

// ISR - RUTINA DE ATENCION A INTERRUPCION
// Lo mas corta posible, solo avanza el modo y limpia la bandera.
// El operador % 3 hace que el ciclo sea 0 -> 1 -> 2 -> 0 -> 1 ->, etc
void __interrupt() ISR(void) {
    if (INTF) {
        modo = (modo + 1) % 3;  // Avanza al siguiente modo de forma circular
        __delay_ms(30);         // Para el antirrebote
        INTF = 0;               // Limpia la bandera de interrupcion
    }
}
