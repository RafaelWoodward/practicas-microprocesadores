// EXPLICACION PROYECTO
//  Control de Servomotor MOT-100 (Steren) con interfaz Keypad + LCD
//
//  Estados de pantalla:
//    1. MENU -> A:ANG B:RANG / C:VEL D:OPER
//    2. ANGULO (A) -> Mover el servo a un angulo exacto (instantaneo)
//    3. RANGO (B)  -> Definir A1 y A2 (extremos del barrido, 0-180)
//    4. VELOCIDAD (C) -> Definir velocidad de barrido en % (1-100)
//    5. OPER - countdown (D) -> Cuenta 3..2..1 antes de iniciar el barrido
//    6. OPER - corriendo  -> Barrido continuo A1<->A2
// 
//  Navegacion al menu (cada estado usa su propia tecla):
//    Dentro de ANGULO (A)-> 'A' regresa al menu
//    Dentro de RANGO (B) -> 'B' regresa al menu
//    Dentro de VELOCIDAD (C) -> 'C' regresa al menu
//    Dentro de OPER (D) -> 'D' regresa al menu (en countdown o corriendo)
// 
//  Tecla *  -> Confirma valor capturado / Reinicia countdown en OPER detenido
//  Tecla #  -> Borra ultimo digito / Detiene operacion (estado 6)
// 
//  Calibracion hecha por nosotros confirmada del servo MOT-100 (medida con transportador, no es lineal -> ver TABLA_ANGULO/TABLA_PULSO):
//    400us  = 0 grados
//    2204us = 180 grados (tope fisico real, no se puede pasar de aqui)
 
#include <xc.h>
#include <stdbool.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

#include "lcd.h"
#include "keypad.h"

#define SERVO PORTDbits.RD0
#define SERVO_TRIS TRISDbits.TRISD0

// (SERVO_PULSO_MIN/MAX no se usan como formula lineal simple: el
// servo fisico no responde proporcionalmente al pulso. Se usa la tabla
// de calibracion directa de abajo en su lugar.)

// --- Variables del sistema ---
int angulo_actual = 0; // Posicion logica actual del servo (0-180)
int v1 = -1; // Extremo 1 del rango (-1 = no definido)
int v2 = -1; // Extremo 2 del rango (-1 = no definido)
int velocidad_pct = 0; // Velocidad en % (0 = no definido)

// =============================================================================
// PWM / Servo
//
// Tabla de calibracion DIRECTA: angulo real deseado (0-180) -> ancho de pulso en microsegundos que logra ese
// angulo real exacto en el servo fisico. Medida con transportador.
// El servo no responde proporcionalmente al pulso (no es lineal), por
// eso se usa una tabla con interpolacion en vez de una formula simple.
// El ultimo punto (180) ya corresponde al tope fisico real del servo.
// =============================================================================

#define TABLA_CALIBRACION_PUNTOS 15

static const int TABLA_ANGULO[TABLA_CALIBRACION_PUNTOS] = {
    0, 10, 20, 40, 50, 60, 70, 80, 100, 110, 120, 130, 160, 170, 180
}; // Esta tabla la elaboramos en el proceso de creacion del codigo
static const unsigned int TABLA_PULSO[TABLA_CALIBRACION_PUNTOS] = {
    400, 546, 693, 840, 986, 1060, 1133, 1280, 1426, 1573, 1646, 1720, 2013, 2101, 2204
};

// Traduce un angulo real deseado (0-180) al ancho de pulso (en us) que
// logra ese angulo exacto en el servo fisico. Usa interpolacion lineal
// entre los puntos medidos de la tabla.
unsigned int Angulo_A_Pulso(int angulo)
{
    if (angulo <= TABLA_ANGULO[0]) {
        return TABLA_PULSO[0];
    }
    if (angulo >= TABLA_ANGULO[TABLA_CALIBRACION_PUNTOS - 1]) {
        return TABLA_PULSO[TABLA_CALIBRACION_PUNTOS - 1];
    }

    for (unsigned char i = 0; i < TABLA_CALIBRACION_PUNTOS - 1; i++) {
        if (angulo >= TABLA_ANGULO[i] && angulo <= TABLA_ANGULO[i + 1]) {
            int rango_angulo = TABLA_ANGULO[i + 1] - TABLA_ANGULO[i];
            int rango_pulso = (int)(TABLA_PULSO[i + 1] - TABLA_PULSO[i]);
            int offset = angulo - TABLA_ANGULO[i];
            return TABLA_PULSO[i] + (unsigned int)((offset * rango_pulso) / rango_angulo);
        }
    }

    return TABLA_PULSO[TABLA_CALIBRACION_PUNTOS - 1]; // No deberia llegar aqui
}

