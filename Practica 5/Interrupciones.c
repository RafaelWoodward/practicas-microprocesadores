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
#define LED PORTCbits.RC0
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

void blink_LED(){
    for(int i = 0; i < 4; i++){
        LED = 1;
        __delay_ms(500);
        LED = 0;
        __delay_ms(500);
    }
}
void main(void){
    ANSEL = 0;
    ANSELH = 0;
    OPTION_REG = OPTION_REG & 0b01111111;
    
    TRISC = 0;
    TRISD = 0;
    TRISB = 0xFF;
    
    PORTC = 0;
    PORTD = 0; // Estos dos son para evitar que haya un voltaje residual en esos pines
    
    unsigned char count = 0;
    
    GIE = 1; // Activar todas las interrupciones
    INTE = 1;
    INTEDG = 0; // Interrupciones
    
    while(1){
        PORTD = patron[count];
        count = (count + 1)%10;
        __delay_ms(500);
        
    }
}

void __interrupt() ISR(void){
    if(INTF ){ // INTF = Interrupcion externa
        GIE = 0; // Desactivar todas las interrupciones
        blink_LED(); // Ejecutar las funciones de la interrupción
        GIE = 1;
        INTF = 0; // reseteamos la bandera, 
    }
}
