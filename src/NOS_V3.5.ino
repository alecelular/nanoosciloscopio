/*
 Nanoosciloscopio

 Alejandro F. Fernández  
 nanoosciloscopio@gmail.com
 Año: 2026

 Descripción:
 Osciloscopio simple usando ADC y temporización por
 interrupciones.

 CPU:
 - ATtiny85, eventualmente ATmega328P

 Licencia: Uso no comercial
 Ver archivo LICENSE para más detalles

 Notas:
 - Optimizado para bajo consumo de memoria
 - Uso intensivo de interrupciones
*/

// Para usar con ATtiny85 o con ATmega328P
// Las explicaciones son mayormente para ATtiny85 para que
// siga siendo un osciloscopio muy nano.

#define VERSION  "V 3.5"       // Hasta 9 de largo
#define FECHA    "27/09/26"
#define EQUIPO   "NOS"
#define AUTOR    "alecelular"

// Líneas en la pantalla a visibilizar, por si se quiere
// agregar más información debajo en el modo osciloscopio.
// (7 agrega una, 6 agrega 2), visto en caracteres simples, no
// dobles. En el caso de OLED<=4, usará una pantalla de 128x32.
// Más de 4, usará una pantalla de 128x64. Usar normalmente 8
// para 128x64 y 4 para 128x32. Los otros modos, se verán co-
// rrectamente, ya que no ocupan más de 4 líneas.
// Se usa una pantalla con controlador SSD1306/SH1106
#define OLED 8

// Si se prefiere usar un SH1106, poner #define SH1106
// No poner el #define si es SSD1306
//#define SH1106

// Establezco idioma. Si no está, se usa castellano IDIOMA_ES
// IDIOMA_ES / IDIOMA_BR / IDIOMA_EN son los definidos.
//#define IDIOMA_BR

// Si se quiere sacar para que haya espacio y hacer código
// Se compilará sin la opción del sensor DS18B20. Se ganan unos
// 350 bytes de flash.
#define CON_DS18B20

// Define la amplitud mínima de la señal para ser detectada
#define AMPLITUD_MINIMA_DETECCION 4

// Accesorios:
//    R     ADC85 ADC328 Conector
// Infinito  250   250   Insertar accesorio
//    0 kΩ   151   139   Osc 'X'
//  5,6 kΩ   172   163   Osc 'Y'
//   10 kΩ   186   175   Osc 'Z'
//   22 kΩ   203   197   Generador
//   47 kΩ   221   218   Frecuencia
//  150 KΩ   241   240   Sensor

// ATtiny85 
// Compilar usando ATtinyCore 1.5.2 sin millis()
// En preferencias: http://drazzy.com/package_drazzy.com_index.json
//| --------------------------------------|
//| En ATtiny85 "Es el NanoOsciloscopio"  |
//|--------------------|------|-----------|
//| Función            | Pata |   Señal   |
//|--------------------|------|-----------|
//| RESET / Selector   |   1  |PB5/ADC0/Re|
//| Cristal A          |   2  |PB3/ADC3/XX|
//| Cristal B          |   3  |PB4/ADC2   |
//| MASA               |   4  |  0 V      |
//| SDA'(BitBang)      |   5  |PB0/AIN0   |
//| SCL'(BitBang)      |   6  |PB1/AIN1   |
//| SAL/ENT/Frec/SENSOR|   7  |PB2/ADC1/T0|
//| Alimentación       |   8  |  VCC      |
//|--------------------| -----|-----------|
//|---------------------------------------|
//| En ATtiny84 (Bosquejo, pero no usado) |
//|--------------------|------|-----------|
//| Función            | Pata |   Señal   |
//|--------------------|------|-----------|
//| Alimentación       |   1  | VCC       |
//| Cristal A          |   2  | PB0       |
//| Cristal B          |   3  | PB1       |
//| RESET              |   4  | PB3 Reset |
//| SALIDA GEN         |   5  | PB2       |
//| PUL 1              |   6  | PA7       |
//| SDA'(bitbang)      |   7  | PA6 MOSI  |
//| PUL 2              |   8  | PA5 MISO  |
//| SCL'(bitbang)      |   9  | PA4 SCK   |
//| ENT FREC           |  10  | PA3 T0    |
//| Selector           |  11  | PA2 ADC2  |
//| ENT ANALOGICA      |  12  | PA1 ADC1  |
//| AREF Capacitor     |  13  | PA0 AREF  |
//| MASA               |  14  | 0 V       |
//|--------------------| -----|-----------|
//| --------------------------------------|
//| En ATmega328P probado con Arduino nano|
//|------------------|---------|----------|
//| Función          |  Pata   |  Señal   |
//|------------------|---------|----------|
//| SAL/ENT/SENSOR   | A0      | PC0      |
//| Frecuencímetro   | D5      | PD5      |
//| SDA'(bitbang)    | D9      | PB1      |
//| SCL'(bitbang)    | D10     | PB2      |
//| PUL1             | D2      | PD2      |
//| PUL2             | D7      | PD7      |
//| Selector         | A1      | PC1      |
//| Capacitor        | AREF    | VREF     |
//|------------------|---------|----------|
//| Extras no usadas aquí                 |
//|------------------|---------|----------|
//| DTR              | Reset   |  Reset   |
//| RXI              | D0/RXD  |  PD0     |
//| TXO              | D1/TXD  |  PD1     |
//|------------------|---------|----------|
//| PB0              |  D8/ICP | PB0      |
//| PB3              |MOSI/OC2 | PB3 3/D11|
//| PB4              |  MISO   | PB4 4    |
//| PB5  LED         | PB5/SCK | PB5 5    |
//| PB6              | PB6(XTAL1/TOSC1)   |
//| PB7              | PB7(XTAL2/TOSC2)   |
//| PC2/PC3          |  A2/A3  |  ADC2/3  |
//| PD3              | D3/INT1 | PD3      |
//| PD4              |D4/XCK/T0| PD4      |
//| PD6              | D6/AIN0 | PD6      |
//| SDA              |   A4    | PC4/ADC4 |
//| SCL              |   A5    | PC5/ADC5 |
//| PC6              | RESET   |  PC6     |
//| ADC6/7           |  A6/7   |  ADC6/7  |
//|------------------|---------|----------|

// Para programar con Arduino as ISP:
//|-----------|-------------|-------------|-------------|
//| Señal ISP |  ATtiny85   |  ATtiny84   |  ATmega328P |
//|-----------|-------------|-------------|-------------|
//| RESET     | pin 1 (RES) | Pin 4 (RST) | RST         |
//| VCC       | pin 8 (VCC) | Pin 1 (VCC) | VCC         |
//| SCK       | pin 7 (PB2) | Pin 9 (PA4) | D13  (PB5)  |
//| MISO      | pin 6 (PB1) | Pin 8 (PA5) | D12  (PB4)  |
//| MOSI      | pin 5 (PB0) | Pin 7 (PA6) | D11  (PB3)  |
//| GND       | pin 4 ( 0V) | Pin 14 (0V) | GND         |
//|-----------|-------------|-------------|-------------|

// Sensor DHT11 Azul / DHT22 Blanco
// Visto de frente, agujeros delante, patas abajo desde
// la izquierda hacia la derecha:
// 1 VCC / 2 Datos / 3 NC / 4 Masa

// Sensor DHT12. Conectado así, se comporta como DHT11.
// Visto de frente, agujeros delante, patas abajo desde
// la izquierda hacia la derecha:
// 1 VCC / 2 Datos / 3 Masa / 4 Masa

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

// Algoritmo Hash FNV-1a de 16 bits ejecutado en tiempo
// de compilación para hacer un ID de verificación de EEPROM
constexpr uint16_t generar_hash_16bits(const char* str,uint16_t hash=0x811C)
{
 return *str?generar_hash_16bits(str+1,(hash^(uint8_t)*str)*0x0101):hash;
}

#define EE_ID (generar_hash_16bits(VERSION))

// Pantalla:
#define ALTURA_MAX (OLED*8-9)
#define PAGINA_ESTADO (OLED-1)

// EN ATTinycore:
// CLOCK_SOURCE==0 RC interno
// CLOCK_SOURCE==1 Cristal externo
// CLOCK_SOURCE==2 Oscilador externo baja frecuencia
// CLOCK_SOURCE==3 Oscilador externo alta frecuencia
// CLOCK_SOURCE==4 Cristal (32 kHz) según micro/core
// CLOCK_SOURCE==5 Reservado o variante especial según micro/core
// CLOCK_SOURCE==6 PLL

// En Minicore versión 3.x (ATMega328P) hay que modificar:
// .arduino15/packages/MiniCore/hardware/avr/3.X.X/boards.txt
// Buscar la opción Internal 8 MHz y añadir la línea al final
// de ese bloque:
// 328.menu.clock.8MHz_internal.build.extra_flags=-DCLOCK_SOURCE=0

// El nativo de Arduino para ATmega328P no tiene opción de
// fuente de señal, siempre asume cristal externo.

// Solo puede usarse con cristal de 8, 12, 16 o 20 MHz
// o bien sin cristal, pero solo a 8 MHz con RC interno.
// En el caso del RC interno es impreciso y debe ajustarse
// calibrando, por omisión, a 50 Hz. Más que nada para hacer
// pruebas o integrar el diseño en otro aparato compilándolo
// con otras opciones que modifiquen el código.

#ifdef CLOCK_SOURCE
 #if CLOCK_SOURCE==0
 #define SIN_CRISTAL
 #endif
#else
 // No se sabe, entonces será impreciso
 #if defined(__AVR_ATtiny85__)
 #define SIN_CRISTAL
 #endif
#endif

#ifdef SIN_CRISTAL
 #if F_CPU!=8000000UL
 #error "No puede usarse sin Cristal que no sea a 8 MHz y solo para pruebas"
 #endif
#endif
#if F_CPU==20000000UL
#define TIPOF " 20 MHz"
#elif F_CPU==16000000UL
#define TIPOF " 16 MHz"
#elif F_CPU==12000000UL
#define TIPOF " 12 MHz"
#elif F_CPU==8000000UL
#ifdef SIN_CRISTAL
#define TIPOF " RC 8MHz"
#else
#define TIPOF " 8 MHz"
#endif
#else
#error "Frecuencia no soportada"
#endif

// Para el caso sin cristal a 8 MHz, si quiero calibrar
// usando una frecuencia conocida de 50 o 60 Hz
// Si es 60 Hz, solo hay que comentar a CAL50
#define CAL50          // Si quiero calibrar por 50 Hz

// Donde está puesto el sensor DHT/DS18B20
#if defined(__AVR_ATtiny85__)
#define PATA_SENSOR PB2
#define DDR_SENSOR DDRB
#define PIN_SENSOR PINB
#define PORT_SENSOR PORTB
#else
#define PATA_SENSOR PC0
#define DDR_SENSOR DDRC
#define PIN_SENSOR PINC
#define PORT_SENSOR PORTC
#endif 

#if F_CPU==8000000UL
 #ifdef SIN_CRISTAL
 #pragma message "Compilado para RC interno 8 MHz, poca precisión."
 #else
 #pragma message "Compilando para 8 MHz"
 #endif
#endif
#if F_CPU==12000000UL
#pragma message "Compilando para 12 MHz"
#endif
#if F_CPU==16000000UL
#pragma message "Compilando para 16 MHz"
#endif
#if F_CPU==20000000UL
#pragma message "Compilando para 20 MHz"
#endif

// Macros de Compatibilidad
#if defined(__AVR_ATtiny85__)
 #define START_CONTR() TCCR0B=(1<<CS02) | (1<<CS01) | (1<<CS00) // PB2
 #define STOP_CONTR()  TCCR0B=0
 #define CLEAR_FLAGS() TIFR=(1<<TOV0) | (1<<OCF1A)
 #define ENABLE_INTS() TIMSK|=(1<<TOIE0) | (1<<OCIE1A)
 #define LECTURA_HW    TCNT0
 #define DESPLAZAMIENTO 8  // Temporizador 0 es de 8 bits
#else
 // ATMega328P:
 // Uso T1 para contar (pata D5) y T2 para base de tiempo
 #define START_CONTR() TCCR1B=(1<<CS12) | (1<<CS11) | (1<<CS10) // D5
 #define STOP_CONTR()  TCCR1B=0
 #define CLEAR_FLAGS() TIFR1=(1<<TOV1); TIFR2=(1<<OCF2A)
 #define ENABLE_INTS() TIMSK1|=(1<<TOIE1); TIMSK2|=(1<<OCIE2A)
 #define LECTURA_HW    TCNT1
 #define DESPLAZAMIENTO 16 // Temporizador 1 es de 16 bits
#endif

// Se pone uno menos, ya que el cero cuenta.
#if F_CPU==16000000L
 #define VALOR_OCR 249
#elif F_CPU==8000000L
 #define VALOR_OCR 124
// Para Attiny85 en 12 y 20 Mhz se suma 1 en un hemiciclo
// Así dan valores exactos.
#elif F_CPU==12000000UL
 // 187,5 -> pongo 187 para que cuente entre 188 y 187
 // Se compensa después
 #define VALOR_OCR 187
#elif F_CPU==20000000UL
 // Hubiera sido 312,5. Excede un byte. entonces lo hago
 // de 156,25. Compenso cada cuatro veces.
 // Se compensa después
 #define VALOR_OCR 155
#endif

// Para iniciar la EEPROM. Se borra entre versiones diferentes.
// EEPROM_ID1 toma los 8 bits superiores
// (equivale a dividir por 256)
#define EEPROM_ID1 ((uint8_t)((EE_ID>>8)&0xFF))

// EEPROM_ID2 toma los 8 bits inferiores
// (equivale al resto de la división por 256)
#define EEPROM_ID2 ((uint8_t)(EE_ID&0xFF))

// Si no se quiere borrar la eeprom entre versiones, usar esto:
//#define EEPROM_ID1 0xAA
//#define EEPROM_ID2 0X55

// No puede ser superior a 254
#define ADC_SAT_ALTO 250

// No puede ser inferior a 1
#define ADC_SAT_BAJO   2

// Valores ensayados que dan según el selector usado.
// La tolerancia (SEL_TOL) de los valores nominales
// serán de +/-SEL_TOL cuentas
// Accesorios:
//    R     ADC85 ADC328 Conector
// Infinito  250   250   Insertar accesorio
//    0 kΩ   151   139   Osc 'X'
//  5,6 kΩ   172   163   Osc 'Y'
//   10 kΩ   186   175   Osc 'Z'
//   22 kΩ   203   197   Generador
//   47 kΩ   221   218   Frecuencia
//  150 KΩ   241   240   Sensor

