// Microcontroladores
// Verano 2026
// Practica 1 - Reto: Caminata de 6 leds
// Emilio Gonzalez y Rafael Woodward

#include <xc.h>         // Biblioteca principal del compilador XC8

//=============================================================================
// CONFIGURACI N DE BITS DE CONFIGURACI N (FUSES)
//=============================================================================

// Selecci n de oscilador (usar XT si est s usando un cristal de 4 MHz)
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (disabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection (disabled)
#pragma config WRT = OFF        // Flash Program Memory Write Enable (disabled)
#pragma config CP = OFF         // Flash Program Memory Code Protection (disabled)

//=============================================================================
// DEFINICONES
//=============================================================================

// Frecuencia del oscilador (para __delay_ms y __delay_us)
#define _XTAL_FREQ 8000000    

void main(void) {
    TRISD = 0b00000000;
    PORTD = 0b00000000; // Reseteamos todas las salidas en 0
    unsigned char led = 0b00000001;
    while(1){
        for(int i = 0; i < 7; i++){
            PORTD = led;
            __delay_ms(300);
            led = led << 1;
        }
        
         for(int i = 0; i <7; i++){
            PORTD = led;
            __delay_ms(300);
            led = led >> 1;
        }
    }
}
