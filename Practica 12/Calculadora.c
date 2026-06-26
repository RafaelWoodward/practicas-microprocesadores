 // Calculadora basica con teclado matricial 4x4 y LCD 16x2.
 // Mapa de operaciones: A = (/), B = (x), C = (-), D = (+)
 // Flujo (maquina de estados):
 //  
 //  ESTADO_NUM1      -> digitos 0-9, '#' borra, '*' confirma -> ESTADO_OPERACION
 //  ESTADO_OPERACION -> elegir con A/B/C/D, '*' confirma -> ESTADO_NUM2
 //  ESTADO_NUM2      -> digitos 0-9, '#' borra, '*' confirma -> ESTADO_RESULTADO
 //  ESTADO_RESULTADO -> '*' reinicia -> ESTADO_NUM1

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"
#include "keypad.h"

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
#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (cristal HS = 8 MHz)

// Estados de la maquina de estados de la calculadora
#define ESTADO_OPERACION    0
#define ESTADO_NUM1         1
#define ESTADO_NUM2         2
#define ESTADO_RESULTADO    3

// Codigos de operacion
#define OP_DIVISION         0 // A
#define OP_MULTIPLICACION   1 // B
#define OP_RESTA            2 // C
#define OP_SUMA             3 // D

// VARIABLES GLOBALES
unsigned char estado_actual = ESTADO_NUM1;
unsigned char operacion_elegida = OP_SUMA;

// Los numeros se acumulan como texto para poder borrar digitos con '#'
// y se convierten a entero solo cuando se necesita calcular
char buffer_num1[7] = "";
char buffer_num2[7] = ""; // Estos buffer son la cantidad de numeros que puedo poner
unsigned char pos_num1 = 0;
unsigned char pos_num2 = 0;

// PROTOTIPOS - Se utilizan para avisar que se utilizaran estas funciones
void Sistema_Init(void);
void Mostrar_Operacion(void);
void Mostrar_Num1(void);
void Mostrar_Num2(void);
void Mostrar_Resultado(void);
void Procesar_Tecla(char tecla);
void Reiniciar_Calculadora(void);
long Texto_A_Numero(char *texto);
void Numero_A_Texto(long numero, char *destino);

// FUNCION PRINCIPAL
void main(void) {
    Sistema_Init(); // Activa el LCD y el Keypad
    Mostrar_Num1();
            
    while (1) {
        // switch_press_scan() hace polling hasta que el usuario presione una tleca
        char tecla = switch_press_scan();
        Procesar_Tecla(tecla);

    }
}

// INICIALIZACION DEL SISTEMA LCD y Keypad
void Sistema_Init(void) {
    LCD lcd_local = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd_local);

    InitKeypad();
}

// PROCESAMIENTO DE TECLAS SEGUN EL ESTADO ACTUAL
void Procesar_Tecla(char tecla) {
    switch (estado_actual) {
        //-----------------------------------------------------------
        case ESTADO_OPERACION: // Cambia la operación a utilizar hasta confirmar

            if (tecla == 'A') {
                operacion_elegida = OP_DIVISION;
                Mostrar_Operacion();
            }
            else if (tecla == 'B') {
                operacion_elegida = OP_MULTIPLICACION;
                Mostrar_Operacion();
            }
            else if (tecla == 'C') {
                operacion_elegida = OP_RESTA;
                Mostrar_Operacion();
            }
            else if (tecla == 'D') {
                operacion_elegida = OP_SUMA;
                Mostrar_Operacion();
            }
            else if (tecla == '*') {
                estado_actual = ESTADO_NUM2;
                Mostrar_Num2();
            }

            break; // A menos que se confirme con * regresa el codigo a este estado

        //-----------------------------------------------------------
        case ESTADO_NUM1: // Ingresa el numero 1

            if (tecla >= '0' && tecla <= '9') {
                if (pos_num1 < (sizeof(buffer_num1) - 1)) { // Verifica que haya espacio limite 6 numeros
                    buffer_num1[pos_num1] = tecla;
                    pos_num1++;
                    buffer_num1[pos_num1] = '\0';
                    Mostrar_Num1();
                }
            }
            else if (tecla == '#') {
                if (pos_num1 > 0) {
                    pos_num1--;
                    buffer_num1[pos_num1] = '\0'; // Borra el ultimo digito
                    Mostrar_Num1();
                }
            }
            else if (tecla == '*') {
                estado_actual = ESTADO_OPERACION;
                Mostrar_Operacion();
            }

            break;

        //-----------------------------------------------------------
        case ESTADO_NUM2:
            if (tecla >= '0' && tecla <= '9') {
                if (pos_num2 < (sizeof(buffer_num2) - 1)) {
                    buffer_num2[pos_num2] = tecla;
                    pos_num2++;
                    buffer_num2[pos_num2] = '\0';
                    Mostrar_Num2();
                }
            }
            else if (tecla == '#') {
                if (pos_num2 > 0) {
                    pos_num2--;
                    buffer_num2[pos_num2] = '\0';
                    Mostrar_Num2();
                }
            }
            else if (tecla == '*') {
                estado_actual = ESTADO_RESULTADO;
                Mostrar_Resultado();
            }

            break;

        //-----------------------------------------------------------
        case ESTADO_RESULTADO:

            if (tecla == '*') {
                Reiniciar_Calculadora();
            }
            break;
    }
}