// Valores de arranque para la primera vez.
#if defined(__AVR_ATtiny85__)
#define SEL_X   151
#define SEL_Y   172
#define SEL_Z   186
#define SEL_G   203
#define SEL_F   221
#define SEL_S   241
#else
#define SEL_X   139
#define SEL_Y   163
#define SEL_Z   175
#define SEL_G   197
#define SEL_F   218
#define SEL_S   240
#endif
#define SEL_TOL 5
#define SEL_MIN 128
#define SEL_MAX 250

// Según el índice, tendré el valor aproximado del ADC para
// la resistencia correspondiente. Es para la primera vez
const byte selValores[6]={SEL_X,SEL_Y,SEL_Z,SEL_G,SEL_F,SEL_S};

// Calibración de 0V
#define RANGO_CAL0 '0'

// Calibración de los divisores de tensión de entrada. Para el
// ATtiny85, los divisores no deberán superar 2,3V y para el
// ATmega328P, 1,1V.
#define RANGO_CALX 'X'
// Las defino, pero no las uso, arranco con la anterior y sumo.
#define RANGO_CALY 'Y'
#define RANGO_CALZ 'Z'

byte EEMEM ee_id1;
byte EEMEM ee_id2;
byte EEMEM ee_band;
byte EEMEM ee_adc_fe0;
byte EEMEM ee_adc_fe[3];
byte EEMEM ee_r[6];
#ifdef SIN_CRISTAL
byte EEMEM ee_osccal;
#endif

// Tamaño del guardado de las mediciones a mostrar
// Mínimo es el ancho del oled. Más, sirve para precisión.
// Tiene que ser en 8 bits. No pongo hasta 255.
#define CAPTURAS_TOTAL 254

// Puntos de la pantalla para que en autoescala muestre una
// onda completa.
#define PUNTOS 100        // Puede ser 128 o menos

// La ESCALA_MINIMA es válida para osciloscopio
// Para el modo generador, debe arrancar en 2
#define ESCALA_MINIMA    0

#if defined(__AVR_ATtiny85__)
 #if F_CPU == 8000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 12000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 16000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 20000000UL
   #define ESCALA_MAXIMA  640  // Máximo 6400 µs
 #else
   #define ESCALA_MAXIMA  960  // No llega acá
 #endif
#else
 // Para el ATmega328P también lo limito a 960 para que
 // el menú se comporte igual
 #define ESCALA_MAXIMA    960  
#endif

#define PRESION_PULSADOR  800

// Configuración de las macros de conversión a cadena
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Carteles
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

#if defined(IDIOMA_EN)

// Inglés
const char RAYAS[]          PROGMEM = "-----";
const char SAT[]            PROGMEM = "  SAT";
const char QUIERO[]         PROGMEM = "Set ";
const char CHZ[]            PROGMEM = ".0 Hz";
const char DA[]             PROGMEM = "Out ";
const char HECHO[]          PROGMEM = "Pass";
const char BUSCO[]          PROGMEM = "Scan";
const char SINO[]           PROGMEM = "<YES/NO>";
const char ABORTA[]         PROGMEM = "<Abort>";
const char ANTES[]          PROGMEM = "Old:";
const char AHORA[]          PROGMEM = "New:";
const char HZ[]             PROGMEM = " Hz";
const char CONFIG[]         PROGMEM = "CONFIG";
const char ESPERA[]         PROGMEM = "Connect\naccessory";
const char FRECUENCIA[]     PROGMEM = "Frequency";
const char GENERADOR[]      PROGMEM = "Generator";
const char FRECUENCIMETRO[] PROGMEM = "Freq";
const char SENSOR[]         PROGMEM = "Sensor";
const char NOVALIDO[]       PROGMEM = "INVALID";
const char SELNOVA[]        PROGMEM = "Invalid Sw";
const char SELESPERA[]      PROGMEM = "Connect accessory";
const char OSCISERIE[]      PROGMEM = "Nano Scope '";
const char OSCILOSCOPIO[]   PROGMEM = "NanoScope ";
const char ANUEVO[]         PROGMEM = "Reset";
const char TE[]             PROGMEM = "T:";
const char CH[]             PROGMEM = " C\nH:";
const char CE[]             PROGMEM = " C";
const char POR[]            PROGMEM = " %";

const char Volver[]    PROGMEM = "<Home>";
const char Captura[]   PROGMEM = "Freeze";
const char AutoEsc[]   PROGMEM = "Autoset";
const char Grilla[]    PROGMEM = "Grid";
const char LibreAuto[] PROGMEM = "Free/Auto";
#define Libre 'F'
const char Flanco[]    PROGMEM = "Slope";
const char LinPun[]    PROGMEM = "Vector/Dot";
const char Escalar[]   PROGMEM = "Zoom";
const char Acerca[]    PROGMEM = "About";

#elif defined(IDIOMA_BR)

// Portugués

const char RAYAS[]          PROGMEM = "-----";
const char SAT[]            PROGMEM = "  SAT";
const char QUIERO[]         PROGMEM = "Def ";
const char CHZ[]            PROGMEM = ",0 Hz";
const char DA[]             PROGMEM = "Sai ";
const char HECHO[]          PROGMEM = "PRONTO";
const char BUSCO[]          PROGMEM = "Busca";
const char SINO[]           PROGMEM = "<SIM/NAO>";
const char ABORTA[]         PROGMEM = "<Aborta>";
const char ANTES[]          PROGMEM = "Ant:";
const char AHORA[]          PROGMEM = "Novo:";
const char HZ[]             PROGMEM = " Hz";
const char CONFIG[]         PROGMEM = "CONFIG";
const char ESPERA[]         PROGMEM = "Conectar\nacessorio";
const char FRECUENCIA[]     PROGMEM = "Frequencia";
const char GENERADOR[]      PROGMEM = "Gerador";
const char FRECUENCIMETRO[] PROGMEM = "Freq";
const char SENSOR[]         PROGMEM = "Sensor";
const char NOVALIDO[]       PROGMEM = "INVALIDO";
const char SELNOVA[]        PROGMEM = "Sel inválido";
const char SELESPERA[]      PROGMEM = "Conectar acessorio";
const char OSCISERIE[]      PROGMEM = "Nano Escopo '";
const char OSCILOSCOPIO[]   PROGMEM = "Escopo ";
const char ANUEVO[]         PROGMEM = "Reset";
const char TE[]             PROGMEM = "T:";
const char CH[]             PROGMEM = " C\nU:";
const char CE[]             PROGMEM = " C";
const char POR[]            PROGMEM = " %";

const char Volver[]    PROGMEM = "<Sair>";
const char Captura[]   PROGMEM = "Retem";
const char AutoEsc[]   PROGMEM = "Autoaj.";
const char Grilla[]    PROGMEM = "Grade";
const char LibreAuto[] PROGMEM = "Livre/Auto";
#define Libre 'L'
const char Flanco[]    PROGMEM = "Borda";
const char LinPun[]    PROGMEM = "Vetor/Ponto";
const char Escalar[]   PROGMEM = "Expandir";
const char Acerca[]    PROGMEM = "POR";

#else

// Castellano

const char RAYAS[]          PROGMEM = "-----";
const char SAT[]            PROGMEM = "  SAT";
const char QUIERO[]         PROGMEM = "Pido";
const char CHZ[]            PROGMEM = ",0 Hz";
const char DA[]             PROGMEM = "Da ";
const char HECHO[]          PROGMEM = "HECHO";
const char BUSCO[]          PROGMEM = "Busco";
const char SINO[]           PROGMEM = "<SI/NO>";
const char ABORTA[]         PROGMEM = "<Aborta>";
const char ANTES[]          PROGMEM = "Antes:";
const char AHORA[]          PROGMEM = "Ahora:";
const char NO[]             PROGMEM = "NO ";
const char HZ[]             PROGMEM = " Hz";
const char CONFIG[]         PROGMEM = "CONFIG";
const char ESPERA[]         PROGMEM = "Conectar\naccesorio";
const char FRECUENCIA[]     PROGMEM = "Frecuencia";
const char GENERADOR[]      PROGMEM = "Generador";
const char FRECUENCIMETRO[] PROGMEM = "Frecuencímetro";
const char SENSOR[]         PROGMEM = "Sensor";
const char NOVALIDO[]       PROGMEM = "INVALIDO";
const char SELNOVA[]        PROGMEM = "Selector no válido";
const char SELESPERA[]      PROGMEM = "Conectar accesorio";
const char OSCISERIE[]      PROGMEM = "Nano Osciloscopio '";
const char OSCILOSCOPIO[]   PROGMEM = "NanoOsc ";
const char ANUEVO[]         PROGMEM = "A nuevo";
const char TE[]             PROGMEM = "T:";
const char CH[]             PROGMEM = " C\nH:";
const char CE[]             PROGMEM = " C";
const char POR[]            PROGMEM = " %";

const char Volver[]    PROGMEM = "<Volver>";
const char Captura[]   PROGMEM = "Captura";
const char AutoEsc[]   PROGMEM = "Autoescala";
const char Grilla[]    PROGMEM = "Grilla";
const char LibreAuto[] PROGMEM = "Libre/Auto";
#define Libre 'L'
const char Flanco[]    PROGMEM = "Flanco";
const char LinPun[]    PROGMEM = "Lineas\nPuntos";
const char Escalar[]   PROGMEM = "<X4>";
const char Acerca[]    PROGMEM = "AUTOR";

#endif

// En común con los idiomas

const char EscUno[]    PROGMEM = "<+1>";
const char EscDiez[]   PROGMEM = "<+10>";
const char EscMax[]    PROGMEM = "<MAX>";
const char Escuno[]    PROGMEM = "<-1>";
const char Escdiez[]   PROGMEM = "<-10>";
const char EscMin[]    PROGMEM = "<MIN>";
const char Gen100[]    PROGMEM = "<+100>";
const char Gen1000[]   PROGMEM = "<+1000>";
const char Gen100M[]   PROGMEM = "<-100>";
const char Gen1000M[]  PROGMEM = "<-1000>";
const char GenEn100[]  PROGMEM = "100 Hz";
const char GenEn1000[] PROGMEM = "1 kHz";
const char GenEn10K[]  PROGMEM = "10 kHz";
const char GenEn25K[]  PROGMEM = "25 kHz";

#ifdef SIN_CRISTAL
#ifdef CAL50
const char Cal50Hz[]   PROGMEM = "CAL 50Hz";
#else
const char Cal50Hz[]   PROGMEM = "CAL 60Hz";
#endif
#endif
const char Calibra0[]  PROGMEM = "Cal 0";
const char Calibrax[]  PROGMEM = "Cal X";
const char Calibray[]  PROGMEM = "Cal Y";
const char Calibraz[]  PROGMEM = "Cal Z";
const char Calibrasx[] PROGMEM = "X  0/"  STR(SEL_X);
const char Calibrasy[] PROGMEM = "Y 5k6/"  STR(SEL_Y);
const char Calibrasz[] PROGMEM = "Z 10k/"  STR(SEL_Z);
const char Calibrasg[] PROGMEM = "G 22k/"  STR(SEL_G);
const char Calibrasf[] PROGMEM = "F 47k/"  STR(SEL_F);
const char Calibrass[] PROGMEM = "S 150k/" STR(SEL_S);

enum Menu_Osciloscopio
{
 // Pulsador Más

 MENU_OSC_AUTOESCALA=1,
 MENU_OSC_ESCALA_MAS1,
 MENU_OSC_ESCALA_MAS10,
 MENU_OSC_ESCALA_MAX,
 MENU_OSC_ESCALAR,
 MENU_OSC_GRILLA,
 MENU_OSC_PUL1_VOLVER,

 // Pulsador Menos

 MENU_OSC_CAPTURA,
 MENU_OSC_LIBRE_AUTO,
 MENU_OSC_ESCALA_MENOS1,
 MENU_OSC_ESCALA_MENOS10,
 MENU_OSC_ESCALA_MIN,
 MENU_OSC_LINEAS,
 MENU_OSC_FLANCO,
 MENU_OSC_PUL2_VOLVER,
};

const char* const menuOsc[] PROGMEM =
{
 AutoEsc,EscUno,EscDiez,
 EscMax,Escalar,Grilla,Volver,

 Captura,LibreAuto,Escuno,Escdiez,
 EscMin,LinPun,Flanco,Volver
};

enum Menu_Generador
{
 MENU_GEN_FREC1=1,
 MENU_GEN_FREC10,
 MENU_GEN_FREC100,
 MENU_GEN_FREC1000,
 MENU_GEN_FRECEN100,
 MENU_GEN_FRECEN25000,
 MENU_GEN_PUL1_VOLVER,

 MENU_GEN_FREC1M,
 MENU_GEN_FREC10M,
 MENU_GEN_FREC100M,
 MENU_GEN_FREC1000M,
 MENU_GEN_FRECEN1000,
 MENU_GEN_FRECEN10000,
 MENU_GEN_PUL2_VOLVER,
};

// Usa menúes del osciloscopio porque se repiten
const char* const menuGen[] PROGMEM =
{
 EscUno, EscDiez, Gen100, Gen1000,
 GenEn100,GenEn25K,Volver,
 Escuno, Escdiez, Gen100M, Gen1000M,
 GenEn1000, GenEn10K, Volver
};

enum Menu_Conf
{
 MENU_CFG_CAL_0=1,
 MENU_CFG_CAL_X,
 MENU_CFG_CAL_Y,
 MENU_CFG_CAL_Z,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC50,
 #endif
 MENU_CFG_ACERCADE,
 MENU_CFG_PUL1_VOLVER,

 MENU_CFG_CAL_SX,
 MENU_CFG_CAL_SY,
 MENU_CFG_CAL_SZ,
 MENU_CFG_CAL_SG,
 MENU_CFG_CAL_SF,
 MENU_CFG_CAL_SS,
 MENU_CFG_PUL2_VOLVER,
};

const char* const menuCfg[] PROGMEM =
{
 Calibra0, Calibrax, Calibray, Calibraz,
 #ifdef SIN_CRISTAL
 Cal50Hz,
 #endif
 Acerca, Volver,
 Calibrasx, Calibrasy, Calibrasz, Calibrasg, Calibrasf, Calibrass, Volver
};

// Para Osciloscopio
volatile byte capturas[CAPTURAS_TOTAL];
volatile byte indice;   // Lo usa el frecuencímetro también
int escala;       // Puede tomar valores negativos momentáneos
unsigned int tiempoReal_us;

// Para frecuencímetro
volatile unsigned int excesos_contador=0;
volatile byte cuenta_base_tiempo=0;
volatile byte limite_cuentas;

