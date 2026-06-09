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
// DEFINICIONES
//=============================================================================
#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (para __delay_ms y __delay_us)
unsigned char patron[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
}; 
void main(void){
    TRISD = 0;
    TRISC = 0;
    int num = 0;
    while(1){
        int dec = (num / 10) % 10;   // Extraemos las decenas
        int uni = num % 10;          // Extraemos las unidades
        
        // Iniciar la multiplexacion
        for(int i = 0; i < 10; i++){
            // Mostrar las decenas
            PORTC = 0b11111101;
            PORTD = patron[dec];
            __delay_ms(1);
            
            // Mostrar las unidades
            PORTC = 0b11111110;
            PORTD = patron[uni];
            __delay_ms(1);
        }
        num++;
        
        if(num == 100){
            num = 0;
        }
    }
}
