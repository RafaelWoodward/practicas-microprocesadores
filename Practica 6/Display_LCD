#include <stdio.h>
#include <stdlib.h>
#include <xc.h>         // Biblioteca principal del compilador XC8
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACION DE BITS DE CONFIGURACION (FUSES)
//=============================================================================

// Seleccion de oscilador (usar XT si estos usando un cristal de 4 MHz)
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

//pines que se usan del puerto C para conectar el LCD
void main(void){
    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7}; // PORT, RS, EN, D4, D5, D6, D7
    
    LCD_Init(lcd);
    
    while(1){
        LCD_Clear(); // elimina todo lo que esta en el LCD
        LCD_Set_Cursor(0,0); // coordenadas del cursor donde va a empesar el mensaje (filas, columnas))
        LCD_putrs(" HELLO WORLD! "); // se usan " en lugar de '' porque no es solo un character
        // putrs es para una cadena de texto
        
        LCD_Set_Cursor(1,0);
        for (char c = 'A'; c < 'Q'; c++){
            LCD_putc(c); // putc para un caracter y luego avanza al siguiente
            __delay_ms(300); // Cada vez que sobreescribimos un valor hay que borrar lo que hay en la LCD con clear porque se puede quedar parte del numero
        }
        __delay_ms(1000);
    }
}