char rangoActual=RANGO_CALX;

// Bits:
// 0 mostrar grilla
// 1 modo línea     falso = puntos, verdadero = línea
// 2 modo gatillo   Espera cruce, falso: Barrido libre
// 3 auto escala    Arranca con autoescala. Falso, Manual
// 4 Escalar        Estira por 4 lo visualizado
// 5 Flanco         falso, es flanco ascendente
// 6 Captura        Pone en negativo la imagen del osciloscopio
// 7 Sensor         Determina cual sensor de temperatura está
byte band=0;
#define BIT_MOSTRARGRILLA  0
#define BIT_MODOLINEA      1
#define BIT_MODOGATILLO    2
#define BIT_AUTOESCALA     3
#define BIT_ESCALAR        4
#define BIT_FLANCO         5
#define BIT_CAPAN          6
#define BIT_SENSOR         7
#define BAND_MOSTRARGRILLA (1<<BIT_MOSTRARGRILLA)
#define BAND_MODOLINEA     (1<<BIT_MODOLINEA)
#define BAND_MODOGATILLO   (1<<BIT_MODOGATILLO)
#define BAND_AUTOESCALA    (1<<BIT_AUTOESCALA)
#define BAND_ESCALAR       (1<<BIT_ESCALAR)
#define BAND_FLANCO        (1<<BIT_FLANCO)
#define BAND_CAPAN         (1<<BIT_CAPAN)
#ifdef CON_DS18B20
#define BAND_SENSOR        (1<<BIT_SENSOR)
#endif

// Para leer el pulsador
volatile bool pido=false; // Necesario en modo generador
bool aborta=false;        // Termina de golpe la medición

// Selector anterior
byte lsant=254;          // La primera vez, inicializa

byte adc_fondo_escala;
byte adc_offset;

// Estados de modo
#define MODO_OSCILOSCOPIO 0     // Debe ser 0 OSCILOSCOPIO
#define MODO_GENERADOR    1
#define MODO_CFG          2
#define MODO_FREC         3
#define MODO_SENSOR       4
#define MODO_ESPERA       5
#define MODO_NOVALIDO     6

volatile byte modoActual=(byte)MODO_ESPERA;

// Variables para el motor del generador
volatile unsigned int contadorFrecuencia;
volatile unsigned int recargaFrecuencia;
signed int genFrec=50;        // Arranca en 50 Hz

// Convierte y lee el ADC
byte adira(void)
{
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC));
 return ADCH;
}

// Habilita interrupciones del temporizador 1 y las generales.
void habilitarT1(void)
{
 // Y hago una primera conversión del ADC para empezar, solo
 // en modo osciloscopio
 if(modoActual==(byte)MODO_OSCILOSCOPIO)
 {
  // Estabilizo el muestreo de onda en ATmega328P
  // Necesita estabilizarse así. No es necesario en ATtiny85
  // pero lo aplico de la misma manera, con menos cantidad.
  #if defined(__AVR_ATmega328P__)
  for(unsigned int i=0;i<2000;i++)
  #else
  for(byte i=0;i<250;i++)
  #endif
  {
   adira();
  }
  // Primera captura
  capturas[0]=adira();
  indice=1;
 }
 aborta=false;
 #if defined(__AVR_ATtiny85__)
 TIMSK|=(1<<OCIE1A);
 #else
 TIMSK1|=(1<<OCIE1A);
 #endif
 sei();
}

// Devuelve verdadero si está apagado T1 o por si se aborta
// Si hay cristal, puede abortarse en cualquier momento.
// Sin cristal, las escalas debajo de 60 no abortan, son rá-
// pidas y no necesitan ser abortadas, y conviene para que
// pueda ser calibrado.
bool finCaptura(void)
{
 // Detección de pulsador 1 o 2 para abortar
 // Termina cuando leyó todo, o bien se apretó un pulsador
 #if defined(__AVR_ATtiny85__)
 #ifdef SIN_CRISTAL
 if(escala>60)
 {
 #endif
  aborta=((!(ACSR & (1<<ACO))) | (!(PINB & (1<<PB1))));
  if(aborta) TIMSK&=~(1<<OCIE1A);
 #ifdef SIN_CRISTAL
 }
 #endif
 return !(TIMSK&(1<<OCIE1A));

 #else

 // Pata D2 en ATmega328P es pulsador 1, D7 es pulsador 2.
 #ifdef SIN_CRISTAL
 if(escala>60)
 {
 #endif
  aborta=((!(PIND & (1<<PD2))) | (!(PIND & (1<<PD7))));
  if(aborta) TIMSK1&=~(1<<OCIE1A);
 #ifdef SIN_CRISTAL
 }
 #endif
 return !(TIMSK1&(1<<OCIE1A));

 #endif
}
/* 
 * -------------------------------------------------------------------------
 * Multiplicador (m) | Tiempo/Punto | Tiempo Pantalla | Frecuencia (1 Ciclo)
 * -------------------------------------------------------------------------
 *       m = 1       |     10 us    |     1.28 ms     |      781,24 Hz
 *       m = 2       |     20 us    |     2.56 ms     |      390.62 Hz
 *       m = 5       |     50 us    |     6.40 ms     |      156.25 Hz
 *       m = 8       |     80 us    |    10.24 ms     |       97.65 Hz
 *       m = 10      |    100 us    |    12.80 ms     |       78.12 Hz
 *       m = 16      |    160 us    |    20.48 ms     |       48.82 Hz
 *       m = 20      |    200 us    |    25.60 ms     |       39.06 Hz
 *       m = 50      |    500 us    |     64.0 ms     |       15.62 Hz
 *       m = 100     |      1 ms    |      128 ms     |        7.81 Hz
 *       m = 200     |      2 ms    |      256 ms     |        3.90 Hz
 *       m = 408     |   4.08 ms    |      522 ms     |        1.91 Hz
 *       m = 816     |   8.16 ms    |     1044 ms     |        0.95 Hz
 * -------------------------------------------------------------------------
 | ---------------------------------------------------- |
 | ADC para cristal de 16 MHz                           |
 | ---------------------------------------------------- |
 | Prescaler | Fadc    | Tiempo real (µs) | Recomendado |
 | ---------------------------------------------------- |
 | /2        | 8 MHz   | **1,625 µs**     | Overclock   |
 | /4        | 4 MHz   | **3,25 µs**      | Overclock   |
 | /8        | 2 MHz   | **6,5 µs**       | Overclock   |
 | /16       | 1 MHz   | **13 µs**        | Límite alto |
 | /32       | 500 kHz | **26 µs**        | Bueno       |
 | /64       | 250 kHz | **52 µs**        | Óptimo      |
 | /128      | 125 kHz | **104 µs**       | Óptimo      |
 | ---------------------------------------------------- |
 | ADC para cristal de 8 MHz                            |
 | ---------------------------------------------------- |
 | Prescaler | Fadc    | Tiempo real (µs) | Recomendado |
 | ---------------------------------------------------- |
 | /2        | 4 MHz   | **3,25 µs**      | Overclock   |
 | /4        | 2 MHz   | **6,5 µs**       | Overclock   |
 | /8        | 1 MHz   | **13 µs**        | Límite alto |
 | /16       | 500 kHz | **26 µs**        | Bueno       |
 | /32       | 250 kHz | **52 µs**        | Óptimo      |
 | /64       | 125 kHz | **104 µs**       | Óptimo      |
 | /128      | 62,5 kHz| **208 µs**       | Conservador |
 | ---------------------------------------------------- |
 | Para ATtiny85/ ATmega328P en el ADC:
 | ADPS2 | ADPS1 | ADPS0 | División |
 | ----- | ----- | ----- | -------- |
 | 0     | 0     | 0     | /2       | Son dos valores iguales
 | 0     | 0     | 1     | /2       |
 | 0     | 1     | 0     | /4       |
 | 0     | 1     | 1     | /8       |
 | 1     | 0     | 0     | /16      |
 | 1     | 0     | 1     | /32      |
 | 1     | 1     | 0     | /64      |
 | 1     | 1     | 1     | /128     |

 Para prescaler de AtMega328P
 CS12,CS11,CS10,Descripción (Prescaler del Timer)
  0    0     1  División por 1
  0    1     0, División por 8
  0    1     1  División por 64
  1    0     0  División por 256
  1    0     1, División por 1024

 Prescaler ATtiny85:
 CS13 CS12 CS11 CS10,Factor de División (Prescaler)
   0    0    0    1     1 (Reloj directo)
   0    0    1    0     2
   0    0    1    1     4
   0    1    0    0     8
   0    1    0    1    16
   0    1    1    0    32
   0    1    1    1    64
   1    0    0    0   128
   1    0    0    1   256
   1    0    1    0   512
   1    0    1    1  1024
   1    1    0    0  2048
   1    1    0    1  4096
   1    1    1    0  8192
   1    1    1    1 16384

 Para cada cristal:
 | ------ | --------------- | -------- | ----------------- |
 | F_CPU  | mínimo práctico | sugerido | Vcc mínima segura |
 | ------ | --------------- | -------- | ----------------- |
 | 8 MHz  | 1 µs            | 2–5 µs   | ~2.7 V            |
 | 12 MHz | 2 µs            | 4 µs     | ~3.3 V            |
 | 16 MHz | 1 µs            | 2–5 µs   | ~4.5 V            |
 | 20 MHz | 2 µs            | 4 µs     | ~5.0 V            |
 | ------ | --------------- | -------- | ----------------- |

 Se deben ajustar el tiempo deseado, según procesador
 y la frecuencia
 FUNCIÓN AUXILIAR DE VALIDACIÓN

   TABLA DE REFERENCIA: LAS 70 ESCALAS REALES VALIDADAS
   CON F_CPU = 12 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 6 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Prescaler /1 y /2 | Pasos lineales de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   20. m = 20  --> 200 uS      21. m = 22  --> 220 uS      22. m = 24  --> 240 uS
   23. m = 26  --> 260 uS      24. m = 28  --> 280 uS      25. m = 30  --> 300 uS
   26. m = 32  --> 320 uS      27. m = 34  --> 340 uS      28. m = 36  --> 360 uS
   29. m = 40  --> 400 uS      30. m = 44  --> 440 uS      31. m = 48  --> 480 uS
   32. m = 52  --> 520 uS      33. m = 56  --> 560 uS      34. m = 60  --> 600 uS
   35. m = 64  --> 640 uS      36. m = 68  --> 680 uS      37. m = 72  --> 720 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   38. m = 80  --> 800 uS      39. m = 88  --> 880 uS      40. m = 96  --> 960 uS
   41. m = 104 --> 1.04 ms     42. m = 112 --> 1.12 ms     43. m = 120 --> 1.20 ms
   44. m = 128 --> 1.28 ms     45. m = 136 --> 1.36 ms     46. m = 144 --> 1.44 ms
   47. m = 160 --> 1.60 ms     48. m = 176 --> 1.76 ms     49. m = 192 --> 1.92 ms
   50. m = 208 --> 2.08 ms     51. m = 224 --> 2.24 ms     52. m = 240 --> 2.40 ms
   53. m = 256 --> 2.56 ms     54. m = 272 --> 2.72 ms     55. m = 288 --> 2.88 ms
   56. m = 320 --> 3.20 ms     57. m = 352 --> 3.52 ms     58. m = 384 --> 3.84 ms
   59. m = 416 --> 4.16 ms     60. m = 448 --> 4.48 ms     61. m = 480 --> 4.80 ms
   62. m = 512 --> 5.12 ms     63. m = 544 --> 5.44 ms     64. m = 576 --> 5.76 ms
   65. m = 640 --> 6.40 ms     66. m = 704 --> 7.04 ms     67. m = 768 --> 7.68 ms
   68. m = 832 --> 8.32 ms     69. m = 896 --> 8.96 ms     70. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 61 ESCALAS REALES VALIDADAS
   CON F_CPU = 8 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 10 uS       02. m = 1   --> 12 uS

   [RANGO RÁPIDO] (Prescaler /1 | Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS      20. m = 19  --> 190 uS
   21. m = 20  --> 200 uS      22. m = 21  --> 210 uS      23. m = 22  --> 220 uS
   24. m = 23  --> 230 uS      25. m = 24  --> 240 uS      26. m = 25  --> 250 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   27. m = 26  --> 260 uS      28. m = 28  --> 280 uS      29. m = 30  --> 300 uS
   30. m = 32  --> 320 uS      31. m = 34  --> 340 uS      32. m = 36  --> 360 uS
   33. m = 38  --> 380 uS      34. m = 40  --> 400 uS      35. m = 44  --> 440 uS
   36. m = 48  --> 480 uS      37. m = 52  --> 520 uS      38. m = 56  --> 560 uS
   39. m = 60  --> 600 uS      40. m = 64  --> 640 uS      41. m = 68  --> 680 uS
   42. m = 72  --> 720 uS      43. m = 76  --> 760 uS      44. m = 80  --> 800 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   45. m = 96  --> 960 uS      46. m = 128 --> 1.28 ms     47. m = 160 --> 1.60 ms
   48. m = 192 --> 1.92 ms     49. m = 224 --> 2.24 ms     50. m = 256 --> 2.56 ms
   51. m = 288 --> 2.88 ms     52. m = 320 --> 3.20 ms     53. m = 352 --> 3.52 ms
   54. m = 384 --> 3.84 ms     55. m = 448 --> 4.48 ms     56. m = 512 --> 5.12 ms
   57. m = 576 --> 5.76 ms     58. m = 640 --> 6.40 ms     59. m = 768 --> 7.68 ms
   60. m = 896 --> 8.96 ms     61. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 70 ESCALAS REALES VALIDADAS
   CON F_CPU = 16 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 6 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS       

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   19. m = 18  --> 180 uS      20. m = 20  --> 200 uS      21. m = 22  --> 220 uS
   22. m = 24  --> 240 uS      23. m = 26  --> 260 uS      24. m = 28  --> 280 uS
   25. m = 30  --> 300 uS      26. m = 32  --> 320 uS      27. m = 34  --> 340 uS
   28. m = 36  --> 360 uS      29. m = 40  --> 400 uS      30. m = 44  --> 440 uS
   31. m = 48  --> 480 uS      32. m = 52  --> 520 uS      33. m = 56  --> 560 uS
   34. m = 60  --> 600 uS      35. m = 64  --> 640 uS      36. m = 68  --> 680 uS
   37. m = 72  --> 720 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   38. m = 80  --> 800 uS      39. m = 88  --> 880 uS      40. m = 96  --> 960 uS
   41. m = 104 --> 1.04 ms     42. m = 112 --> 1.12 ms     43. m = 120 --> 1.20 ms
   44. m = 128 --> 1.28 ms     45. m = 136 --> 1.36 ms     46. m = 144 --> 1.44 ms
   47. m = 160 --> 1.60 ms     48. m = 176 --> 1.76 ms     49. m = 192 --> 1.92 ms
   50. m = 208 --> 2.08 ms     51. m = 224 --> 2.24 ms     52. m = 240 --> 2.40 ms
   53. m = 256 --> 2.56 ms     54. m = 272 --> 2.72 ms     55. m = 288 --> 2.88 ms
   56. m = 320 --> 3.20 ms     57. m = 352 --> 3.52 ms     58. m = 384 --> 3.84 ms
   59. m = 416 --> 4.16 ms     60. m = 448 --> 4.48 ms     61. m = 480 --> 4.80 ms
   62. m = 512 --> 5.12 ms     63. m = 544 --> 5.44 ms     64. m = 576 --> 5.76 ms
   65. m = 640 --> 6.40 ms     66. m = 704 --> 7.04 ms     67. m = 768 --> 7.68 ms
   68. m = 832 --> 8.32 ms     69. m = 896 --> 8.96 ms     70. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 61 ESCALAS REALES VALIDADAS
   CON F_CPU = 20 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 4 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Prescaler /1 | Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS      20. m = 19  --> 190 uS
   21. m = 20  --> 200 uS      22. m = 21  --> 210 uS      23. m = 22  --> 220 uS
   24. m = 23  --> 230 uS      25. m = 24  --> 240 uS      26. m = 25  --> 250 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   27. m = 26  --> 260 uS      28. m = 28  --> 280 uS      29. m = 30  --> 300 uS
   30. m = 32  --> 320 uS      31. m = 34  --> 340 uS      32. m = 36  --> 360 uS
   33. m = 38  --> 380 uS      34. m = 40  --> 400 uS      35. m = 44  --> 440 uS
   36. m = 48  --> 480 uS      37. m = 52  --> 520 uS      38. m = 56  --> 560 uS
   39. m = 60  --> 600 uS      40. m = 64  --> 640 uS      41. m = 68  --> 680 uS
   42. m = 72  --> 720 uS      43. m = 76  --> 760 uS      44. m = 80  --> 800 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   45. m = 96  --> 960 uS      46. m = 128 --> 1.28 ms     47. m = 160 --> 1.60 ms
   48. m = 192 --> 1.92 ms     49. m = 224 --> 2.24 ms     50. m = 256 --> 2.56 ms
   51. m = 288 --> 2.88 ms     52. m = 320 --> 3.20 ms     53. m = 352 --> 3.52 ms
   54. m = 384 --> 3.84 ms     55. m = 448 --> 4.48 ms     56. m = 512 --> 5.12 ms
   57. m = 576 --> 5.76 ms     58. m = 640 --> 6.40 ms     59. m = 768 --> 7.68 ms
   60. m = 896 --> 8.96 ms     61. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
*/
bool esEscalaValida(unsigned int m)
{
 if(m<=1) return true; // Casos especiales calibrados fijos

 unsigned int tDeseadoTemp;
 #if F_CPU==12000000UL
 tDeseadoTemp=m*15;
 #elif F_CPU==20000000UL
 tDeseadoTemp=m*25;
 #else
 tDeseadoTemp=m*10;
 #endif

 // Aplico el mismo criterio en ATmega328P para
 // que haga saltos y no tenga tantas escalas, por más que
 // en ese procesador no lo necesite. porque tendría demasiadas
 // escalas intermedias superfluas, y así, más breve, mejora.
 byte ticTemp;
 if(tDeseadoTemp<=255)       ticTemp=1;
 else if(tDeseadoTemp<=510)  ticTemp=2;
 else if(tDeseadoTemp<=1020) ticTemp=4;
 else if(tDeseadoTemp<=2040) ticTemp=8;
 else if(tDeseadoTemp<=4080) ticTemp=16;
 else if(tDeseadoTemp<=8160) ticTemp=32;
 else                        ticTemp=64;

 // Solo los tiempos redondos son los deseados
 return (tDeseadoTemp%ticTemp==0);
}

