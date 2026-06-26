#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

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
#define _XTAL_FREQ 8000000 // Frecuencia del oscilador (cristal HS = 8 MHz)

#define UART_OK     '1' // Byte recibido cuando la contraseña fue correcta
#define UART_FAIL   '0' // Byte recibido cuando la contraseña fue incorrecta

#define LED_VERDE   RB0 // LED de acceso correcto
#define LED_ROJO    RB1 // LED de alerta

// VARIABLES GLOBALES
volatile char dato_recibido = 0;
volatile bool dato_disponible = false;

// PROTOTIPOS  - FUNCIONES A UTILIZAR
void Init_UART(void);
void Mostrar_Acceso(void);
void Mostrar_Alerta(void);

// FUNCIÓN PRINCIPAL
void main(void) {

    LCD lcd_pic2 = { &PORTD, 2, 3, 4, 5, 6, 7 }; // RS=RD2, EN=RD3, D4-D7=RD4-RD7

    ANSELH = 0; // RB0/RB1 son digitales (AN12/AN10 comparten estos pines)
    TRISB0 = 0; // LED verde como salida
    TRISB1 = 0; // LED rojo como salida
    LED_VERDE = 0;
    LED_ROJO = 0;

    Init_UART();
    LCD_Init(lcd_pic2);

    LCD_Clear();
    __delay_ms(5); 
    LCD_putrs("Esperando...");

    while (1) {

        if (dato_disponible) {

            dato_disponible = false;

            if (dato_recibido == UART_OK) {
                Mostrar_Acceso();
            }
            else if (dato_recibido == UART_FAIL) {
                Mostrar_Alerta();
            }

            LCD_Clear();
            __delay_ms(5);
            LCD_putrs("Esperando...");
        }
    }

    return;
}

// FUNCIONES
// Configura el módulo EUSART en modo receptor con interrupción, 9600 baudios, 8N1
void Init_UART(void) {

    TRISC7 = 1; // RX como entrada
    TRISC6 = 0; // TX como salida (no se usa en el Esclavo, pero se deja configurado)

    SPBRG = 51; // Para 9600 baudios @ 8MHz con BRGH = 1
    BRGH = 1; // Baud rate alto (mejor precisión)
    SYNC = 0; // Modo asíncrono
    SPEN = 1; // Habilita el puerto serie (EUSART)
    CREN = 1; // Habilita recepción continua

    RCIE = 1; // Habilita interrupción de recepción UART
    PEIE = 1; // Habilita interrupciones de periféricos
    GIE = 1; // Habilita interrupciones globales
}

// Rutina de interrupción: se activa cuando llega un byte por UART
void __interrupt() ISR(void) {
    if (RCIF) {
        dato_recibido = RCREG; // Leer el dato limpia la bandera RCIF automáticamente
        dato_disponible = true;
    }
}

// Muestra ACCESO en el LCD y enciende el LED verde fijo por 2 segundos
void Mostrar_Acceso(void) {
    LCD_Clear();
    __delay_ms(5);
    LCD_putrs("ACCESO");

    LED_VERDE = 1;
    __delay_ms(2000);
    LED_VERDE = 0;
}

// Muestra ALERTA en el LCD y parpadea el LED rojo durante 3 segundos
void Mostrar_Alerta(void) {
    LCD_Clear();
    __delay_ms(5);
    LCD_putrs("ALERTA");

    for (unsigned char i = 0; i < 6; i++) { // 6 parpadeos de 500ms = 3 segundos
        LED_ROJO = 1;
        __delay_ms(250);
        LED_ROJO = 0;
        __delay_ms(250);
    }
}
