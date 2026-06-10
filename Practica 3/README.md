# Practica 3 - Contadores
Implementación de contador decimal y hexadecimal mediante el manejo de salidas digitales con el microcontrolador PIC16F887.

<br>**Esquematico:** <br>
![Esquematico Practica 3](./Esquematico_Contador_Hexadecimal.png)

<br> **Circuito:** <br>
![Circuito](./Circuito_practica3.jpeg)

---
## Actividades
### Clase - Contador Decimal (0 a 9)
Programa que implementa un contador incremental en base 10 (decimal) empleando un display de 7 segmentos conectados a los pines del PIC16F887. Se configuran los puertos como salidas digitales (TRIS) y se envía la secuencia de estados lógicos (PORTB) del 0 al 9 de forma cíclica con retardos de tiempo.
- "unsigned char seg7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};", aqui cada valor es una palabra de bits que enciende los segmentos correctos para mostrar un dígito del 0 al 9.

<br> **Codigo:** <br> [Contador Decimal.c](./Contador_0-9.c)

---
### Actividad 1 - Contador Hexadecimal (0 a F)
Programa diseñado para realizar un conteo secuencial en base 16 (hexadecimal) desde el 0 hasta la F ($15$ en decimal). Se utiliza un display de 7 segmentos conectado a los pines del PIC16F887, manipulando los registros de salida para mostrar los caracteres alfanuméricos correspondientes (0-9 y A-F) mediante el mapeo de bits correspondientes.
<br> **Codigo:** <br> [Contador Hexadecimal.c](./Contador_0-F.c)

---
## Observaciones
- Los registros TRIS deben configurarse en 0x00 para definir el puerto como salida antes de cualquier operacion.
- Se implemento un arreglo de tipo const unsigned char de 16 elementos para almacenar los codigos hexadecimales equivalentes a los caracteres del 0 al F.
- Se implemento una condición de reinicio que actúa como límite para el indice al superar el valor númerico de la F.
- Se utilizo la funcion __delay_ms() para controlar los tiempos de cada secuencia.
- 0x00 = 0x00000000 = 0
- Se toma la entrada a del display de 7 segmentos como el bit menos significativo (LSB)