// Se pasa una escala deseada y se obtiene la real.
// Los valores se ajustan a tiempos exactos, no fraccionarios.
// Pasos puede ir de -10 hasta +10
// m puede ir de 0 a 960
unsigned int configurarEscala(unsigned int m, int8_t pasos)
{
 // PROCESAMIENTO Y VALIDACIÓN DE LA ESCALA
 if(pasos!=0) 
 {
  bool direccion=(pasos>0);
  byte pasosAbsolutos=(pasos>0)?pasos:-pasos;

  for(byte i=0;i<pasosAbsolutos;i++)
  {
   do
   {
    if(!direccion && m<=ESCALA_MINIMA) {m=ESCALA_MINIMA; break;}
    if(direccion  && m>=ESCALA_MAXIMA) {m=ESCALA_MAXIMA; break;}
    m+=(direccion?1:-1);
   } while(!esEscalaValida(m));
  }
 } 
 else 
 {
  if(!esEscalaValida(m))
  {
   m=ESCALA_MINIMA; 
  }
 }

 // CÁLCULO DEL TIEMPO DESEADO Y CALIBRACIONES ESPECIALES
 #if F_CPU==12000000UL
 unsigned int tiempoDeseado=(unsigned int)m*15;
 #elif F_CPU==20000000UL
 unsigned int tiempoDeseado=(unsigned int)m*25;
 #else
 unsigned int tiempoDeseado=(unsigned int)m*10;
 #endif

 #if F_CPU==20000000UL
  // En 20 MHz, modo osciloscopio
  // 10 son 4 µs. 20 son 8 µs. 25 son 10 µs.
  // No bajar de 4 µs, ya es óptimo/mínimo
  if(m==0) tiempoDeseado=10;            // 4 µs Calibrado
  if(m==1) tiempoDeseado=20;            // 8 µs Calibrado
 #elif F_CPU==16000000UL
  #if defined(__AVR_ATtiny85__)
   if(m==0) tiempoDeseado=6;            // 6 µs Calibrado
   if(m==1) tiempoDeseado=8;            // 8 µs Calibrado
  #else
   // Usado en Arduino UNO, Arduino nano y en Arduino mini
   // pero en ese último no es muy preciso.
   if(m==0) tiempoDeseado=6;            // Supuesto
   if(m==1) tiempoDeseado=8;            // Supuesto
  #endif
 #elif F_CPU==12000000UL
  // En 12 MHz, modo osciloscopio
  // 12 son 8 µs. 15 son 10 µs. 9 son 6 µs
  // No bajar de 6 µs, ya es óptimo/mínimo
  if(m==0) tiempoDeseado=9;             // 6 µs Calibrado
  if(m==1) tiempoDeseado=12;            // 8 µs Calibrado
 #else // 8 MHz
  #if defined(__AVR_ATtiny85__)
   // En 8 MHz, modo osciloscopio, 8 es minimo, 10 óptimo
   if(m==0) tiempoDeseado=10;          // 10 µs Calibrado
   // En 8 MHz, modo generador, 12 mínimo/óptimo
   if(m==1) tiempoDeseado=12;          // 12 µs Calibrado
  #else
   // Para los ATmega328P, un poco más lento
   // En 8 MHz, modo osciloscopio, 12 es minimo
   // Usado en placas Arduino mini de 3,3 V, pero que
   // la frecuencia de 8 MHz no es muy precisa.
   if(m==0) tiempoDeseado=12;         // 12 µs Supuesto
   // En 8 MHz, modo generador, 15 es minimo
   if(m==1) tiempoDeseado=15;         // 15 µs Supuesto
  #endif
 #endif

 // Temporizador 1
 #if defined(__AVR_ATtiny85__)
 TCCR1&=~0x0F;
 byte tic;

 #if F_CPU==8000000UL || F_CPU==12000000UL || F_CPU==20000000UL
 if(tiempoDeseado<=255)
 {
  TCCR1|=(1<<CS12);
  OCR1C=(byte)(tiempoDeseado-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=2;
  #else
  tic=1;
  #endif
 } 
 else if(tiempoDeseado<=510)
 {
  TCCR1|=(1<<CS12)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/2)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=4;
  #else
  tic=2;
  #endif
 }
 else if(tiempoDeseado<=1020)
 {
  TCCR1|=(1<<CS12)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/4)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=8;
  #else
  tic=4;
  #endif
 } 
 else if(tiempoDeseado<=2040)
 {
  TCCR1|=(1<<CS12)|(1<<CS11)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/8)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=16;
  #else
  tic=8;
  #endif
 }
 else if(tiempoDeseado<=4080)
 {
  TCCR1|=(1<<CS13);
  OCR1C=(byte)((tiempoDeseado/16)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=32;
  #else
  tic=16;
  #endif
 }
 else if(tiempoDeseado<=8160)
 {
  TCCR1|=(1<<CS13)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=64;
  #else
  tic=32;
  #endif
 }
 else // Hasta 16320 ticks absolutos
 {
  TCCR1|=(1<<CS13)|(1<<CS11); // Bits correspondientes al prescaler /64 en ATtiny85
  OCR1C=(byte)((tiempoDeseado/64)-1);
  #if F_CPU==12000000UL || F_CPU==20000000UL
  tic=128;
  #else
  tic=64;
  #endif
 }
 #else // 16 MHz ATtiny85
 if(tiempoDeseado<=255)
 {
  TCCR1|=(1<<CS12)|(1<<CS10);
  OCR1C=(byte)(tiempoDeseado-1);
  tic=1;
 } 
 else if(tiempoDeseado<=510)
 {
  TCCR1|=(1<<CS12)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/2)-1);
  tic=2;
 } 
 else if(tiempoDeseado<=1020)
 {
  TCCR1|=(1<<CS12)|(1<<CS11)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/4)-1);
  tic=4;
 } 
 else if(tiempoDeseado<=2040)
 {
  TCCR1|=(1<<CS13);
  OCR1C=(byte)((tiempoDeseado/8)-1);
  tic=8;
 }
 else if(tiempoDeseado<=4080)
 {
  TCCR1|=(1<<CS13)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/16)-1);
  tic=16;
 }
 else if(tiempoDeseado<=8160)
 {
  TCCR1|=(1<<CS13)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  tic=32;
 }
 else
 {
  TCCR1|=(1<<CS13)|(1<<CS11)|(1<<CS10); // Bits para prescaler /64 en modo 16MHz
  OCR1C=(byte)((tiempoDeseado/64)-1);
  tic=64;
 }
 #endif

 // Tienen que igualarse para provocar las interrupciones
 OCR1A=OCR1C;

 // Ya viene multiplicado tic por 2 para beneficio de la com-
 // pilación y no tener que multiplicar por 2 en los casos
 // de 12 y 20 MHz.

 #if F_CPU==12000000UL
 tiempoReal_us=((unsigned int)(OCR1C+1)*tic)/3;
 #elif F_CPU==20000000UL
 tiempoReal_us=((unsigned int)(OCR1C+1)*tic)/5;
 #else
 tiempoReal_us=(unsigned int)(OCR1C+1)*tic;
 #endif

 #else

 // ATMega328P

 TCCR1A=0;
 TCCR1B=0;

 TCCR1B|=(1<<WGM12); // Modo CTC
 TCCR1B|=(1<<CS11);  // Prescaler /8

 #if F_CPU==16000000UL
 OCR1A=(tiempoDeseado*2)-1;
 tiempoReal_us=(unsigned int)(OCR1A+1)/2;
 #else
 OCR1A=tiempoDeseado-1;
 tiempoReal_us=(unsigned int)(OCR1A+1);
 #endif

 #endif

 // Prescaler del ADC para el modo osciloscopio
 if(!modoActual)
 {
  // Mayor divisor (1<<ps) cuya conversión cabe en el período.
  // ps=1 es /2 ... ps=7 es /128 (coincide con ADPS2..0).
  // F_CPU/4 MHz es entero en 8, 12, 16 y 20 MHz.
  unsigned int t=tiempoReal_us*(unsigned int)(F_CPU/4000000UL);
  unsigned int v=14;    // 7<<ps con ps=1
  byte ps=1;
  while(ps<7 && v<t) { ps++; v<<=1; }
  
  while(ADCSRA & (1<<ADSC));
  ADCSRA&=~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
  ADCSRA|=ps;

  // Conversión de limpieza
  adira();
 }

 return m; 
}

// Interrupciones
ISR(TIMER1_COMPA_vect)
{
 // El modo osciloscopio es 0.
 // Es más rápido que la comparación
 if(!modoActual)
 {
  if(indice<CAPTURAS_TOTAL)
  {
   capturas[indice++]=ADCH;
   ADCSRA|=(1<<ADSC);          // es ADCSRA|=0x40;
  }
  else
  {
   #if defined(__AVR_ATtiny85__)
   TIMSK&=~(1<<OCIE1A);  // Registro específico del ATtiny85
   #else
   TIMSK1&=~(1<<OCIE1A); // Registro específico del ATmega328P
   #endif
  }
  return;
 }

 // Modo frecuencímetro
 if(modoActual==(byte)MODO_FREC)
 {
  #if defined(__AVR_ATtiny85__)
  #if (F_CPU==12000000UL)

  // TCNT1 es el contador que va desde 0 hasta la
  // dupla OCR1A/C. Corrijo las cuentas con decimal en 0,5
  // Esto hace que en 12 MHz sea también preciso
  if(pido=!pido) TCNT1++;

  #endif
  #if (F_CPU==20000000UL)
  // TCNT1 es el contador que va desde 0 hasta la
  // dupla OCR1A/C./ Corrijo las cuentas con decimal en 0,25.
  // La ventana debería tener el doble de medición para que
  // lo sea, o bien usar en vez de 250 para medir, valores que
  // sean múltiplos de 4, por ejemplo 252. Pero no sería exacta-
  // mente un segundo pero sería exacta.
  // (Da ventana de 1,008 s y mido con esa ventana y corrijo).
  if(++indice>=4)       // Reciclo esta variable
  {
   indice=0;
   TCNT1--;
  }
  #endif
  if(++cuenta_base_tiempo>=limite_cuentas)
  {
   STOP_CONTR();      // Termina la lectura
  }
  #endif
  return;
 }

 // MODO GENERADOR
 pido=false;
 if(--contadorFrecuencia==0)
 {
  #if defined(__AVR_ATtiny85__)
  PINB=(1<<PB2); 
  #else
  PINC=(1<<PC0);
  #endif
  contadorFrecuencia=recargaFrecuencia;
 }
}

