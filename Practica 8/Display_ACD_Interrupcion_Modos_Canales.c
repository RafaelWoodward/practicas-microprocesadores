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

// VARIABLES GLOBALES
// "volatile" porque la ISR puede modificarlas en cualquier momento
volatile unsigned char canal = 0; // 0 = Canal 1 (RA0), 1 = Canal 2 (RA1)
volatile unsigned char modo  = 0; // 0 = Voltaje, 1 = Porcentaje, 2 = ADC

//=============================================================================
// FUNCIONES ADC
//=============================================================================
void ADC_Init() {
    ANSEL  = 0x03; // RA0 y RA1 como entradas analogicas (2^0 + 2^1 = 3)
    ANSELH = 0x00; // Resto de pines analogicos como digitales
    ADCON0 = 0x01; // Canal AN0 por defecto, ADC habilitado
    ADCON1 = 0x80; // Resultado justificado a la derecha
}

unsigned int ADC_Read(unsigned char channel) {
    __delay_us(5);
    ADCON0 &= 0x83;         // Limpia los bits de seleccion de canal
    ADCON0 |= channel << 2; // Selecciona el canal deseado
    __delay_ms(2);
    GO_nDONE = 1;           // Inicia la conversion
    while(GO_nDONE);        // Espera a que termine
    return ((ADRESH << 8) | ADRESL); // Une los 10 bits del resultado
}

//=============================================================================
// MAIN
//=============================================================================
void main(void) {
    ADC_Init();

    // PORTB: entradas digitales con pull-ups internos
    ANSELH = 0x00;
    OPTION_REG &= 0x7F; // Bit 7 = 0: activa pull-ups internos de PORTB
    WPUB0 = 1;          // Pull-up en RB0 (boton de canal)
    WPUB1 = 1;          // Pull-up en RB1 (boton de modo)
    TRISB = 0xFF;       // PORTB como entradas

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    char buffer[16];

    // CONFIGURACION DE INTERRUPCIONES
    GIE    = 1; // Interrupciones globales habilitadas
    INTE   = 1; // Interrupcion externa por RB0 habilitada
    INTEDG = 0; // Flanco de bajada (boton a GND)

    while(1) {
        // --- Polling de RB1 para cambiar modo ---
        if (RB1 == 0) {
            __delay_ms(30);      // Antirrebote
            if (RB1 == 0) {
                modo = (modo + 1) % 3; // Avanza: 0 -> 1 -> 2 -> 0
                while(RB1 == 0); // Espera a que suelten el boton
            }
        }

        // --- Lectura ADC del canal activo ---
        unsigned long adc = ADC_Read(canal);

        // --- Linea 0: nombre del canal y modo activo ---
        LCD_Clear();
        LCD_Set_Cursor(0, 0);
        if (canal == 0) {
            if      (modo == 0) LCD_putrs("Canal 1 - Volt");
            else if (modo == 1) LCD_putrs("Canal 1 - %   ");
            else                LCD_putrs("Canal 1 - ADC ");
        } else {
            if      (modo == 0) LCD_putrs("Canal 2 - Volt");
            else if (modo == 1) LCD_putrs("Canal 2 - %   ");
            else                LCD_putrs("Canal 2 - ADC ");
        }

        // Linea 1: valor calculado segun el modo
        LCD_Set_Cursor(1, 0);
        if (modo == 0) {
            //VOLTAJE
            unsigned long volt     = (adc * 50000UL) / 1023;
            unsigned int  part_int = volt / 10000;
            unsigned int  part_dec = volt % 10000;
            sprintf(buffer, "%u.%04u V", part_int, part_dec);
        } else if (modo == 1) {
            // PORCENTAJE
            unsigned long pct      = (adc * 10000UL) / 1023;
            unsigned int  part_int = pct / 100;
            unsigned int  part_dec = pct % 100;
            sprintf(buffer, "%u.%02u %%", part_int, part_dec);
        } else {
            // Valor ADC
            sprintf(buffer, "%u", (unsigned int)adc);
        }
        LCD_putrs(buffer);

        __delay_ms(200);
    }
}

// ISR - RUTINA DE ATENCION A INTERRUPCION
// Solo alterna el canal y limpia la bandera. Lo mas corta posible.
void __interrupt() ISR(void) {
    if (INTF) {
        canal = (canal + 1) % 2; // Alterna: 0 -> 1 -> 0
        __delay_ms(30);          // Antirrebote
        INTF = 0;                // Limpia la bandera de interrupcion
    }
}
