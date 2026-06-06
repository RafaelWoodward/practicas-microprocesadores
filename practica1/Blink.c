// Microcontroladores 
// Verano 2026
// Practica 1: Actividad en clase
// Emiliano Gonzalez y Rafael Woodward 

//=============================================================================
// ESTADO INICIAL
//=============================================================================

#include <xc.h>         // Biblioteca principal del compilador XC8

//=============================================================================
// CONFIGURACIÓN DE BITS DE CONFIGURACIÓN (FUSES)
//=============================================================================

// Seleccion de oscilador (usar XT si estás usando un cristal de 8 MHz)
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
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

// Frecuencia del oscilador (para __delay_ms y __delay_us)
#define _XTAL_FREQ 8000000    

void main (void){
    TRISD = 0b00000000;
    PORTD = 0b00000000;
    
    while(1){
        PORTD = 0xf;
        __delay_ms(500);
        PORTD = 0x0;
        __delay_ms(500);
    }
}