// ISR Base de Tiempo en ATmega328P: Detiene el cronómetro
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_COMPA_vect)
{
 if(++cuenta_base_tiempo>=limite_cuentas)
 {
  STOP_CONTR();      // Termina la lectura
 }
}

ISR(TIMER1_OVF_vect)
{
 excesos_contador++;
}

#else

// ISR Contador: Acumula los desbordamientos del Timer 0
ISR(TIMER0_OVF_vect)
{
 excesos_contador++;    // Cuento en 16 bits + 8 bits = 16777215
}
#endif

/*
| CS13..10 | Divisor |
| -------- | ------- |
| 0100     | /8      |
| 0101     | /16     |
| 0110     | /32     |
| 0111     | /64     |
| 1000     | /128    |
| 1001     | /256    |
| 1010     | /512    |
| 1011     | /1024   |
| 1100     | /2048   |
| 1101     | /4096   |
| 1110     | /8192   |
| 1111     | /16384  |
*/
unsigned long int medirFrecuencia(void)
{
 // Respaldo
 byte guarda_sreg=SREG;
 cli();

 #if !defined(__AVR_ATtiny85__)

 // En ATmega328P:
 // Timer 1 (Contador) y Timer 2 (Base de tiempo)

 byte guarda_T1A=TCCR1A, guarda_T1B=TCCR1B;
 unsigned int guarda_TCNT1=TCNT1;
 byte guarda_T2A=TCCR2A, guarda_T2B=TCCR2B, guarda_MSK1=TIMSK1, guarda_MSK2=TIMSK2;

 #else

 // En ATtiny85: guardo cómo estaba TCCR1 antes de tocarlo,
 // para no dejarle un CTC1 "regalado" al osciloscopio.
 // Excepto si no tiene cristal, no lo hace
 byte guarda_TCCR1=TCCR1;

 #endif

 // Preparación
 excesos_contador=0;
 cuenta_base_tiempo=0;

 // Configuración del Contador
 #if defined(__AVR_ATtiny85__)
 TCCR0A=0;
 TCNT0=0;
 TCNT1=0;
 #if F_CPU==20000000
 TCCR1=(1<<CTC1) | (1<<CS13) | (1<<CS11); // /512
 #else
 TCCR1=(1<<CTC1) | (1<<CS13) | (1<<CS10); // /256
 #endif
 // TCNT1 es el contador que va desde 0 hasta la dupla OCR1A/C
 OCR1A=VALOR_OCR;
 OCR1C=VALOR_OCR;
 #else
 // ATMega328P
 TCCR1A=0;
 TCCR1B=0;
 TCNT1=0;
 TCNT2=0;
 TCCR2A=(1<<WGM21);
 TCCR2B=(1<<CS22) | (1<<CS21);
 OCR2A=VALOR_OCR;
 #endif

 CLEAR_FLAGS();
 ENABLE_INTS();
    
 sei();
 START_CONTR(); 

 // Espero a que termine
 #if defined(__AVR_ATtiny85__)
 while(TCCR0B!=0);
 #else
 while(TCCR1B!=0);
 #endif

 // El resultado tiene 3 bytes. Puedo llegar a multiplicarlo
 // por 125 en caso del cristal de 20 MHz para luego dividirlo
 // por 126, sin perder precisión. Corrije el 1,008 segundos
 unsigned long int conteo_final=((unsigned long int)excesos_contador<<DESPLAZAMIENTO)+LECTURA_HW;

 // Restauro
 cli();

 #if !defined(__AVR_ATtiny85__)
 TCCR1A=guarda_T1A;
 TCCR1B=guarda_T1B;
 TCNT1=guarda_TCNT1;
 TCCR2A=guarda_T2A;
 TCCR2B=guarda_T2B;
 TIMSK1=guarda_MSK1;
 TIMSK2=guarda_MSK2;
 #else
 TCCR1=guarda_TCCR1;
 #endif
    
 SREG=guarda_sreg;

 // Corrijo por si limite_cuentas superan 250
 // Es el caso del cristal de 20 MHz
 #if F_CPU==20000000UL

 // Debo corregir
 conteo_final*=125;
 return conteo_final/126;

 #else

 return conteo_final;

 #endif
}

// Captura según la escala elegida y analiza los datos. Devuel-
// ve la frecuencia x10 y en Gato donde comienza el ciclo para
// hacer la opción gatillo. Si no se quiere y es libre, se ig-
// norará ese resultado y se tomará cero por comienzo.
// Si la frecuencia es cero o p1 es 255, es porque no pudo
// detectarse un ciclo completo.
unsigned long int analiza(unsigned int cap, byte &p1)
{
 #if defined(__AVR_ATtiny85__)

 // Guardo estado del port
 byte ddr=DDRB;
 byte port=PORTB;

 #endif

 cli();
 escala=configurarEscala(cap,0);

 #if defined(__AVR_ATtiny85__)

 // Alta impedancia, para leer el comparador interno.
 // Así leo el pulsador 1 y poder abortar la lectura
 // Y también leo el pulsador 2
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));

 // Habilito comparador para detectar el pulsador
 ACSR&=~(1<<ACD);

 delai(4);     // Doy tiempo para estabilizar el comparador
 
 #endif

 habilitarT1();         // Lanzo la captura
 while(!finCaptura());  // Espero a que termine

 #if defined(__AVR_ATtiny85__)

 // Devuelvo estado del port
 DDRB=ddr;
 PORTB=port;

 #endif

 if(aborta) return 0xFFFFFFFF;

 // Ya tengo datos. Ahora a analizar.
 // Hago un cálculo de frecuencia
 unsigned long int hz_x10=0;

 // Cálculo de los picos en todo el área de análisis
 byte vMin=255;
 byte vMax=0;
 for(byte i=0;i<CAPTURAS_TOTAL;i++)
 {
  if(capturas[i]>vMax) vMax=capturas[i];
  if(capturas[i]<vMin) vMin=capturas[i];
 }

 byte nivel=(vMin+vMax)>>1;
 byte amplitud=vMax-vMin;
 int h=amplitud>>2;   // 1/4 amplitud
 
 int nivel_bajo=nivel-h;
 int nivel_alto=nivel+h;

 if(nivel_bajo<0) nivel_bajo=0;
 if(nivel_alto>255) nivel_alto=255;

 p1=255;
 byte ultimo=255;
 byte periodos=0;
 bool armado=false;

 for(byte i=1;i<CAPTURAS_TOTAL;i++)
 {
  byte m_prev=capturas[i-1];
  byte m=capturas[i];
  if(!armado)
  {
   if(((band&BAND_FLANCO)==0))
   {
    if(m<nivel_bajo) armado=true;
   }
   else
   {
    if(m>nivel_alto) armado=true;
   }
  }
  else
  {
   if((band&BAND_FLANCO)==0)
   {
    if(m_prev<nivel && m>=nivel)
    {
     if(p1==255)
     {
      p1=i;
      ultimo=i;
     }
     else
     {
      ultimo=i;
      periodos++;
     }
     armado=false;
    }
   }
   else
   {
    if(m_prev>nivel && m<=nivel)
    {
     if(p1==255)
     {
      p1=i;
      ultimo=i;
     }
     else
     {
      ultimo=i;
      periodos++;
     }
     armado=false;
    }
   }
  }
 }

 if(periodos>0)
 {
  // Calculo sobre el tiempo TOTAL transcurrido entre el
  // primer y último pulso
  unsigned long int deltaTotal_us=(unsigned long)(ultimo-p1)*(unsigned long)tiempoReal_us;
  if(deltaTotal_us>0)
  {
   // Multiplico por periodos ANTES de dividir para ganar
   // resolución
   hz_x10=(10000000UL*periodos)/deltaTotal_us;
  }
 }

 // No se detectó absolutamente nada
 // (Línea plana / Sin flancos)
 // demasiado chica como para confiar en que sea señal real
 if(p1==255 || amplitud<AMPLITUD_MINIMA_DETECCION)
 {
  hz_x10=0; // Fuerzo frecuencia a cero
  // Grafica de forma libre desde el inicio de la captura
  p1=0;
 }
 // Hubo señal, pero el ciclo empieza fuera del área visible
 // del OLED
 else if(p1>CAPTURAS_TOTAL-128) 
 {
  // Indico con 255 que el gatillo está fuera de rango
  p1=255;
 }
 
 // Si no entra en los anteriores, p1 conserva su valor
 // original (0 a 127) y hz_x10 conserva su frecuencia calcu-
 // lada con éxito.
 return hz_x10;
}

// Visualiza un número entero con espacios de alineación
// a la izquierda
void pantalla_enteros(unsigned long int valor, byte ancho)
{
 unsigned long int temp=valor;
 byte digitos=(valor==0)?1:0;

 // Cuenta cuántos dígitos tiene el número
 while(temp>0)
 {
  temp/=10;
  digitos++;
 }
 // Evita el subdesbordamiento si el número es
 // más ancho que el límite
 if(ancho>digitos)
 {
  byte espacios=ancho-digitos;
  for(byte i=0;i<espacios;i++)
  {
   pantalla_print(' ');
  }
 }
 // Imprime el número final
 pantalla_print(valor);
}

// Visualiza una cifra con un decimal y con espacios delante
void pantalla_decimal(unsigned long int val, byte enteros)
{
 pantalla_enteros(val/10,enteros);
 pantalla_print(',');
 pantalla_print(val%10);
}

#if defined(__AVR_ATmega328P__)

void SerialPrintDecimal(unsigned long int val)
{
 Serial.print(val/10);
 Serial.print(',');
 Serial.print(val%10);
}

#endif

void actualizar(int comienzo, unsigned long int hz_x10, unsigned int ms)
{
 // Calculo si es posible hacer lupa.
 // Si es así, multiplico por 2 o 3 lo que se ve
 // Recorro todos los valores. Todos tienen que ser menor a
 // adc_fondo_escala/2 o 3 para que puedan expandirse
 byte lupa=3;
 for(byte i=0;i<128;i++)
 {
  if(capturas[comienzo+i]>=(adc_fondo_escala+adc_offset)/3)
  {
   lupa=2;
   if(capturas[comienzo+i]>=(adc_fondo_escala+adc_offset)/2)
   {
    lupa=1;
    break;
   }
  }
 }

 static byte yAnterior=0;
 static bool primerPunto=true;
 byte vMax=0;

 unsigned int factorY=((unsigned int)(ALTURA_MAX)<<8)/adc_fondo_escala;

 // Líneas de referencia (en coordenadas de pantalla)
 byte y0  =ALTURA_MAX-(((unsigned int)                       factorY)>>8);
 byte y25 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala/4)  *factorY)>>8);
 byte y50 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala/2)  *factorY)>>8);
 byte y75 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala*3/4)*factorY)>>8);
 bool exceso=false;
 for(byte pagina=0;pagina<OLED-1;pagina++)
 { 
  pantalla_cursor(0,pagina);
  pantalla_comienzoDatos();

  primerPunto=true;

  // Qué filas (0 a 7) de esta página caen sobre una línea
  // de referencia. Se calcula una vez por página, en vez de
  // una vez por cada una de las 128 columnas.
  byte filasGrilla=0;
  if(band&BAND_MOSTRARGRILLA)
  {
   for(byte b=0;b<8;b++)
   {
    byte yr=pagina*8+b;
    if(yr==y0||yr==y25||yr==y50||yr==y75||yr==0) filasGrilla|=(1<<b);
   }
  }

  for(byte x=0;x<128;x++)
  {
   // Calculo vpp/%. La onda siempre parte de cero
   // Veo vpp/% a partir de lo que se ve
   // Ponerlo acá ahorra código aunque se procese varias veces
   byte v=capturas[x+comienzo];
   if(v>adc_offset)
   {
    v-=adc_offset;
    if(v>vMax) vMax=v;
   }

   byte byteSalida=0x00;

   // Veo si se produce expansión de la escala
   byte e=((band&BAND_ESCALAR)?x/4:x);

   // Aumento visualización, si corresponde
   int datoTemp=capturas[comienzo+e]-adc_offset;
   if(datoTemp<0) datoTemp=0;
   byte dato=(byte)(datoTemp*lupa);

   // Encajo la lectura dentro del patrón de puntos
   // de la pantalla, que dará un valor entre 0 y ALTURA_MAX
   // para que las discrepacias de unas pocas cuentas, encajen
   // en una misma hilera de puntos.
   // Si dato supera el fondo de escala, no se muestra
   if(dato<=adc_fondo_escala)
   {
    byte y_pixel=((unsigned int)dato*factorY)>>8;
    if(y_pixel>ALTURA_MAX) y_pixel=ALTURA_MAX;
    byte filaInvertida=ALTURA_MAX-y_pixel;
    if(band&BAND_MODOLINEA)
    {
     if(primerPunto) yAnterior=filaInvertida;
     byte yMin=(filaInvertida<yAnterior)?filaInvertida:yAnterior;
     byte yMax=(filaInvertida>yAnterior)?filaInvertida:yAnterior;
     for(byte y=yMin;y<=yMax;y++)
     {
      if((y/8)==pagina) byteSalida|=(1<<(y%8));
     }
     yAnterior=filaInvertida;
     primerPunto=false;
    }
    else
    {
     if((filaInvertida/8)==pagina) byteSalida|=(1<<(filaInvertida%8));
    }
   }
   else
   {
    exceso=true;
   }
   // GRILLA
   if(band&BAND_MOSTRARGRILLA)
   {
    if((x%20)==0) byteSalida|=0x88;         // Lineas verticales
    if((x%4)==0)  byteSalida|=filasGrilla;  // punteada
   }
   i2c_escribir(byteSalida);
  }
  i2c_parar();
 }

 // Pie de la pantalla

 // Ahora escribo los valores en posiciones fijas
 pantalla_cursor(0,PAGINA_ESTADO);
 pantalla_modoTipo(0);
 pantalla_separacion(1);

 // Pone si está magnificado todo (1, 2 o 3)
 pantalla_print(lupa);

 // Autoescala o manual
 pantalla_print((band&BAND_AUTOESCALA)?'A':'M');

 // Si está escalada o normal
 pantalla_print((band&BAND_ESCALAR)?'4':'N');

 pantalla_separacion(3);

 // Verdadero: Espera cruce, falso: Barrido libre
 pantalla_print((band&BAND_MODOGATILLO)?((band&BAND_FLANCO)?'-':'+'):Libre);

 pantalla_separacion(0);

 pantalla_enteros(ms,4);

 pantalla_print('u');
 pantalla_separacion(0);
 pantalla_print(' ');

 // Muestra la frecuencia. No muestra debajo de 0,5 Hz
 pantalla_separacion(1);
 if(hz_x10<=5)
 {
  pantalla_print(FPSTR(RAYAS));  // No hay
 }
 else
 {
  // Redondeo a 1 Hz a hz_x10 cuando da decimales
  pantalla_enteros((hz_x10+5)/10,5);
 }

 pantalla_separacion(1);
 pantalla_print('H');

 if(exceso)
 {
  pantalla_print(FPSTR(SAT));
 }
 else
 {
  // Muestro el porcentaje. No se debe simplificar la fórmula,
  // para poder truncar.
  unsigned int p=(unsigned int)vMax*100/(unsigned int)adc_fondo_escala;
 
  pantalla_enteros(p,4);
  pantalla_print('%');
 }

 // Que tensión seleccionada usa
 pantalla_print(rangoActual);
}

