// Clase
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>         // Biblioteca principal del compilador X<C8
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACI?N DE BITS DE CONFIGURACI?N (FUSES)
//=============================================================================

// Selecci?n de oscilador (usar XT si est?s usando un cristal de 4 MHz)
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

volatile unsigned int tiempo   = 0;
volatile unsigned int contador = 0;
char exec[6];

void Timer1_Init() {
    T1CON = 0b00110001; // prescaler 1:8, reloj interno, Timer1 ON

    TMR1H = 0xF6;
    TMR1L = 0x3C;

    TMR1IE = 1; // Interrupcion de Timer 1
    PEIE   = 1; // Interrupciones de perifericos 
    GIE    = 1; // Interrupciones globales
}

void __interrupt() ISR(void) {
    if (TMR1IF) {
        contador++;
        if (contador >= 100) {   // 100 interrupciones x 10ms = 1 segundo
            tiempo++;
            contador = 0;
        }

        // Recargar el valor inicial en ambos registros
        // Es obligatorio igual que en Timer 0, el timer arranca desde 0 al desbordar
        TMR1H = 0xF6;
        TMR1L = 0x3C;

        TMR1IF = 0; // Limpia bandera
    }
}

void main(void) {
    Timer1_Init();

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("Tiempo: ");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("00:00");

    while(1) {
        LCD_Set_Cursor(1, 0);
        sprintf(exec, "%02u:%02u", tiempo/60, tiempo%60);
        LCD_putrs(exec);
    }
}