void Pulso_Servo_us(unsigned int ancho_us)
{
    SERVO = 1;
    while (ancho_us >= 100) {
        __delay_us(100);
        ancho_us -= 100;
    }
    SERVO = 0;
    __delay_ms(18);
}

// Mantiene al servo en un angulo fijo durante aproximadamente ms_total
// (repite el pulso cada ~20ms para que el servo no pierda posicion)
void Mantener_Angulo(int angulo, unsigned int ms_total)
{
    unsigned int pulso = Angulo_A_Pulso(angulo);
    unsigned int ciclos = ms_total / 20;
    if (ciclos == 0) ciclos = 1;
    for (unsigned int i = 0; i < ciclos; i++) {
        Pulso_Servo_us(pulso);
    }
}

// LCD - Utilidades
void LCD_Linea(unsigned char fila, const char *texto)
{
    LCD_Set_Cursor(fila, 0);
    LCD_putrs("                "); // Limpia la linea (16 espacios)
    LCD_Set_Cursor(fila, 0);
    LCD_putrs(texto);
}

// Convierte un entero (0-180) a texto y lo imprime en el LCD
void LCD_Numero(int numero)
{
    char buffer[4];
    unsigned char i = 0;
    bool negativo = false;

    if (numero < 0) {
        negativo = true;
        numero = -numero;
    }

    if (numero == 0) {
        buffer[i++] = '0';
    } else {
        char temp[4];
        unsigned char j = 0;
        while (numero > 0 && j < 3) {
            temp[j++] = (char)(numero % 10) + '0';
            numero /= 10;
        }
        while (j > 0) {
            buffer[i++] = temp[--j];
        }
    }
    buffer[i] = '\0';

    if (negativo) LCD_putc('-');
    LCD_puts(buffer);
}

typedef struct {
    char buffer[4];
    unsigned char len;
} captura_t;

void Captura_Iniciar(captura_t *c)
{
    c->buffer[0] = '\0';
    c->len = 0;
}

int Captura_Valor(captura_t *c)
{
    int valor = 0;
    for (unsigned char i = 0; i < c->len; i++) {
        valor = valor * 10 + (int)(c->buffer[i] - '0');
    }
    return valor;
}

// Muestra "ERROR / EXCEDIDO" centrado por 1 segundo cuando un valor
// confirmado con '*' esta fuera del rango permitido.
void Mostrar_Error_Excedido(void)
{
    LCD_Linea(0, "     ERROR      ");
    LCD_Linea(1, "    EXCEDIDO    ");
    __delay_ms(1000);
}


// ESTADO 1: MENU
void Mostrar_Menu(void)
{
    LCD_Linea(0, "A:ANG B:RANG");
    LCD_Linea(1, "C:VEL D:OPER");
}