// Obtengo un valor estable del ADC de ADC0/1 para tener una
// lectura firme, usando el prescaler al máximo.
byte leerADCestable(void)
{
 // Guardo configuración actual
 byte Admux=ADMUX;
 byte Adcsra=ADCSRA;

 #if defined(__AVR_ATtiny85__)

 // Leo un selector en pata de reset/ADC0
 // VCC ref y Justificación Izquierda (8 bits)
 ADMUX=(0<<REFS1)|(0<<REFS0)|(1<<ADLAR);

 #else

 // ATmega328P: AVcc como referencia, ajuste a izquierda, ADC1
 ADMUX=(1<<REFS0)|(1<<ADLAR)|1;

 #endif

 // ADC habilitado, prescaler 128
 ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

 // Inicio una conversión de descarte. El micro necesita purgar
 // la carga de la referencia anterior (2,56 V / 1,1 V)
 adira();

 // Lectura válida
 byte valor=adira();

 // Restauro configuración anterior
 ADCSRA=Adcsra;
 ADMUX=Admux;

 return valor;
}

// Leo que selección se hace, al iniciar el equipo
// Se crean selectores en reset
// Las selecciones dan 0 a 6. 255 es un valor no válido
byte leerSelector(void)
{
 // En los modos no osciloscopio, no existe abortar
 // Leo un selector en pata de reset/ADC0 en ATtiny85 y en
 // PC1/A1 para el ATmega328P. Y funciona bien.
 // Son leídos cuando termina de medir o cuando es frecuencia.
 // Uso mismos valores y criterio para los dos procesadores,
 // para no hacerlo complicado. Pero en ATmega328P pueden ha-
 // cerse más selecciones, ya que puede recorrer desde 0 hasta
 // 255, cambiando los valores de resistencias, sacando la de
 // 12kΩ que está en el diseño y poniendo en reemplazo, 0 Ω y
 // aumentar la cantidad de posiciones de eeprom para guardar
 // esos valores, y también modificar inicio() para poner los
 // de omisión. En ATmega328P hay más lugar para código nuevo.
 // Ahora inicio la conversión y leo un valor estable
 byte res=leerADCestable();

 // Se calibra a través del menú config que se activa
 // manteniendo apretado el pulsador 1 (menos) en el momento de
 // encendido. Se calibran las posiciones 2 a 6, a causa de
 // la posible dispersión de tolerancias de las resistencias.
 //   0 Es insertar accesorio
 //   1 Es Osciloscopio X
 //   2 Es Osciloscopio Y
 //   3 Es Osciloscopio Z
 //   4 Es Generador
 //   5 Es Frecuencímetro
 //   6 Es Sensor
 // 255 Es un valor no válido y no debe tenerse en cuenta.

 // Quien quiera puede agregar en caso de un ATmega328P, combi-
 // naciones 7 hasta 253.

 // La rama de entrada tiene una R de 10 kΩ y otra de 12 kΩ
 // Fórmula: donde R es el valor en kΩ y da cuentas de ADC.
 // Para el ATtiny85: 20,265 es el resultado de la resistencia
 // interna de la pa ta de reset en paralelo con la de 10kΩ y
 // en serie con la de 12 k, que en los ensayos resultó 47,6 kΩ
 // internos. La fórmula, si se desea, puede corregirse por la
 // dispersión que puede haber entre otros ATtiny85.
 // En ATmega328P: La pata del ADC no lleva un pullup interno
 // y por lo tanto la fórmula es perfecta con los valores de
 // resistencias reales.
 // Los valores de resistencia están en kΩ

 // Cálculo del valor de ADC para una R de selector dada:
 // ADC = 255*((R+12)/(R+20,265)) Para ATtiny85
 // ADC = 255*((R+12)/(R+22))     Para ATmega328P

 // Y teniendo un ADC deseado, calculo la R selectora:
 // R = (ADC*20,265−255*12)/(255-ADC) Para ATtiny85
 // R = (ADC*22−255*12)/(255-ADC)     Para Atmega328P

 // Uso valores normalizados y este es el resultado de mis
 // ensayos con resistencias de tolerancia 5 %.
 //    R     ADC85 ADC328 Conector
 // Infinito  250   250   Insertar accesorio
 //    0 kΩ   151   139   Osc 'X'
 //  5,6 kΩ   172   163   Osc 'Y'
 //   10 kΩ   186   175   Osc 'Z'
 //   22 kΩ   203   197   Generador
 //   47 kΩ   221   218   Frecuencia
 //  150 KΩ   241   240   Sensor

 // Veo en cual encaja la lectura y obtengo el
 // valor de retorno
 if(res>=SEL_MAX) return 0; // No necesita comparar con eeprom

 // Busco la resistencia calibrada más cercana a la lectura,
 // en un solo recorrido de la tabla.
 byte mejorPos=255;
 byte mejorDist=255;
 for(byte i=0;i<6;i++)
 {
  byte v=eeprom_read_byte(&ee_r[i]);
  byte dist=(res>v)?(res-v):(v-res);
  if(dist<mejorDist)
  {
   mejorDist=dist;
   mejorPos=i+1;
  }
 }
 // Si la más cercana de todos modos está fuera de la toleran-
 // cia máxima, es que cae en un hueco entre valores calibra-
 // dos. Se mostrará como un "estado inválido" para corregir
 // el valor la resistencia a uno más apropiado.
 if(mejorDist>SEL_TOL) return 255;
 return mejorPos;
}

// Lectura del pulsador. Verdadero si es presionado. Solo es
// leído cuando termina de medir o cuando es frecuencia o cfg.
// En modo osciloscopio, entre análisis. Los valores son:
// Da 0 si no se apretaron los pulsadores, da 1 si se apretó
// pulsador MENOS, un 2 para el pulsador MAS. Cero también sig-
// nifica falso, para saber si hubo presión de un pulsador.
// Se necesitan unos ms para estabilizar el comparador cuando
// se usa el pulsador 1.
byte leerPulsador(void)
{
 // Pido leer pulsadores cuando haya atendido una interrupción
 // cuando es generador porque va a cambiar el port B.
 // No puedo usar a fincaptura(), ya que el port B espera estar
 // alterado antes de ser llamado. Aquí, simplemente si está
 // el modo generador, espero a pido para sincronizar con la
 // interrupción, para hacer todo
 if(modoActual==(byte)MODO_GENERADOR)
 {
  // Debo asegurarme que haya interrupciones
  // para que funcione pido.
  pido=true;
  while(pido);   // Espero a que atienda la interrupción
 }
 // Busco el pulsador diferencial en ATtiny85 que es el pulsa-
 // dor 1 De paso da tiempo a estabilizar la referencia de
 // 2,56 V. Guardo estado del PORT B para no molestar al bus
 // i2C bitbang,  ya que lo uso para leer el pulsador
 #if defined(__AVR_ATtiny85__)
 byte ddr=DDRB;
 byte port=PORTB;

 // Alta impedancia, para leer el comparador interno.
 // Y de paso puedo leer un segundo pulsador en PB1
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));

 ACSR&=~(1<<ACD);         // Habilito comparador

 // Busco pulsación estable del comparador
 // No puedo usar delay en modo generador
 if(modoActual!=(byte)MODO_GENERADOR) delai(4);  // Estabilizo

 // Leo el pulsador 2
 bool pul2=!(PINB & (1<<PB1));

 // Leo pulsador 1
 bool pul=!(ACSR & (1<<ACO));

 #else

 // Leo pulsador 1
 bool pul=!(PIND & (1<<PD2));

 // Leo pulsador 2
 bool pul2=!(PIND & (1<<PD7));

 #endif

 #if defined(__AVR_ATtiny85__)

 // Devuelvo estado del port
 DDRB=ddr;
 PORTB=port;

 #endif

 if(pul)  return 1;
 if(pul2) return 2;

 // Nada de lo anterior, es 0
 return 0;
}

// Devuelve la opción elegida
byte menu(void)
{
 // Veo si hubo una pulsación para hacer menú. Si no, salgo
 byte cual=leerPulsador();

 // Solo si es 1 o 2, hace menú
 if(cual<1) return 0;

 const char* const* ptrMenu;
 byte inicio=0;
 byte finBloque1,finBloque2;

 // Selección de tabla y asignación directa de ambos
 // límites reales
 if(!modoActual)        // MODO_OSCILOSCOPIO
 {
  ptrMenu=menuOsc;
  finBloque1=(byte)MENU_OSC_PUL1_VOLVER;
  finBloque2=(byte)MENU_OSC_PUL2_VOLVER;
 }
 else if(modoActual==(byte)MODO_GENERADOR)
 {
  ptrMenu=menuGen;
  finBloque1=(byte)MENU_GEN_PUL1_VOLVER;
  finBloque2=(byte)MENU_GEN_PUL2_VOLVER;
 
  #if defined(__AVR_ATtiny85__)
  TIMSK&=~(1<<OCIE1A);
  #else
  TIMSK1&=~(1<<OCIE1A);
  #endif
  modoActual=255;   // Para que no actue pido en el menú
 }
 else if(modoActual==(byte)MODO_CFG)
 {
  ptrMenu=menuCfg;
  finBloque1=(byte)MENU_CFG_PUL1_VOLVER;
  finBloque2=(byte)MENU_CFG_PUL2_VOLVER;
 }
 // El modo frecuencia y sensor no llaman al menú.

 // Ajuste definitivo de ventana según el pulsador detectado
 byte fin=finBloque1;
 if(cual==1)    // Pulsador Menos, debe usar la segunda parte
 {
  inicio=finBloque1;
  fin=finBloque2;
 }

 // Veo si se mantiene apretado
 byte cant=fin-inicio;
 byte i=0;
 while(leerPulsador())
 {
  pantalla_limpia();
  if(++i>cant) i=1;
  pantalla_println((__FlashStringHelper*)pgm_read_word(ptrMenu+inicio+i-1));
  delai(PRESION_PULSADOR);
 }

 // Devuelvo interrupciones en el modo generador
 if(modoActual==255)
 {
  modoActual=(byte)MODO_GENERADOR;
  #if defined(__AVR_ATtiny85__)
  TIMSK|=(1<<OCIE1A);
  #else
  TIMSK1|=(1<<OCIE1A);
  #endif
 }
 return inicio+i; 
}

// En modo generador, las frecuencias se ajustan según la
// escala perfecta.
void activarGenerador(unsigned int fDeseada_Hz)
{
 cli();

 modoActual=(byte)MODO_GENERADOR;

 unsigned int mejorEscala=ESCALA_MINIMA + 2;
 unsigned int mejorRecarga=1;
 unsigned long int mejorFrecuencia_mHz=0;
 unsigned long int mejorError=0xFFFFFFFF;
 bool mejorPerfecta=false;

 // Frecuencia objetivo unificada estrictamente en miliHertz
 unsigned long int objetivo_mHz=(unsigned long int)fDeseada_Hz*1000UL;

 unsigned int esc=configurarEscala(ESCALA_MINIMA+2,0);

 while(esc<=ESCALA_MAXIMA)
 {
  if(tiempoReal_us>0)
  {
   // Frecuencia de la ISR de hardware en miliHertz
   unsigned long int f_isr_mHz=1000000000UL/(unsigned long int)tiempoReal_us;

   // Ambos términos operan en miliHertz
   // de forma homogénea
   // R = F_isr_mHz / (2 * F_objetivo_mHz)
   unsigned long int divisor_r=2UL*objetivo_mHz;
   unsigned long int r_ideal=1;
   
   if(divisor_r>0)
   {
    r_ideal=f_isr_mHz/divisor_r;
   }
   if(r_ideal==0) r_ideal=1;

   // Calculo la frecuencia real final que se va a generar
   // con este r_ideal
   unsigned long int f_calculada_mHz=f_isr_mHz/(2UL*r_ideal);
   
   // Error absoluto en miliHertz
   unsigned long int error=(f_calculada_mHz>objetivo_mHz)?(f_calculada_mHz-objetivo_mHz):(objetivo_mHz-f_calculada_mHz);

   if(error<mejorError)
   {
    mejorError=error;
    mejorEscala=esc;
    mejorRecarga=(unsigned int)r_ideal;
    mejorFrecuencia_mHz=f_calculada_mHz;
    mejorPerfecta=(error==0);
    if(mejorPerfecta) break;
   }
  }

  unsigned int escSiguiente=configurarEscala(esc, 1);
  if(escSiguiente==esc) break; 
  esc=escSiguiente;
 }

 // Consolidación de los registros de la escala ganadora
 configurarEscala(mejorEscala,0);

 // Seteo coordinado de los contadores para la ISR
 recargaFrecuencia=mejorRecarga;
 contadorFrecuencia=mejorRecarga; 

 pantalla_limpia();

 // Interfaz OLED
 pantalla_println(FPSTR(QUIERO));
 pantalla_enteros(fDeseada_Hz, 5);
 pantalla_println(FPSTR(CHZ));

 pantalla_print(FPSTR(DA));
 pantalla_print(mejorPerfecta?'=':'#');
 pantalla_println();
 pantalla_decimal(mejorFrecuencia_mHz/100,5); 
 ihz();

 #if defined(__AVR_ATtiny85__)
 DDRB|=(1<<PB2);
 #else
 // Configuro pata de Generador de Funciones. Siempre activa
 // Pongo como salida a D14 (PC0)
 DDRC|=(1<<PC0);
 #endif

 habilitarT1();
}

