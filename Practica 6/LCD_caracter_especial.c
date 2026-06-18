// PRACTICA 6 - LCD con interrupcion externa y caracter personalizado
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACION DE FUSES
//=============================================================================
#pragma config FOSC = XT
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

// VARIABLE GLOBAL DE ESTADO
// La ISR solo la pone en 1. El main la consume: muestra el mensaje
// 3 veces y luego la regresa a 0 por su cuenta automaticamente.
// "volatile" le dice al compilador que esta variable puede cambiar
// en cualquier momento desde la ISR y que no la optimice.
volatile unsigned char interrupcion_activa = 0;

// Crear caracter personalizado en la CGRAM del LCD
// El LCD 16x2 permite guardar hasta 8 caracteres custom en posiciones 0-7.
// Cada caracter se define con 8 bytes (filas), usando solo los 5 bits menos
// significativos de cada byte (la pantalla es de 5 columnas).
void LCD_CreateChar(unsigned char pos, unsigned char *patron_char) {
    // Comando para escribir en CGRAM: 0x40 + (pos * 8)
    LCD_Cmd(0x40 | (pos << 3));

    // Enviamos las 8 filas del caracter
    for (unsigned char i = 0; i < 8; i++) {
        LCD_putc(patron_char[i]);
    }

    // Regresar a escribir en DDRAM (pantalla normal)
    LCD_Cmd(0x80);
}

void main(void) {
    ANSEL  = 0;
    ANSELH = 0;
    OPTION_REG = OPTION_REG & 0b01111111; // Pull-ups internos activados

    TRISB = 0xFF;   // Puerto B como entradas (boton en RB0)
  
    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7}; // PORT, RS, EN, D4, D5, D6, D7
    LCD_Init(lcd);

    // dibujamos un caracter especial fila por fila 
    unsigned char corazon[8] = {0x11, 0x00, 0x0A, 0x15, 0x11, 0x0A, 0x04, 0x11};

    // Guardamos el caracter en la posicion 0 de la CGRAM
    // Para usarlo despues simplemente hacemos LCD_putc(0)
    LCD_CreateChar(0, corazon);

    // CONFIGURACION DE INTERRUPCIONES
    // Igual que en la practica 5: boton en RB0, flanco de bajada
    GIE    = 1;  // Habilitar interrupciones globales
    INTE   = 1;  // Habilitar interrupcion externa (RB0)
    INTEDG = 0;  // Disparar en flanco de bajada (boton presionado a GND)

    while(1) {

        if (interrupcion_activa == 0) {
            // MENSAJE NORMAL: HELLO WORLD 
            LCD_Clear();
            LCD_Set_Cursor(0, 0);
            LCD_putrs(" HELLO WORLD! ");

            LCD_Set_Cursor(1, 0);
            for (char c = 'A'; c < 'Q'; c++) {
                // Si se presiona el boton durante la animacion,
                // salimos del for inmediatamente para responder rapido
                if (interrupcion_activa == 1) break;
                LCD_putc(c);
                __delay_ms(300);
            }
            __delay_ms(500);

        } else {
            // MENSAJE DE INTERRUPCION: PONGANOS 100
            // Se muestra 3 veces y luego regresa automaticamente al Hello World.
            // Cada repeticion muestra el mensaje 2 segundos, luego parpadea
            // brevemente antes de la siguiente repeticion.
            for (unsigned char rep = 0; rep < 3; rep++) {
                LCD_Clear();
                LCD_Set_Cursor(0, 1);   // Fila 0, columna 1 (centrado)
                LCD_putrs("PONGANOS 100");

                LCD_Set_Cursor(1, 4);   // Fila 1, columna 4 (centrado)
                LCD_putc(0);            // Caracter izquierdo
                LCD_putrs(" :) ");
                LCD_putc(0);            // Caracter derecho

                __delay_ms(1000);       

                // Parpadeo rapido entre repeticiones para que se note la transicion
                LCD_Clear();
                __delay_ms(300);
            }
            interrupcion_activa = 0;
        }
    }
}

//=============================================================================
// ISR - RUTINA DE ATENCION A INTERRUPCION
// Lo mas corta posible: solo activa la bandera y limpia INTF.
// El main se encarga de todo el trabajo real.
//=============================================================================
void __interrupt() ISR(void) {
    if (INTF) {
        interrupcion_activa = 1;    // Activar bandera (el main la regresa a 0)
        __delay_ms(30);             // Antirrebote
        INTF = 0;                   // Limpiar bandera de interrupcion
    }
}