// REINICIO DE LA CALCULADORA
void Reiniciar_Calculadora(void) {
    estado_actual = ESTADO_NUM1;

    pos_num1 = 0;
    buffer_num1[0] = '\0';

    pos_num2 = 0;
    buffer_num2[0] = '\0';

    Mostrar_Num1();
}

// PANTALLAS
void Mostrar_Operacion(void) {
    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("Operacion:");

    LCD_Set_Cursor(1, 0);
    switch (operacion_elegida) {
        case OP_DIVISION:
            LCD_putrs("Div (/)");
            break;
        case OP_MULTIPLICACION:
            LCD_putrs("Multi(x)");
            break;
        case OP_RESTA:
            LCD_putrs("Rest (-)");
            break;
        case OP_SUMA:
            LCD_putrs("Sum (+)");
            break;
    }
}

void Mostrar_Num1(void) {
    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("1er Num:");
    LCD_Set_Cursor(1, 0);
    LCD_putrs(buffer_num1);
}

void Mostrar_Num2(void) {
    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("2do Num:");
    LCD_Set_Cursor(1, 0);
    LCD_putrs(buffer_num2);
}

void Mostrar_Resultado(void) {
    long num1 = Texto_A_Numero(buffer_num1);
    long num2 = Texto_A_Numero(buffer_num2);
    char buffer_resultado[16];

    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("Resultado:");
    LCD_Set_Cursor(1, 0);

    switch (operacion_elegida) {

        case OP_SUMA: {
            long resultado = num1 + num2;
            Numero_A_Texto(resultado, buffer_resultado);
            LCD_putrs(buffer_resultado);
            break;
        }

        case OP_RESTA: {
            long resultado = num1 - num2;
            Numero_A_Texto(resultado, buffer_resultado);
            LCD_putrs(buffer_resultado);
            break;
        }

        case OP_MULTIPLICACION: {
            long resultado = num1 * num2;
            Numero_A_Texto(resultado, buffer_resultado);
            LCD_putrs(buffer_resultado);
            break;
        }

        case OP_DIVISION:
        {
            if (num2 == 0) {
                LCD_putrs("Error: /0");
            } else {
                unsigned long resultado_x100 = ((unsigned long) num1 * 100UL) / (unsigned long) num2;
                long parte_entera = resultado_x100 / 100;
                long parte_decimal = resultado_x100 % 100;

                char texto_entero[8];
                char texto_decimal[4];

                Numero_A_Texto(parte_entera, texto_entero);
                Numero_A_Texto(parte_decimal, texto_decimal);

                LCD_putrs(texto_entero);
                LCD_putc('.');

                if (parte_decimal < 10) {
                    LCD_putc('0');
                }

                LCD_putrs(texto_decimal);
            }
            break;
        }
    }
}

// CONVIERTE UN TEXTO (DIGITOS) A UN NUMERO LONG
long Texto_A_Numero(char *texto) {
    long resultado = 0;

    for (int i = 0; texto[i] != '\0'; i++) {
        resultado = (resultado * 10) + (texto[i] - '0');
    }

    return resultado;
}

// CONVIERTE UN NUMERO LONG A TEXTO
void Numero_A_Texto(long numero, char *destino) {
    char temporal[8];
    unsigned char i = 0;
    unsigned char j = 0;
    bool es_negativo = false;

    // El unico caso que puede mandar un numero negativo aqui
    // es el resultado de una resta (num1 - num2 con num1 < num2)
    if (numero < 0) {
        es_negativo = true;
        numero = -numero;
    }

    if (numero == 0) {
        temporal[i] = '0';
        i++;
    }

    // Se extraen los digitos de derecha a izquierda
    while (numero > 0) {
        temporal[i] = (numero % 10) + '0';
        i++;
        numero = numero / 10;
    }

    j = 0;

    if (es_negativo) {
        destino[j] = '-';
        j++;
    }

    // Se invierte el orden para que quede de izquierda a derecha
    while (i > 0) {
        i--;
        destino[j] = temporal[i];
        j++;
    }

    destino[j] = '\0';
}