void restaurarOsc(void)
{
 cli();

 modoActual=(byte)MODO_OSCILOSCOPIO;

 #if defined(__AVR_ATtiny85__)

 DDRB&=~(1<<PB2);          // PB2 vuelve a ser entrada
 PORTB&=~(1<<PB2);         // Sin pullup

 // Configuración ADC (PB2/ADC1)
 // Referencia de 2,56 V
 // (Es como poner ADMUX=0xB1 pero lo detallo)
 ADMUX=(1<<REFS2)|(1<<REFS1)|(0<<REFS0)|(1<<ADLAR)|(1<<MUX0);
 
 delai(20); 

 // Prepara la conversión de limpieza
 ADCSRA=0xC3;  // ADEN, ADSC y Prescaler /8

 #else

 // Caso ATmega328P

 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado

 DDRC&=~(1<<PC0);    // PC0 vuelve a ser entrada
 PORTC&=~(1<<PC0);   // Sin pullup

 // Configuro ADC para máxima velocidad inicial
 // Mantiene 1.1V y canal ADC0/PC0
 ADMUX=(1<<REFS1) | (1<<REFS0);

 // ADCSRA: Habilito ADC e interrupciones
 // Prescaler inicial de 128 para estabilidad
 ADCSRA=(1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

 // Configuro Temporizador 1 (16 bits)
 TCCR1A=0; 
 TCCR1B=0;
 TCNT1=0;

 // Alinea el resultado para leer solo ADCH
 ADMUX|=(1<<ADLAR);

 #endif

 // Realizar una conversión de "limpieza" para asentar
 // la referencia
 adira();

 // Restaura la escala que tenía el osciloscopio
 escala=configurarEscala(escala,0);
 sei();
}

void hecho(void)
{
 pantalla_print(FPSTR(HECHO));
}

void busco(void)
{
 pantalla_limpia();
 pantalla_print(FPSTR(BUSCO));
 d3000();                   // Busca cada tres segundos
}

// Verdadero si aborta con pulsador menos. Pulsador más, acepta
// O sea:
// 'Sí' es el más y da falso, 'No' es el menos y da verdadero.
bool veoAborta(void)
{
 pantalla_println(FPSTR(SINO));
 while(leerPulsador());  // Espero a soltar del menú
 delai(250);
 byte sale;
 do                      // Espero a apretar
 {
  sale=leerPulsador();
 } while(!sale);
 pantalla_limpia();
 if(sale==2) return false;
 pantalla_print(FPSTR(ABORTA));
 return true;
}

void anah(byte antes, byte ahora)
{
 pantalla_print(FPSTR(ANTES));
 pantalla_print(antes);
 pantalla_println();
 pantalla_print(FPSTR(AHORA));
 pantalla_print(ahora);
 pantalla_println();
}

// Acepta y guarda, o descarta
void confirmarGuardar(byte anterior, byte nuevo, bool valido, byte *direccionEE)
{
 anah(anterior,nuevo);
 if(!veoAborta())
 {
  pantalla_limpia();
  if(valido)
  {
   eeprom_update_byte(direccionEE,nuevo);
   hecho();
  }
  else
  {
   pantalla_print(FPSTR(ABORTA));
  }
 }
 d3000();
}

void calibrarRangoGenerico(char rangoActivo)
{
 restaurarOsc();
 byte Gato;
 analiza(10,Gato);

 unsigned int suma=0;
 adc_offset=eeprom_read_byte(&ee_adc_fe0);
 byte actual=adc_offset;

 for(byte i=64;i<(128+64);i++) suma+=capturas[i];
 suma/=128;

 byte *direccion;
 bool valido;

 if(rangoActivo!='0')
 {
  if(suma<adc_offset) suma=0; else suma-=adc_offset;
  actual=eeprom_read_byte(&ee_adc_fe[rangoActivo-'X']);
  direccion=&ee_adc_fe[rangoActivo-'X'];
  valido=(suma>=ALTURA_MAX && suma<=ADC_SAT_ALTO);
 }
 else
 {
  direccion=&ee_adc_fe0;
  valido=(suma<=ADC_SAT_BAJO*2);
 }
 confirmarGuardar(actual,suma,valido,direccion);
}

void ihz(void)
{
 pantalla_println(FPSTR(HZ));
}

// Calibra la frecuencia
#ifdef SIN_CRISTAL
void calibrarFrecuencia(void)
{
 // Debe estar en modo osciloscopio para poder medir
 restaurarOsc();
 if(veoAborta()) return;

 byte minimo_error=255;
 byte maximo_error=0;
 byte osqui=OSCCAL;       // Guardo el que estaba
 for(byte osccal=2;osccal<254;osccal++)
 {
  pantalla_cursor(0,0);
  pantalla_println(FPSTR(BUSCO));

  OSCCAL=osccal;
  delai(10);               // Estabiliza RC

  // Busco que dé una frecuencia valida
  byte Gato;

  // 600 ms por punto. Múltiplo de 50 y 60 Hz
  unsigned int r=analiza(60,Gato);

  // Analizo si da una frecuencia
  pantalla_decimal(r,5);
  ihz();

  // Analizo para buscar el valor correcto
  #ifdef CAL50
  if(r>=500) minimo_error=osccal;
  if(r<=500) maximo_error=osccal;
  #else
  if(r>=600) minimo_error=osccal;
  if(r<=600) maximo_error=osccal;
  #endif

  // Si mínimo error pasa a ser menor que máximo error, o
  // son iguales, ya terminó.
  if(minimo_error<=maximo_error) break;
 }
 // Aplicar y guardar, si no hubo error. Si no, dejo
 // el que estaba
 if(minimo_error>maximo_error)
 {
  // Debo recuperar a OSCCAL
  OSCCAL=osqui;
  pantalla_println(FPSTR(ABORTA));
 }
 else
 {
  // Guardo el valor calibrado
  eeprom_update_byte(&ee_osccal,OSCCAL);
  pantalla_println(FPSTR(HECHO));
 }
}
#endif

void acercade(void)
{
 pantalla_limpia();
 pantalla_println(F(EQUIPO TIPOF));
 pantalla_separacion(0);
 pantalla_println(F(VERSION "\n" FECHA));
 pantalla_separacion(1);
 pantalla_println(F(AUTOR));
 d3000();
}

void modoCFG(void)
{
 restaurarOsc();       // Pone como entrada a PB2
 modoActual=(byte)MODO_CFG;
 pantalla_limpia();
 pantalla_print(FPSTR(CONFIG));
}

void printFrec(void)
{
 pantalla_println(FPSTR(FRECUENCIA));
}

// Pone los valores por omisión en la EEPROM

void inicio(void)
{
 eeprom_update_byte(&ee_id1,EEPROM_ID1);
 eeprom_update_byte(&ee_id2,EEPROM_ID2);
 eeprom_update_byte(&ee_band,(BAND_MODOGATILLO | BAND_AUTOESCALA));
 eeprom_update_byte(&ee_adc_fe0,ADC_SAT_BAJO);   // Cal 0V
 eeprom_update_byte(&ee_adc_fe[0],ADC_SAT_ALTO); // Vpp X
 eeprom_update_byte(&ee_adc_fe[1],ADC_SAT_ALTO); // Vpp Y
 eeprom_update_byte(&ee_adc_fe[2],ADC_SAT_ALTO); // Vpp Z
 #ifdef SIN_CRISTAL
 eeprom_update_byte(&ee_osccal,OSCCAL); // Valor inicial
 #endif
 for(byte i=0;i<6;i++)
 {
  // Pongo valores por primera vez. Después cambiarán
  // al calibrar.
  eeprom_update_byte(&ee_r[i],selValores[i]);
 }
}

void delai(int d)
{
 delay(d);
}

void d3000(void)
{
 delai(3000);
}

// Pone un 0
static inline void sensorEn0(void)
{
 DDR_SENSOR|=(1<<PATA_SENSOR);      // Salida
 PORT_SENSOR&=~(1<<PATA_SENSOR);    // 0
}

// Libero línea (pullup). Es como poner un 1
static inline void sensorEn1(void)
{
 PORT_SENSOR|=(1<<PATA_SENSOR);     // 1
 DDR_SENSOR&=~(1<<PATA_SENSOR);     // Entrada con PULLUP
}

// Leer estado
static inline byte sensorLeePata(void)
{
 return (PIN_SENSOR & (1<<PATA_SENSOR))!=0;
}

#ifdef CON_DS18B20
byte sensorInicio(void)
{
 byte r;
 sensorEn0();
 delayMicroseconds(480);
 sensorEn1();
 delayMicroseconds(70);
 r=!sensorLeePata();   // pulso presencia
 delayMicroseconds(410);
 return r;
}

// Esta se optimiza con 20 MHz
#if F_CPU==20000000UL

void sensorPoneBit(byte b)
{
 if(b)
 {
  sensorEn0();
  delayMicroseconds(6);
  sensorEn1();
  delayMicroseconds(64);
 }
 else
 {
  sensorEn0();
  delayMicroseconds(60);
  sensorEn1();
  delayMicroseconds(10);
 }
}
#else

// Para los demás cristales
// Espera en µs resuelta en tiempo de compilación.
// Divido F_CPU primero para no desbordar 32 bits con 480*20000000.
#define ESPERA_US(x) __builtin_avr_delay_cycles(((F_CPU)/1000000UL)*(x))

void sensorPoneBit(byte b)
{
 if(b)
 {
  sensorEn0();
  ESPERA_US(6);
  sensorEn1();
  ESPERA_US(64);
 }
 else
 {
  sensorEn0();
  ESPERA_US(60);
  sensorEn1();
  ESPERA_US(10);
 }
}

#endif

byte sensorLeeBit(void)
{
 byte r;
 sensorEn0();
 delayMicroseconds(6);
 sensorEn1();
 delayMicroseconds(9);
 r=sensorLeePata();
 delayMicroseconds(55);
 return r;
}

void sensorPoneByte(byte b)
{
 for(byte i=0;i<8;i++)
 {
  sensorPoneBit(b&1);
  b>>=1;
 }
}

byte sensorLeeByte(void)
{
 byte r=0;
 for(byte i=0;i<8;i++)
 {
  r>>=1;
  if(sensorLeeBit()) r|=0x80;
 }
 return r;
}

//#ifdef CON_DS18B20
// Reset + Saltear ROM + comando. Se repite 2 veces, lo unifico.
byte sensorComando(byte cmd)
{
 if(!sensorInicio()) return 0;
 sensorPoneByte(0xCC);   // Saltear ROM
 sensorPoneByte(cmd);
 return 1;
}

int16_t leeDS18B20(void)
{
 byte l,h;
 if(!sensorComando(0x44)) return 32767;   // Convertir T

 // Espera conversión
 while(!sensorLeeBit());

 if(!sensorComando(0xBE)) return 32767;   // Lee Scratchpad
 l=sensorLeeByte();
 h=sensorLeeByte();
 return ((int16_t)h<<8)|l;
}
#endif

// Según lo conectado, elige. Solo cambia la función, en el
// momento de cambio del conector.
void poneModos(void)
{
 // Config no altera los modos
 if(modoActual==(byte)MODO_CFG) return;

 // Pone un modo.

 // Selectores:
 //  0) Sin accesorio
 //  1) Osciloscopio 'X'
 //  2) Osciloscopio 'Y'
 //  3) Osciloscopio 'Z'
 //  4) Generador
 //  5) Frecuencímetro
 //  6) Sensor de temperatura
 //255) No válido

 byte ls=leerSelector();

 // Si el selector no cambia, no cambio el modo
 // La primera vez lsant tiene 254 para obligar a elegir modo.
 if(ls==lsant) return;

 lsant=ls;

 // Apago la ISR del generador, si estaba activa. No molesta
 // para los otros modos.
 #if defined(__AVR_ATtiny85__)
 TIMSK&=~(1<<OCIE1A);
 #else
 TIMSK1&=~(1<<OCIE1A);
 #endif

 // Indico el modo
 pantalla_limpia();

 // 0 es sin accesorio enchufado
 if(ls==0)
 {
  // Sin nada conectado
  modoActual=(byte)MODO_ESPERA;
  pantalla_println(FPSTR(ESPERA));
  // No pongo modelo y versión en SIN_CRISTAL
  #ifndef SIN_CRISTAL
  pantalla_println();
  pantalla_println(F(EQUIPO " " VERSION));
  #endif
  #if defined(__AVR_ATmega328P__)
  Serial.println(FPSTR(SELESPERA));
  Serial.println(F(EQUIPO " " VERSION));
  #endif
 }
 // Modo osciloscopio 1 a 3 / X a Z
 else if(ls<4)
 {
  #if defined(__AVR_ATmega328P__)
  Serial.print(FPSTR(OSCISERIE));
  #endif
  pantalla_print(FPSTR(OSCILOSCOPIO));
  restaurarOsc();

  // Esto ayuda al insertar el accesorio, a buscar escala
  if(band&BAND_AUTOESCALA)
  {
   escala=configurarEscala(ESCALA_MAXIMA,0);
  }

  // Muestreo de prueba para que no se trabe el arranque
  // Traigo valores guardados
  rangoActual=ls+'X'-1;
  pantalla_print(rangoActual);
  #if defined(__AVR_ATmega328P__)
  Serial.print(rangoActual);
  Serial.println('\'');
  #endif
  adc_fondo_escala=eeprom_read_byte(&ee_adc_fe[rangoActual-'X']);
  adc_offset=eeprom_read_byte(&ee_adc_fe0);
  d3000();
 }
 // 4 es generador
 else if(ls==4)
 {
  #if defined(__AVR_ATmega328P__)
  Serial.println(FPSTR(GENERADOR));
  #endif
  pantalla_print(FPSTR(GENERADOR));
  d3000();
  activarGenerador(genFrec);  // Comienzo en 50 Hz
 }
 // 5 es frecuencímetro
 else if(ls==5)
 {
  #if defined(__AVR_ATmega328P__)
  Serial.println(FPSTR(FRECUENCIMETRO));
  #endif
  printFrec();
  d3000();
  #if F_CPU==20000000UL
  limite_cuentas=252;
  #else
  limite_cuentas=250;
  #endif
  pantalla_limpia();
  printFrec();
  restaurarOsc();       // Pone como entrada a PB2 o PD5/PC0
  modoActual=(byte)MODO_FREC;
 }
 // 6 es sensor de temperatura / humedad
 else if(ls==6)
 {
  #if defined(__AVR_ATmega328P__)
  Serial.println(FPSTR(SENSOR));
  #endif
  pantalla_print(FPSTR(SENSOR));
  modoActual=(byte)MODO_SENSOR;

  // Veo cual sensor uso
  // Sin presencia 1-Wire, asumo DHT
  #ifdef CON_DS18B20
  bitWrite(band,BIT_SENSOR,!sensorInicio());
  #endif
  d3000();
  pantalla_limpia();
 }              // Fin modo sensor
 // 255 u otro valor, es no válido
 else
 {
  // Valores de resistencias no contemplados
  // o fuera de tolerancia
  modoActual=(byte)MODO_NOVALIDO;
  pantalla_print(FPSTR(NOVALIDO));
  #if defined(__AVR_ATmega328P__)
  Serial.println(FPSTR(SELNOVA));
  #endif
 }
}

// SETUP

void setup(void)
{
 #if defined(__AVR_ATmega328P__)
 
 // Configuarción ATmega328P
 // Para que no queden flotantes, pongo a todas en INPUT_PULLUP
 // y luego excluyo las que necesito. Y de paso favorece a los
 // pulsadores.
 // No toco a A0 y A1 que es por donde se mide.
 // A6 y A7 deben cablearse, ya que no es un port real
 for(byte i=0;i<=13;i++)  pinMode(i,INPUT_PULLUP);
 for(byte i=A2;i<=A5;i++) pinMode(i,INPUT_PULLUP);

 // Apago led
 pinMode(13,OUTPUT);
 digitalWrite(13,LOW);

 // PD5 siempre es entrada sin PULLUP
 // Las patas PD5 y PC0 están unidas. Desacoplo a PD5
 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado

 // Saco modelo y autor, por port serie.
 Serial.begin(115200);
 Serial.println();
 Serial.println();
 Serial.println(F(EQUIPO));
 Serial.println(F(VERSION "\n" FECHA));
 Serial.println(F(AUTOR));

 #endif

 // Si es la primera vez, inicio la EEPROM
 if(eeprom_read_byte(&ee_id1)!=EEPROM_ID1 || eeprom_read_byte(&ee_id2)!=EEPROM_ID2)
 {
  #if defined(__AVR_ATmega328P__)
  Serial.println();
  Serial.println(FPSTR(ANUEVO));
  Serial.println();
  #endif
  inicio();
 }

 // Traigo las opciones de band. Saco posible captura
 band=eeprom_read_byte(&ee_band)&~BAND_CAPAN;

 // Traigo calibración de frecuencia
 #ifdef SIN_CRISTAL
 OSCCAL=eeprom_read_byte(&ee_osccal);
 #endif

 pantalla_comienzo();
 pantalla_encendida();

 acercade();     // Espera pulsador o selector

 // Pulsador:
 // 0) No hubo pulsación, sigo.
 // 1) Configurar (pulsador menos)
 // 2) A nuevo (pulsador más)
 byte lp=leerPulsador();
 while(lp==2)
 {
  // Inicializo EEPROM
  // Espero a reiniciar
  pantalla_limpia();
  pantalla_println(FPSTR(ANUEVO));
  if(!veoAborta())
  {
   pantalla_limpia();
   inicio();
   hecho();
  }
  d3000();
  lp=1;         // Pasa a modo config después de reinicarlo
 }
 if(lp==1)
 {
  modoActual=(byte)MODO_CFG;
  modoCFG();
  d3000();
 }
}

//  LOOP

void loop(void)
{
 // Según el selector, lo pone en uno u otro modo,
 // excepto el modo CFG, que se mantiene según el inicio.
 poneModos();

 // Pongo en entrada las patas de entrada y salida,
 // para que entren bien cada modo
 #if defined(__AVR_ATmega328P__)
 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado
 #endif

 // Actúo según el modo elegido
 if(!modoActual) // MODO_OSCILOSCOPIO
 {
  // Mido según la escala actual
  byte Gato;
  byte comienzo=0;

  // También quita el aborto
  unsigned long int hz_x10=analiza(escala,Gato);

  if(!aborta)
  {
   //  Calculo autoescala para que se acomode en la pantalla
   if(band&BAND_AUTOESCALA)
   {
    // Si hubo algo, pero no detecto Hz, usaré la escala máxima
    if(Gato==255)
    {
     escala=configurarEscala(ESCALA_MAXIMA,0);
    }
    else
    {
     // Si se detectó una frecuencia, calculo la escala adecuada
     if(hz_x10>5) // Si hay señal detectable
     {
      // Llevo directo cerca del objetivo
      signed int escalaIdeal=1000000UL/hz_x10/PUNTOS;
      // Busco la siguiente dos escalas válidas
      while(escalaIdeal<ESCALA_MAXIMA && !esEscalaValida(escalaIdeal))
      {
       escalaIdeal++;
      }
      escalaIdeal++;
      if(escalaIdeal>ESCALA_MAXIMA) escalaIdeal=ESCALA_MAXIMA;
      while(escalaIdeal<ESCALA_MAXIMA && !esEscalaValida(escalaIdeal))
      {
       escalaIdeal++;
      }

      // Ahora busco que las escalas no tiemblen
      bool lejos=false;
      // Busco el siguiente escalón válido hacia arriba
      int t=escala;
      do
      {
       t++;
      } while(t<ESCALA_MAXIMA && !esEscalaValida(t));
      if(escalaIdeal>t) lejos=true;
      // Busco el siguiente escalón válido hacia abajo
      t=escala;
      do
      {
       t--;
      } while(t>ESCALA_MINIMA && !esEscalaValida(t));
      if(escalaIdeal<t) lejos=true;
      if(lejos)
      {
       // Pongo la autoescala
       escala=configurarEscala(escalaIdeal,0);
      }
      else
      {
       // Mantengo la que estaba
       escala=configurarEscala(escala,0);
      }
     }
    }
   }
   // Si tras la búsqueda no hay indicios fiables,
   // asumo muestra limpia desde 0
   if(Gato==255) Gato=0;
   if(band&BAND_MODOGATILLO) comienzo=Gato;

   // Muestro
   actualizar(comienzo,hz_x10,tiempoReal_us);

   if(band&BAND_CAPAN)
   {
    pantalla_invertir(true);
    while(!leerPulsador());
    pantalla_invertir(false);
    delai(PRESION_PULSADOR);
   }
  }
  band&=~BAND_CAPAN;

  // Menú OSCILOSCOPIO
  switch(menu())
  {
   case MENU_OSC_CAPTURA:
    band|=BAND_CAPAN;
    break;
   case MENU_OSC_AUTOESCALA:
    band^=BAND_AUTOESCALA;    // Hace flipflop
    band&=~BAND_ESCALAR;
    // Si hace autoescala, saca al resto
    if(band&BAND_AUTOESCALA)
    {
     band|=BAND_MODOGATILLO;
     //band&=~BAND_FLANCO;
    }
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS1:
    escala=configurarEscala(escala,1);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS10:
    escala=configurarEscala(escala,10);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS1:
    escala=configurarEscala(escala,-1);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS10:
    escala=configurarEscala(escala,-10);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAX:
    escala=configurarEscala(ESCALA_MAXIMA,0);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MIN:
    escala=configurarEscala(ESCALA_MINIMA,0);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_LIBRE_AUTO:
    band^=BAND_MODOGATILLO;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_FLANCO:
    band^=BAND_FLANCO;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_GRILLA:
    band^=BAND_MOSTRARGRILLA;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_LINEAS:
    band^=BAND_MODOLINEA;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALAR:
    band^=BAND_ESCALAR;
    eeprom_update_byte(&ee_band,band);
    break;
   default:
    break;
  }
 }
 else if(modoActual==(byte)MODO_GENERADOR)
 {
  // Veo de hacer un menú GENERADOR al apretar un pulsador
  signed int g=genFrec;
  switch(menu())
  {
   case MENU_GEN_FREC1:
    ++genFrec;
    break;
   case MENU_GEN_FREC1M:
    --genFrec;
    break;
   case MENU_GEN_FREC10:
    genFrec+=10;
    break;
   case MENU_GEN_FREC10M:
    genFrec-=10;
    break;
   case MENU_GEN_FREC100:
    genFrec+=100;
    break;
   case MENU_GEN_FREC100M:
    genFrec-=100;
    break;
   case MENU_GEN_FREC1000:
    genFrec+=1000;
    break;
   case MENU_GEN_FREC1000M:
    genFrec-=1000;
    break;
   case MENU_GEN_FRECEN100:
    genFrec=100;              // 100 Hz
    break;
   case MENU_GEN_FRECEN1000:
    genFrec=1000;             // 1000 Hz
    break;
   case MENU_GEN_FRECEN10000:
    genFrec=10000;             // 10000 Hz
    break;
   case MENU_GEN_FRECEN25000:
    genFrec=25000;             // 25000 Hz
    break;
   case MENU_GEN_PUL1_VOLVER:
   case MENU_GEN_PUL2_VOLVER:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   default:
    break;
  }
  // Por los múltiplos, no genera más allá de 25 kHz.
  // El registro es de 16 bits con signo, genero hasta 25 kHz.
  if(genFrec<1)
  {
   genFrec=1;
  }
  else if(genFrec>25000)
  {
   genFrec=25000;
  }
  if(g!=genFrec) activarGenerador(genFrec);
 }
 else if(modoActual==(byte)MODO_CFG)
 {
  // Modo configurador
  byte v=menu();
  if(v==MENU_CFG_CAL_0)
  {
   calibrarRangoGenerico(RANGO_CAL0);
   modoCFG();
  }
  else if(v>=MENU_CFG_CAL_X && v<=MENU_CFG_CAL_Z)
  {
   v-=MENU_CFG_CAL_X;
   calibrarRangoGenerico(RANGO_CALX+v);
   modoCFG();
  }
  #ifdef SIN_CRISTAL
  else if(v==MENU_CFG_FREC50)
  {
   calibrarFrecuencia();
   d3000();
   modoCFG();
  }
  #endif
  else if(v>=MENU_CFG_CAL_SX && v<=MENU_CFG_CAL_SS)
  {
   v-=MENU_CFG_CAL_SX;
   byte adcro=leerADCestable();
   // Solo verifico que no se pasen de valores prohibidos
   confirmarGuardar(eeprom_read_byte(&ee_r[v]),adcro,adcro>SEL_MIN && adcro<SEL_MAX,&ee_r[v]);
   modoCFG();
  }
  else if(v==MENU_CFG_ACERCADE)
  {
   acercade();
   modoCFG();
  }
  else if(v==MENU_CFG_PUL1_VOLVER || v==MENU_CFG_PUL2_VOLVER)
  {
   modoCFG();
  }
 }
 else if(modoActual==(byte)MODO_FREC)
 {
  // Modo frecuencímetro
  // Entra y sale cada un segundo
  // Al terminar de medir, busca el menú
  // Con cristal de 20 MHz, la ventana de medición varía lige-
  // ramente en pos de precisión y no de tiempos.
  pantalla_cursor(0,4);
  pantalla_enteros(medirFrecuencia(),7);
  ihz();
  #if F_CPU==20000000UL
  limite_cuentas=252;     // Es múltiplo de 4 para precisión.
  #else
  limite_cuentas=250;
  #endif
 }
 else if(modoActual==(byte)MODO_SENSOR)
 {
  // Modo Sensor
  pantalla_cursor(0,0);

  #ifdef CON_DS18B20
  // Leo el sensor
  if(band&BAND_SENSOR)
  {
  #endif
   byte data[5]={0,0,0,0,0};
   sensorEn0();
   delai(18);
   sensorEn1();
   __builtin_avr_delay_cycles((40*F_CPU)/1000000UL);

   // Respuesta sensor
   if(PIN_SENSOR & (1<<PATA_SENSOR))  // Debería ir a 0
   {
    busco();
    return;        // Va a loop
   }

   unsigned int m=60000;
   while(!(PIN_SENSOR & (1<<PATA_SENSOR)))     // Espera 1
   {
    if(--m==0) return;             // Va a loop
   }
   m=60000;
   while(PIN_SENSOR & (1<<PATA_SENSOR))        // Espera 0
   {
    if(--m==0) return;             // Va a loop
   }

   // Lectura 40 bits
   for(byte i=0;i<40;i++)
   {
    // Debería haber un fin de tiempo por si el sensor
    // se queda trabado. Pero por simplicidad, nolo hago.
    while(!(PIN_SENSOR & (1<<PATA_SENSOR)));     // espera 1
    __builtin_avr_delay_cycles((30*F_CPU)/1000000UL);
    if(PIN_SENSOR & (1<<PATA_SENSOR)) data[i/8]|=(1<<(7-(i%8)));
    while(PIN_SENSOR & (1<<PATA_SENSOR));        // fin del bit
   }
   if(data[4]!=(data[0]+data[1]+data[2]+data[3]))
   {
    busco();
    return;        // Va a loop
   }
   int16_t t;
   unsigned int h;

   // Veo cual DHT es
   if(data[0]<=3 && (data[2]&0x7F)<=3)
   {
    // DHT22
    h=((unsigned int)data[0]<<8) | data[1];
    t=((unsigned int)data[2]<<8) | data[3];
    if(t & 0x8000)
    {
     t&=0x7FFF;
     t=-t;
    }
   }
   else
   {
    // DHT11
    h=(unsigned int)data[0]*10;
    t=(int16_t)data[2]*10;
   }
   pantalla_print(FPSTR(TE));
   pantalla_print(t>=0?' ':'-');
   pantalla_decimal(t>0?t:-t,3);
   pantalla_print(FPSTR(CH));
   pantalla_decimal(h,4);
   pantalla_print(FPSTR(POR));

   #if defined(__AVR_ATmega328P__)

   Serial.print(FPSTR(TE));
   Serial.print(t>=0?' ':'-');
   SerialPrintDecimal(t>=0?t:-t);
   Serial.print(FPSTR(CH));
   SerialPrintDecimal(h);
   Serial.print(FPSTR(POR));

   #endif
  #ifdef CON_DS18B20
  }
  else
  {
   int16_t t10=leeDS18B20();
   if(t10!=32767)
   {
    bool neg=false;
    if(t10<0)
    {
     t10=-t10;
     neg=true;
    }

    // En el DS18B20 la temperatura viene en pasos de 1/16 °C.
    t10=(t10*10)>>4;
    pantalla_print(FPSTR(TE));
    pantalla_print(neg?'-':' ');
    pantalla_decimal(t10,3);
    pantalla_println(FPSTR(CE));

    #if defined(__AVR_ATmega328P__)

    Serial.print(FPSTR(TE));
    Serial.print(neg?'-':' ');
    SerialPrintDecimal(t10);
    Serial.println(FPSTR(CE));

    #endif
   }
   else
   {
    busco();
   }
  }
  #endif
  d3000();
 }
 else if(modoActual==(byte)MODO_ESPERA)
 {
  // Se podría poner una advertencia, pero ocupa código.  
 }
 else if(modoActual==(byte)MODO_NOVALIDO)
 {
  // Se podría poner una advertencia, pero ocupa código.  
 }
}
// Fin