// ESTADO 2: ANGULO (A)
// ANGULO: <angulo_actual>
// CALIBRAR: <buffer>
void Refrescar_Pantalla_Angulo(captura_t *c)
{
    LCD_Set_Cursor(0, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(0, 0);
    LCD_putrs("ANGULO: ");
    LCD_Numero(angulo_actual);

    LCD_Set_Cursor(1, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("CALIBRAR: ");
    LCD_puts(c->buffer);
}

void Estado_Angulo(void)
{
    captura_t c;
    Captura_Iniciar(&c);
    Refrescar_Pantalla_Angulo(&c);

    while (1) {
        char tecla = keypad_scanner();
        if (tecla == 'n') continue;

        if (tecla == 'A') return; // Regresa al menu

        if (tecla >= '0' && tecla <= '9') {
            if (c.len < 3) {
                c.buffer[c.len++] = tecla;
                c.buffer[c.len] = '\0';
                Refrescar_Pantalla_Angulo(&c);
            }
        }
        else if (tecla == '#') {
            if (c.len > 0) {
                c.buffer[--c.len] = '\0';
                Refrescar_Pantalla_Angulo(&c);
            }
        }
        else if (tecla == '*') {
            if (c.len == 0) continue;
            int valor = Captura_Valor(&c);

            if (valor > 180) {
                Mostrar_Error_Excedido();
                Captura_Iniciar(&c);
                Refrescar_Pantalla_Angulo(&c);
                continue;
            }

            // Movimiento instantaneo al angulo confirmado
            angulo_actual = valor;
            Mantener_Angulo(angulo_actual, 500);

            Captura_Iniciar(&c);
            Refrescar_Pantalla_Angulo(&c);
        }
        // Teclas B, C, D se ignoran dentro de este estado
    }
}


// ESTADO 3: RANGO (B)
// RANG: <v1> - <v2>
// A1: <buffer1> A2: <buffer2>

void Refrescar_Pantalla_Rango(captura_t *c1, captura_t *c2)
{
    LCD_Set_Cursor(0, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(0, 0);
    LCD_putrs("RANG: ");
    if (v1 == -1 || v2 == -1) {
        LCD_putrs("--- - ---");
    } else {
        LCD_Numero(v1);
        LCD_putrs(" - ");
        LCD_Numero(v2);
    }

    LCD_Set_Cursor(1, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("A1:");
    LCD_puts(c1->buffer);
    LCD_Set_Cursor(1, 8);
    LCD_putrs("A2:");
    LCD_puts(c2->buffer);
}

void Estado_Rango(void)
{
    captura_t c1, c2;
    Captura_Iniciar(&c1);
    Captura_Iniciar(&c2);
    bool capturando_a2 = false; // false = A1 activo, true = A2 activo

    Refrescar_Pantalla_Rango(&c1, &c2);

    while (1) {
        char tecla = keypad_scanner();
        if (tecla == 'n') continue;

        if (tecla == 'B') return; // Regresa al menu

        captura_t *activo = capturando_a2 ? &c2 : &c1;

        if (tecla >= '0' && tecla <= '9') {
            if (activo->len < 3) {
                activo->buffer[activo->len++] = tecla;
                activo->buffer[activo->len] = '\0';
                Refrescar_Pantalla_Rango(&c1, &c2);
            }
        }
        else if (tecla == '#') {
            if (activo->len > 0) {
                activo->buffer[--activo->len] = '\0';
                Refrescar_Pantalla_Rango(&c1, &c2);
            }
        }
        else if (tecla == '*') {
            if (activo->len == 0) continue;
            int valor = Captura_Valor(activo);

            if (valor > 180) {
                Mostrar_Error_Excedido();
                Captura_Iniciar(activo);
                Refrescar_Pantalla_Rango(&c1, &c2);
                continue;
            }

            if (!capturando_a2) {
                // Se confirmo A1, saltar a A2
                capturando_a2 = true;
            } else {
                // Se confirmo A2: guardar A1 y A2 como el rango definitivo
                v1 = Captura_Valor(&c1);
                v2 = valor;
                Captura_Iniciar(&c1);
                Captura_Iniciar(&c2);
                capturando_a2 = false;
            }
            Refrescar_Pantalla_Rango(&c1, &c2);
        }
        // Teclas C, D se ignoran dentro de este estado
    }
}


// ESTADO 4: VELOCIDAD (C)
// L1: VELOCIDAD: <velocidad_pct>
// L2: CALIBRAR: <buffer>%

void Refrescar_Pantalla_Velocidad(captura_t *c)
{
    LCD_Set_Cursor(0, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(0, 0);
    LCD_putrs("VELOCIDAD: ");
    if (velocidad_pct == 0) {
        LCD_putrs("---");
    } else {
        LCD_Numero(velocidad_pct);
    }
    LCD_Set_Cursor(0, 15);
    LCD_putc('%');

    LCD_Set_Cursor(1, 0);
    LCD_putrs("                ");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("CALIBRAR: ");
    LCD_puts(c->buffer);
    LCD_Set_Cursor(1, 15);
    LCD_putc('%');
}

void Estado_Velocidad(void)
{
    captura_t c;
    Captura_Iniciar(&c);
    Refrescar_Pantalla_Velocidad(&c);

    while (1) {
        char tecla = keypad_scanner();
        if (tecla == 'n') continue;

        if (tecla == 'C') return; // Regresa al menu

        if (tecla >= '0' && tecla <= '9') {
            if (c.len < 3) {
                c.buffer[c.len++] = tecla;
                c.buffer[c.len] = '\0';
                Refrescar_Pantalla_Velocidad(&c);
            }
        }
        else if (tecla == '#') {
            if (c.len > 0) {
                c.buffer[--c.len] = '\0';
                Refrescar_Pantalla_Velocidad(&c);
            }
        }
        else if (tecla == '*') {
            if (c.len == 0) continue;
            int valor = Captura_Valor(&c);

            if (valor < 1 || valor > 100) {
                Mostrar_Error_Excedido();
                Captura_Iniciar(&c);
                Refrescar_Pantalla_Velocidad(&c);
                continue;
            }

            velocidad_pct = valor;
            Captura_Iniciar(&c);
            Refrescar_Pantalla_Velocidad(&c);
        }
        // Teclas A, B, D se ignoran dentro de este estado
    }
}

// ESTADO 5: OPER - countdown
// L1: INICIANDO... (centrado)
// L2: 3... 2... 1...  (cada bloque "N..." completo tarda 400ms)

void Estado_Oper_Countdown(void)
{
    LCD_Linea(0, "  INICIANDO...  ");
    LCD_Linea(1, "                ");

    static const char secuencia[] = "3... 2... 1...";
    char buffer[15];
    unsigned char len = 0;

    for (unsigned char i = 0; secuencia[i] != '\0'; i++) {
        buffer[len++] = secuencia[i];
        if (secuencia[i] == ' ') {
            buffer[len++] = secuencia[++i];
        }
        buffer[len] = '\0';

        LCD_Set_Cursor(1, 0);
        LCD_putrs("                ");
        LCD_Set_Cursor(1, 0);
        LCD_puts(buffer);

        __delay_ms(100);
    }
}

// ESTADO 6: OPER - corriendo
// L1: RANG: <v1> - <v2>
// L2: VELOCIDAD: <velocidad_pct>

// Mapeo de velocidad (1-100%) a delay entre pasos de 1 grado (en ms)
// Formula acordada: delay_ms = 50 - (vel * 0.45)
unsigned int Velocidad_A_Delay(int vel)
{
    unsigned long temp = (unsigned long)vel * 45; // vel*0.45 -> (vel*45)/100
    temp = temp / 100;
    return (unsigned int)(50 - temp);
}

typedef enum {
    BARRIDO_COMPLETO,
    BARRIDO_DETENIDO,   // Se presiono '#'
    BARRIDO_MENU        // Se presiono 'D'
} resultado_barrido_t;

resultado_barrido_t Barrer(int desde, int hasta, unsigned int delay_ms)
{
    int paso = (hasta >= desde) ? 1 : -1;
    int angulo = desde;

    while (1) {
        unsigned int pulso = Angulo_A_Pulso(angulo);
        unsigned int ciclos = delay_ms / 20;
        if (ciclos == 0) ciclos = 1;

        for (unsigned int i = 0; i < ciclos; i++) {
            Pulso_Servo_us(pulso);

            char tecla = keypad_scanner();
            if (tecla == '#') {
                return BARRIDO_DETENIDO;
            }
            if (tecla == 'D') {
                return BARRIDO_MENU;
            }
        }

        angulo_actual = angulo;

        if (angulo == hasta) break;
        angulo += paso;
    }
    return BARRIDO_COMPLETO;
}

void Dibujar_Pantalla_Oper(void)
{
    LCD_Linea(0, "RANG: ");
    LCD_Set_Cursor(0, 6);
    LCD_Numero(v1);
    LCD_putrs(" - ");
    LCD_Numero(v2);

    LCD_Linea(1, "VELOCIDAD: ");
    LCD_Set_Cursor(1, 11);
    LCD_Numero(velocidad_pct);
    LCD_Set_Cursor(1, 15);
    LCD_putc('%');
}

void Estado_Oper_Corriendo(void)
{
    unsigned int delay_ms = Velocidad_A_Delay(velocidad_pct);

    Dibujar_Pantalla_Oper();

    int extremo_actual = v1;
    int extremo_siguiente = v2;

    while (1) {
        resultado_barrido_t r = Barrer(extremo_actual, extremo_siguiente, delay_ms);

        if (r == BARRIDO_MENU) {
            return; // 'D': regresar al menu principal
        }
        if (r == BARRIDO_DETENIDO) {
            // Se mantiene en esta pantalla (servo detenido en su posicion
            // actual) esperando '*' para reiniciar el countdown, o 'D'
            // para volver al menu.
            while (1) {
                char tecla = keypad_scanner();
                if (tecla == 'D') {
                    return; // Regresar al menu principal
                }
                if (tecla == '*') {
                    Estado_Oper_Countdown();
                    Dibujar_Pantalla_Oper();
                    break; // Sale del while interno, retoma el barrido
                }
            }
            continue;
        }

        // Barrido completo: intercambiar extremos y continuar el bucle
        int temp = extremo_actual;
        extremo_actual = extremo_siguiente;
        extremo_siguiente = temp;
    }
}

void Estado_Oper(void)
{
    if (v1 == -1 || v2 == -1 || velocidad_pct == 0) {
        LCD_Linea(0, "     FALTA      ");
        LCD_Linea(1, "    CALIBRAR    ");
        char tecla = 'n';
        while (tecla != 'D') {
            tecla = keypad_scanner();
        }
        return;
    }

    Estado_Oper_Countdown();
    Estado_Oper_Corriendo();
}

// Inicializacion
void Inicializar(void)
{
    ANSEL = 0x00;
    ANSELH = 0x00;

    SERVO_TRIS = 0; // RD0 como salida (señal de control del servo)
    SERVO = 0;

    InitKeypad();

    LCD lcd_config = { &PORTC, 2, 3, 4, 5, 6, 7 };
    LCD_Init(lcd_config);
}

// FUNCION PRINCIPAL
void main(void)
{
    Inicializar();

    while (1) {
        Mostrar_Menu();

        char tecla = 'n';
        while (tecla == 'n') {
            tecla = keypad_scanner();
        }

        switch (tecla) {
            case 'A': Estado_Angulo();    break;
            case 'B': Estado_Rango();     break;
            case 'C': Estado_Velocidad(); break;
            case 'D': Estado_Oper();      break;
            default: break;
        }
    }
}
