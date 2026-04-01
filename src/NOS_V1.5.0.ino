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

// Nano Osciloscopio. Para usar con ATTINY85 o con Arduino,
// Arduino Nano o Arduino Pro Mini

#define VERSION  "V1.5.0"   // Hasta 9 de largo
#define FECHA    "01/04/26"
#define NOMBRE   "NOS"
#define AUTOR    "alecelular"

// Cantidad de pulsadores. Si se compila con cristal, los
// pulsadores 2 y 3 están en la pata de reset, si no en PB3/4
#define PULSADORES 3

// Modelo de pantalla:
// OLED 128x64 = 8
// OLED 128x56 = 7
// OLED 128x48 = 6
// OLED 128x40 = 5
// OLED 128x32 = 4
#define OLED 8

//| -------------------------------------|
//| En ATTINY85                          |
//|--------------------|------|----------|
//| Función            | Pata |   Señal  |
//|--------------------|------|----------|
//| RESET / PulRes 2/3 |   1  | Reset    |
//| Cristal A / PulD 3 |   2  | PB3/ADC3 |entrada reloj externo
//| Cristal B / PulD 2 |   3  | PB4/ADC2 |
//| MASA               |   4  |   0 V    |
//| SDA                |   5  | PB0/AIN0 |
//| SCL                |   6  | PB1/AIN1 |
//| Ent/SAL            |   7  | PB2/ADC1 |
//| Alimentación       |   8  |   VCC    |
//|--------------------| -----|----------|
// ATtiny85 a 16 MHz PLL interno o cristal externo de 16 MHz, y
// por lo tanto, la escala 1 será de 5 µs. Y a 8 MHz la escala
// 1 será de 10 µs. OLED SSD1306 128x64 I2C (No el SSH1106)
// Compilar usando ATTinyCore 1.4.1 / 1.5.2 sin millis()
// En preferencias: http://drazzy.com/package_drazzy.com_index.json
//| ---------------------------------- |
//| En Arduino Nano o Arduino Pro Mini |
//|---------------|---------|----------|
//| Función       |  Pata   |  Señal   |
//|---------------|---------|----------|
//| ADC entrada   |   A0    | PC0      |
//| Generador     |   D8    | PB0      |
//| Pulsador      |   D2    | PD2/INT0 |
//| SDA (bitbang) |   A4    | PC4      |
//| SCL (bitbang) |   A5    | PC5      |
//|---------------|---------|----------|
//| Extras en pro mini                 |
//|---------------|---------|----------|
//| DTR           | Reset   |  Reset   |
//| RXI           | D0/RXD  |  PD0     |
//| TXO           | D1/TXD  |  PD1     |
//|---------------|---------|----------|
//| PD2  PUL1 -   | D2/INT0 |  PD2     | Menos
//| PD3  PUL2 CEN | D3/INT1 |  PD3     | Central
//| PD4  PUL3 +   |D4/XCK/T0|  PD4     | Más
//| PD5           |  D5/T1  |  PD5     |
//| PD6           | D6/AIN0 |  PD6     |
//| PD7           | D7/AIN1 |  PD7     |
//| PB0           | D8/ICP  |  PB0 0   |
//| PB1           | D9/OC1A |  PB1 1   |
//| PB2           | SS/OC1B |  PB2 2   |
//| PB3           |MOSI/OC2 |  PB3 3   |
//| PB4           |  MISO   |  PB4 4   |
//| PB5  LED      | PB5/SCK |  PB5 5   |
//| PB6           | PB6(XTAL1/TOSC1)   |
//| PB7           | PB7(XTAL2/TOSC2)   |
//| PC0/PC3       |  A0/A3  |  ADC0/3  |
//| PC4/PC5       |4/5/SDA/L|  ADC4/5  |
//| PC6           | RESET   |  PC6     |
//| ADC6/7        |  A6/7   |  ADC6/7  |
//|---------------|---------|----------|

// Para programar con Arduino as ISP:
//|-----------|-------------|-------------|
//| Señal ISP |  ATtiny85   |  ATmega328P |
//|-----------|-------------|-------------|
//| MOSI      | pin 5 (PB0) | D11  (PB3)  |
//| MISO      | pin 6 (PB1) | D12  (PB4)  |
//| SCK       | pin 7 (PB2) | D13  (PB5)  |
//| RESET     | pin 1 (RES) | RST         |
//| VCC       | pin 8 (VCC) | VCC         |
//| GND       | pin 4 ( 0V) | GND         |
//|-----------|-------------|-------------|

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

// Pantalla:
#define ALTURA_MAX (OLED*8-9)
#define PAGINA_ESTADO OLED-1

#ifndef PULSADORES
#define PULSADORES 1
#endif
#if PULSADORES<1
#undef PULSADORES
#define PULSADORES 1
#endif

// CLOCK_SOURCE==0      // RC
// CLOCK_SOURCE==1      // Cristal externo
// CLOCK_SOURCE==2      // Oscilador externo
// CLOCK_SOURCE==3      // Oscilador externo
// CLOCK_SOURCE==6      // PLL

// Para saber cómo se compila. Determino RC o no RC.
#ifdef CLOCK_SOURCE
 #if CLOCK_SOURCE==0 || CLOCK_SOURCE==6
 #define SIN_CRISTAL
 #endif
#else
 // No se sabe, entonces debo calibrar
 #if defined(__AVR_ATtiny85__)
 #define SIN_CRISTAL
 #endif
#endif
#ifdef SIN_CRISTAL
 #pragma message "Debe calibrar RC"
 #if F_CPU==16000000UL
 #define TIPOV "PLL"
 #else
 #define TIPOV " RC"
 #endif
#else
 #define TIPOV " CR"
#endif
#if F_CPU==16000000UL
#define TIPOF " 16MHz"
#else
#define TIPOF "  8MHz"
#endif

#define ADC_SAT_ALTO 254
#define ADC_SAT_BAJO   3

#define EEPROM_ID 0x55AA

#define RANGO_CAL0 0
#define RANGO_CAL1 10
#define RANGO_CAL3 33
#define RANGO_CAL5 50
#define RANGO_CALC 120

unsigned int EEMEM ee_id;
byte         EEMEM ee_osccal;
byte         EEMEM ee_adc_fs_0;
byte         EEMEM ee_adc_fs_1;
byte         EEMEM ee_adc_fs_3;
byte         EEMEM ee_adc_fs_5;
byte         EEMEM ee_adc_fs_C;
byte         EEMEM ee_rangoActual;
unsigned int EEMEM ee_band;

// Tamaño del guardado de las mediciones a mostrar
// Mínimo es el ancho del oled. Más, sirve para precisión.
// Tiene que ser en 8 bits. Los valores 254 y 255
// se reservan para poder detectar el fin de captura
#define CAPTURAS_TOTAL 254

// Puntos de la pantalla para que en autoescala muestre una
// onda completa.
#define PUNTOS 100        // Puede ser 128 o menos

#define ESCALA_MINIMA    1
#define ESCALA_MAXIMA  816

// Para el pulsador
#define PRESION_PULSADOR_BREVE 250
#define PRESION_PULSADOR       800

const char ModoOsc[]   PROGMEM = "Oscilosco-\npio";
const char ModoGen[]   PROGMEM = "Generador";
const char ModoCfg[]   PROGMEM = "Config";
const char Volver[]    PROGMEM = "<Volver>";

const char AutoEsc[]   PROGMEM = "Autoescala";
const char EscUno[]    PROGMEM = "Escala+1";
const char EscDiez[]   PROGMEM = "Escala+10";
const char EscCien[]   PROGMEM = "Escala+100";
const char EscMax[]    PROGMEM = "Escala MAX";
const char Grilla[]    PROGMEM = "Grilla";
const char Ent3[]      PROGMEM = "Ent 3,3V";
const char Ent5[]      PROGMEM = "Ent 5V";

const char LibreAuto[] PROGMEM = "Libre/Auto";
const char Escuno[]    PROGMEM = "Escala-1";
const char Escdiez[]   PROGMEM = "Escala-10";
const char Esccien[]   PROGMEM = "Escala-100";
const char EscMin[]    PROGMEM = "Escala MIN";
const char LinPun[]    PROGMEM = "Lineas\nPuntos";
const char Ent1[]      PROGMEM = "Ent 1V";
const char Ent12[]     PROGMEM = "Ent 12V";
const char Escalar[]   PROGMEM = "Escalar";

const char Frec1[]     PROGMEM = "Frec +1";
const char Frec10[]    PROGMEM = "Frec +10";
const char Frec100[]   PROGMEM = "Frec +100";
const char Frec1000[]  PROGMEM = "Frec +1000";
const char Frec1M[]    PROGMEM = "Frec -1";
const char Frec10M[]   PROGMEM = "Frec -10";
const char Frec100M[]  PROGMEM = "Frec -100";
const char Frec1000M[] PROGMEM = "Frec -1000";
const char FrecEn50[]  PROGMEM = "Frec\nen 50 Hz";
const char FrecEn100[] PROGMEM = "Frec\nen 100 Hz";
const char FrecEn500[] PROGMEM = "Frec\nen 500 Hz";
const char FrecEn1000[]PROGMEM = "Frec\nen 1 kHz";
const char FrecEn10K[] PROGMEM = "Frec\nen 10 kHz";

const char Calibra0[]  PROGMEM = "CALIBRAR\n0V";
const char Calibra1[]  PROGMEM = "CALIBRAR\n1V";
const char Calibra3[]  PROGMEM = "CALIBRAR\n3,3V";
const char Calibra5[]  PROGMEM = "CALIBRAR\n5V";
const char Calibra12[] PROGMEM = "CALIBRAR\n12V";
const char Girar[]     PROGMEM = "Girar";
const char Cal50Hz[]   PROGMEM = "CALIBRAR\nCON 50 Hz";
const char Cal60Hz[]   PROGMEM = "CALIBRAR\nCON 60 Hz";
const char Acerca[]    PROGMEM = "Acerca de";

#if PULSADORES==1
#pragma message "Versión de un pulsador"
enum Menu_Osciloscopio
{
 MENU_OSC_AUTOESCALA=1,
 MENU_OSC_ESCALA_MAS1,
 MENU_OSC_ESCALA_MENOS1,
 MENU_OSC_ESCALA_MAS10,
 MENU_OSC_ESCALA_MENOS10,
 MENU_OSC_ESCALA_MAS100,
 MENU_OSC_ESCALA_MENOS100,
 MENU_OSC_ESCALA_MAX,
 MENU_OSC_ESCALA_MIN,
 MENU_OSC_ESCALAR,
 MENU_OSC_LIBRE_AUTO,
 MENU_OSC_GRILLA,
 MENU_OSC_LINEAS,
 MENU_OSC_SEL_CAL1,     // Selecciona calibracion guardada
 MENU_OSC_SEL_CAL3,
 MENU_OSC_SEL_CAL5,
 MENU_OSC_SEL_CALC,
 MENU_OSC_GENERADOR,
 MENU_OSC_CONF,
 MENU_OSC_PUL1_VOLVER
};

const char* const menuOsc[] PROGMEM =
{
 AutoEsc, EscUno, Escuno, EscDiez, Escdiez, EscCien, Esccien,
 EscMax, EscMin, Escalar, LibreAuto, Grilla, LinPun,
 Ent1, Ent3, Ent5, Ent12, ModoGen, ModoCfg, Volver
};

enum Menu_Generador
{
 MENU_GEN_FREC1=1,
 MENU_GEN_FREC1M,
 MENU_GEN_FREC10,
 MENU_GEN_FREC10M,
 MENU_GEN_FREC100,
 MENU_GEN_FREC100M,
 MENU_GEN_FREC1000,
 MENU_GEN_FREC1000M,
 MENU_GEN_FRECEN50,
 MENU_GEN_FRECEN100,
 MENU_GEN_FRECEN500,
 MENU_GEN_FRECEN1000,
 MENU_GEN_FRECEN10000,
 MENU_GEN_OSCILOSCOPIO,
 MENU_GEN_CONF,
 MENU_GEN_PUL1_VOLVER
};

const char* const menuGen[] PROGMEM =
{
 Frec1,Frec1M,Frec10,Frec10M,Frec100,
 Frec100M,Frec1000,Frec1000M,
 FrecEn50,FrecEn100,FrecEn500,FrecEn1000,
 FrecEn10K, ModoOsc, ModoCfg,Volver
};

enum Menu_Conf
{
 MENU_CFG_CAL_0=1,
 MENU_CFG_CAL_1,
 MENU_CFG_CAL_3,
 MENU_CFG_CAL_5,
 MENU_CFG_CAL_C,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC50,
 MENU_CFG_FREC60,
 #endif
 MENU_CFG_GIRO,
 MENU_CFG_OSCILOSCOPIO,
 MENU_CFG_GENERADOR,
 MENU_CFG_ACERCADE,
 MENU_CFG_PUL1_VOLVER
};

const char* const menuCfg[] PROGMEM =
{
 Calibra0, Calibra1, Calibra3, Calibra5, Calibra12,
 #ifdef SIN_CRISTAL
 Cal50Hz, Cal60Hz,
 #endif
 Girar, ModoOsc, ModoGen, Acerca, Volver
};

#endif    // Fin PULSADOR 1


#if PULSADORES==2
#pragma message "Versión de dos pulsadores"
enum Menu_Osciloscopio
{
 // Pulsador Más

 MENU_OSC_AUTOESCALA=1,
 MENU_OSC_ESCALA_MAS1,
 MENU_OSC_ESCALA_MAS10,
 MENU_OSC_ESCALA_MAS100,
 MENU_OSC_ESCALA_MAX,
 MENU_OSC_ESCALAR,
 MENU_OSC_GRILLA,
 MENU_OSC_SEL_CAL3,
 MENU_OSC_SEL_CAL5,
 MENU_OSC_GENERADOR,
 MENU_OSC_PUL1_VOLVER,

 // Pulsador Menos

 MENU_OSC_LIBRE_AUTO,
 MENU_OSC_ESCALA_MENOS1,
 MENU_OSC_ESCALA_MENOS10,
 MENU_OSC_ESCALA_MENOS100,
 MENU_OSC_ESCALA_MIN,
 MENU_OSC_LINEAS,
 MENU_OSC_SEL_CAL1,     // Selecciona calibracion guardada
 MENU_OSC_SEL_CALC,
 MENU_OSC_CONF,
 MENU_OSC_PUL2_VOLVER,
};

const char* const menuOsc[] PROGMEM =
{
 AutoEsc, EscUno, EscDiez, EscCien, EscMax, Escalar, Grilla,
 Ent3, Ent5, ModoGen, Volver,
 LibreAuto, Escuno, Escdiez, Esccien, EscMin, LinPun,
 Ent1, Ent12, ModoCfg, Volver
};

enum Menu_Generador
{
 MENU_GEN_FREC1=1,
 MENU_GEN_FREC10,
 MENU_GEN_FREC100,
 MENU_GEN_FREC1000,
 MENU_GEN_FRECEN50,
 MENU_GEN_FRECEN100,
 MENU_GEN_OSCILOSCOPIO,
 MENU_GEN_PUL1_VOLVER,

 MENU_GEN_FREC1M,
 MENU_GEN_FREC10M,
 MENU_GEN_FREC100M,
 MENU_GEN_FREC1000M,
 MENU_GEN_FRECEN500,
 MENU_GEN_FRECEN1000,
 MENU_GEN_FRECEN10000,
 MENU_GEN_CONF,
 MENU_GEN_PUL2_VOLVER,
};

const char* const menuGen[] PROGMEM =
{
 Frec1,Frec10,Frec100,Frec1000,
 FrecEn50,FrecEn100,ModoOsc, Volver,
 Frec1M,Frec10M,Frec100M,Frec1000M,
 FrecEn500,FrecEn1000, FrecEn10K, ModoCfg,Volver
};

enum Menu_Conf
{
 MENU_CFG_CAL_3=1,
 MENU_CFG_CAL_5,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC50,
 #endif
 MENU_CFG_GIRO,
 MENU_CFG_OSCILOSCOPIO,
 MENU_CFG_PUL1_VOLVER,

 MENU_CFG_CAL_0,
 MENU_CFG_CAL_1,        // Ejecuta proceso de calibracion
 MENU_CFG_CAL_C,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC60,
 #endif
 MENU_CFG_GENERADOR,
 MENU_CFG_ACERCADE,
 MENU_CFG_PUL2_VOLVER,
};

const char* const menuCfg[] PROGMEM =
{
 Calibra3, Calibra5,
 #ifdef SIN_CRISTAL
 Cal50Hz,
 #endif
 Girar, ModoOsc, Volver,
 Calibra0, Calibra1, Calibra12,
 #ifdef SIN_CRISTAL
 Cal60Hz,
 #endif
 ModoGen, Acerca, Volver
};

#endif      // Fin PULSADOR 2

#if PULSADORES>2
#pragma message "Versión de tres pulsadores"
enum Menu_Osciloscopio
{
 // Pulsador Más, 1

 MENU_OSC_ESCALA_MAS1=1,
 MENU_OSC_ESCALA_MAS10,
 MENU_OSC_ESCALA_MAS100,
 MENU_OSC_ESCALA_MAX,
 MENU_OSC_PUL1_VOLVER,

 // Pulsador Menos,  2

 MENU_OSC_ESCALA_MENOS1,
 MENU_OSC_ESCALA_MENOS10,
 MENU_OSC_ESCALA_MENOS100,
 MENU_OSC_ESCALA_MIN,
 MENU_OSC_PUL2_VOLVER,

 // Pulsador selección, 3

 MENU_OSC_AUTOESCALA,
 MENU_OSC_ESCALAR,
 MENU_OSC_LIBRE_AUTO,
 MENU_OSC_LINEAS,
 MENU_OSC_GRILLA,
 MENU_OSC_SEL_CAL1,     // Selecciona calibracion guardada
 MENU_OSC_SEL_CAL3,
 MENU_OSC_SEL_CAL5,
 MENU_OSC_SEL_CALC,
 MENU_OSC_GENERADOR,
 MENU_OSC_CONF,
 MENU_OSC_PUL3_VOLVER
};

const char* const menuOsc[] PROGMEM =
{
 EscUno, EscDiez, EscCien, EscMax, Volver,
 Escuno, Escdiez, Esccien, EscMin, Volver,
 AutoEsc, Escalar, LibreAuto, LinPun, Grilla,
 Ent1, Ent3, Ent5, Ent12, ModoGen, ModoCfg, Volver
};

enum Menu_Generador
{
 // Pulsador Más, 1

 MENU_GEN_FREC1=1,
 MENU_GEN_FREC10,
 MENU_GEN_FREC100,
 MENU_GEN_FREC1000,
 MENU_GEN_FRECEN50,
 MENU_GEN_FRECEN100,
 MENU_GEN_FRECEN10000,
 MENU_GEN_PUL1_VOLVER,

 // Pulsador Menos,  2

 MENU_GEN_FREC1M,
 MENU_GEN_FREC10M,
 MENU_GEN_FREC100M,
 MENU_GEN_FREC1000M,
 MENU_GEN_FRECEN500,
 MENU_GEN_FRECEN1000,
 MENU_GEN_PUL2_VOLVER,

 // Pulsador selección, 3

 MENU_GEN_OSCILOSCOPIO,
 MENU_GEN_CONF,
 MENU_GEN_PUL3_VOLVER,
};

const char* const menuGen[] PROGMEM =
{
 Frec1,Frec10, Frec100, Frec1000,
 FrecEn50, FrecEn100,
 FrecEn10K,
 Volver,
 Frec1M, Frec10M, Frec100M, Frec1000M,
 FrecEn500, FrecEn1000, Volver,
 ModoOsc, ModoCfg, Volver
};

enum Menu_Conf
{
 // Pulsador Más, 1

 MENU_CFG_CAL_3=1,
 MENU_CFG_CAL_5,
 MENU_CFG_GIRO,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC50,
 #endif
 MENU_CFG_PUL1_VOLVER,

 // Pulsador Menos,  2

 MENU_CFG_CAL_0,
 MENU_CFG_CAL_1,        // Ejecuta proceso de calibración
 MENU_CFG_CAL_C,
 #ifdef SIN_CRISTAL
 MENU_CFG_FREC60,
 #endif
 MENU_CFG_PUL2_VOLVER,

 // Pulsador selección, 3

 MENU_CFG_OSCILOSCOPIO,
 MENU_CFG_GENERADOR,
 MENU_CFG_ACERCADE,
 MENU_CFG_PUL3_VOLVER,
};

const char* const menuCfg[] PROGMEM =
{
 Calibra3, Calibra5, Girar,
 #ifdef SIN_CRISTAL
 Cal50Hz,
 #endif
 Volver,
 Calibra0, Calibra1, Calibra12,
 #ifdef SIN_CRISTAL
 Cal60Hz,
 #endif
 Volver, ModoOsc,  ModoGen, Acerca, Volver
};

#endif      // Fin PULSADOR 3

volatile byte capturas[CAPTURAS_TOTAL];
volatile byte indice=0;
int escala;  // Puede tomar valores negativos momentáneos
unsigned int tiempoReal_us=10;

byte rangoActual=RANGO_CAL5;

// Bits:
// 0 mostrar grilla
// 1 modoLinea       falso = puntos, verdadero = línea
// 2 modo gatillo    Espera cruce, falso: Barrido libre
// 3 auto escala     Arranca con autoescala
// 4 Giro
// 5 Escalar
#define BAND_MOSTRARGRILLA (1<<0)
#define BAND_MODOLINEA     (1<<1)
#define BAND_MODOGATILLO   (1<<2)
#define BAND_AUTOESCALA    (1<<3)
#define BAND_GIRO          (1<<4)
#define BAND_ESCALAR       (1<<5)
unsigned int band=(BAND_MODOGATILLO | BAND_AUTOESCALA);

// Para leer el pulsador
volatile bool pido=false;
volatile bool aborta=false;
bool capan=false;           // Captura de pantalla

byte adc_fondo_escala=ADC_SAT_ALTO;   // valor por defecto
byte adc_offset=ADC_SAT_BAJO;         // ADC para 0V

// Estados de modo
#define MODO_OSCILOSCOPIO 0     // Debe ser 0
#define MODO_GENERADOR    1
#define MODO_CFG          2

volatile byte modoActual=MODO_OSCILOSCOPIO;

// Variables para el motor del generador
volatile unsigned int contadorFrecuencia=0;
volatile unsigned int recargaFrecuencia=100;  // Divisor
signed int genFrec=100;        // 100 Hz

// Habilita interrupciones del temporizador 1 y las generales.
void habilitarT1(void)
{
 aborta=false;
 #if defined(__AVR_ATtiny85__)
 TIMSK|=(1<<OCIE1A);
 #else
 TIMSK1|=(1<<OCIE1A);
 #endif

 // Y hago una primera conversión del ADC para empezar, solo
 // en modo osciloscopio
 if(modoActual==MODO_OSCILOSCOPIO)
 {
  ADCSRA|=(1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  (void)ADCH;

  // Primer captura
  ADCSRA|=(1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  capturas[0]=ADCH;
  indice=1;
 }
 sei();
}

// Devuelve verdadero si está apagado T1 o por pulsador
bool finCaptura(void)
{
 // Detección de pulsador para abortar
 // Termina cuando leyó todo, o bien se apretó un pulsador
 #if defined(__AVR_ATtiny85__)
 aborta=!(ACSR & (1<<ACO))
 #ifdef SIN_CRISTAL
 #if PULSADORES>1
 | !(PINB & (1<<PB4)) // PB4  Pulsador 2
 #endif
 #if PULSADORES>2
 | !(PINB & (1<<PB3)) // PB3 PUL 3 entrada reloj externo
 #endif
 #endif
 ;
 if(aborta) TIMSK&=~(1<<OCIE1A);
 return !(TIMSK&(1<<OCIE1A));

 #else

 // Pin D2 en ATmega328P es pulsador 1. Está siempre
 aborta=!(PIND & (1<<PD2))
 #if PULSADORES>1
 | !(PIND & (1<<PD3)) // PD4  Pulsador 2
 #endif
 #if PULSADORES>2
 | !(PIND & (1<<PD4)) // PD3 PUL 3
 #endif
 ;
 if(aborta) TIMSK1&=~(1<<OCIE1A); // Registro específico del Nano
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
 *       m = 816     |   2.04 ms    |     1044 ms     |        0.95 Hz
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
*/
void configurarEscala(unsigned int m)
{
 // m representa múltiplos de 10us (m=2 -> 20µs, m=16 -> 160µs)
 unsigned int tiempoDeseado=(unsigned int)m*10;

 // Vale para cualquier procesador a 16 MHz
 #if F_CPU==16000000UL

 #if defined(__AVR_ATtiny85__)
 if(m==1)
 {
  // En 16 MHz, 4 es el borde mínimo,  5 es seguro
  tiempoDeseado=5;
 }
 #else
 // Para los arduinos, un poco más lento
 if(m==1)
 {
  // En 16 MHz, 5 es el borde mínimo, 6 es seguro
  tiempoDeseado=6;
 }
 #endif

 #else

 // 8 MHz
 #if defined(__AVR_ATtiny85__)
 if(m==1)
 {
  // En  8 MHz, 8 borde mínimo, 10 seguro,  12 estable
  tiempoDeseado=10;
 }
 #else
 // Para los arduinos, un poco más lento
 if(m==1)
 {
  // En 8 MHz, 9 borde mínimo, 10 seguro, 12 estable
  tiempoDeseado=10;
 }
 #endif

 #endif

 // Prescaler del ADC
 if(modoActual==MODO_OSCILOSCOPIO)
 {
  // Lógica para ATtiny85 (8MHz o 16MHz PLL)
  // Sirve para Arduino Nano o Pro Mini
  byte presc;
  #if F_CPU==8000000UL
  if(m==1) presc=(1<<ADPS1);                   // /4
  else if(m==2) presc=(1<<ADPS1)|(1<<ADPS0);   // /8
  else if(m==3) presc=(1<<ADPS2);              // /16
  else if(m==4) presc=(1<<ADPS2)|(1<<ADPS0);   // /32
  else presc=(1<<ADPS2)|(1<<ADPS1);            // /64
  #else
  if(m==1) presc=(1<<ADPS1)|(1<<ADPS0);        // /8
  else if(m==2) presc=(1<<ADPS2);              // /16
  else if(m==3) presc=(1<<ADPS2)|(1<<ADPS0);   // /32
  else if(m==4) presc=(1<<ADPS2)|(1<<ADPS1);   // /64
  else presc=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // /128
  #endif

  while(ADCSRA & (1<<ADSC));
  ADCSRA&=~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
  ADCSRA|=presc;
  ADCSRA|=(1<<ADSC); // Conversión de limpieza
  while(ADCSRA & (1<<ADSC));
  (void)ADCH;
 }

 // Temporizador 1
 #if defined(__AVR_ATtiny85__)

 // Para ATtiny85
 TCCR1 &= ~0x0F; // Limpiar prescaler
 byte tic;
 #if F_CPU == 8000000UL
 if(tiempoDeseado<=255)
 {
  TCCR1|=(1<<CS12);
  OCR1C=(byte)(tiempoDeseado-1);
  tic=1;
 } 
 else if(tiempoDeseado<=510)
 {
  TCCR1|=(1<<CS12)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/2)-1);
  tic=2;
 }
 else if(tiempoDeseado<=1020)
 {
  TCCR1|=(1<<CS12)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/4)-1);
  tic=4;
 } 
 else if(tiempoDeseado<=2040)
 {
  TCCR1|=(1<<CS12)|(1<<CS11)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/8)-1);
  tic=8;
 }
 else if(tiempoDeseado<=4080)
 {
  TCCR1|=(1<<CS13);
  OCR1C=(byte)((tiempoDeseado/16)-1);
  tic=16;
 }
 else
 {
  TCCR1|=(1<<CS13)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  tic=32;
 }
 #else
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
 else
 {
  TCCR1|=(1<<CS13)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  tic=32;
 }
 #endif
 OCR1A=OCR1C;
 tiempoReal_us=(unsigned long)(OCR1C+1)*tic;

 #else

 // Para los arduinos
 TCCR1A=0;
 TCCR1B=(1<<WGM12); // Modo CTC
 TCNT1=0;

 // Defino cuántos incrementos del Timer equivalen a 1 µs
 #if F_CPU == 16000000L
  #define TICS_POR_US 2  // 16MHz / 8 = 2MHz (2 tics = 1 µs) 
 #else
  #define TICS_POR_US 1  // 8MHz / 8 = 1MHz (1 tic = 1 µs)
 #endif

 // Aplico la lógica de pasos
 unsigned long int tiempoAjustado;
 if(tiempoDeseado<=10)
 {
  // Pasos de 1 en 1 µs
  tiempoAjustado=tiempoDeseado; 
 }
 else
 {
  // Pasos de 10 en 10 µs
  // (Redondeo al múltiplo de 10 más cercano)
  tiempoAjustado=((tiempoDeseado+5)/10)*10;
 }

 // Configuro el Timer 1
 TCCR1B&=~((1<<CS12) | (1<<CS11) | (1<<CS10)); // Limpiar prescaler
 TCCR1B|=(1<<CS11);                          // Configurar /8
 OCR1A=(tiempoAjustado*TICS_POR_US)-1;
 tiempoReal_us=(unsigned long)(OCR1A + 1)/TICS_POR_US;

 #endif
}

ISR(TIMER1_COMPA_vect)
{
 // El modo osciloscopio es 0. Es más rápido que la comparación
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
   TIMSK&=~(1<<OCIE1A);
   #else
   TIMSK1&=~(1<<OCIE1A); // Registro específico del Nano
   #endif
  }
  return;
 }

 // MODO GENERADOR
 pido=false;
 if(--contadorFrecuencia==0)
 {
  #if defined(__AVR_ATtiny85__)
  PINB=(1<<PB2); 
  #else
  PINB=(1<<PB0); // D8 en arduino es Port B bit 0
  #endif
  contadorFrecuencia=recargaFrecuencia;
 }
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
 configurarEscala(cap);

 #if defined(__AVR_ATtiny85__)
 // Alta impedancia, para leer el comparador interno.
 // Así leo el pulsador y poder abortar la lectura
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));
 // Habilito comparador para detectar el pulsador
 ACSR&=~(1<<ACD);
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

 int h=(vMax-vMin)>>2;   // 1/4 amplitud
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
   if(m<nivel_bajo)
    armado=true;
  }
  else
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
 }

 if(periodos>0)
 {
  // Calculo sobre el tiempo TOTAL transcurrido entre el primer y último pulso
  unsigned long int deltaTotal_us=(unsigned long)(ultimo-p1)*tiempoReal_us;
  if(deltaTotal_us>0)
  {
   // Multiplicamos por periodos ANTES de dividir para ganar resolución
   hz_x10=(10000000UL*periodos)/deltaTotal_us;
  }
 }

 // Si no hubo onda, entonces mostraré desde posición 0
 if(p1==255) p1=0;    // P1 en 255 indica que no hubo nada

 // Si hubo algo, pero no en la zona visible, lo indico con 255
 if(p1>CAPTURAS_TOTAL-128) p1=255;   // Hubo algo, pero no será mostrado

 // Ya tengo la frecuencia
 return hz_x10;
}

// Imprime un número unsigned int con espacios delante
void printEnteros(unsigned long int valor, byte ancho)
{
 unsigned long int temp=valor;
 byte digitos=(valor==0)?1:0;
  
 // Cuento cuántos dígitos tiene el número
 while(temp>0)
 {
  temp/=10;
  digitos++;
 }
 // Imprimo los espacios necesarios
 for(byte i=0;i<(ancho-digitos);i++)
 {
  pantalla_print(' '); 
 }
 // Imprimo el número
 pantalla_print(valor);
}

// Imprime una cifra con un decimal y con espacios delante
void printDecimal(unsigned long int val, byte enteros)
{
 printEnteros(val/10,enteros);
 pantalla_print(',');
 pantalla_print(val%10);
}

void actualizar(int comienzo, unsigned long int hz_x10, unsigned int ms)
{
 // Calculo si es posible hacer lupa.
 // Si es así, multiplico por 2 o 3 lo que se ve
 // Recorro todos los valores. Todos tienen que ser menor a
 // adc_fondo_escala/2 o 3 para que puedan expandirse
 byte lupa=3;
 for(byte i=0;i<128;i++)
 {
  if(capturas[comienzo+i]>=adc_fondo_escala/3)
  {
   lupa=2;
   if(capturas[comienzo+i]>=adc_fondo_escala/2)
   {
    lupa=1;
    break;
   }
  }
 }

 static byte yAnterior=0;
 static bool primerPunto=true;
 byte vMax=0;
 for(byte pagina=0;pagina<OLED-1;pagina++)
 { 
  pantalla_cursor(0,pagina);
  pantalla_comienzoDatos();

  primerPunto=true;

  for(byte x=0;x<128;x++)
  {
   // Calculo vpp. La onda siempre parte de cero
   // Veo VPP a partir de lo que se ve
   // Ponerlo acá ahorra código aunque se procese varias veces
   if(capturas[x+comienzo]>vMax) vMax=capturas[x+comienzo];

   byte byteSalida=0x00;

   // Veo si se produce expansión de la escala
   byte e=((band&BAND_ESCALAR)?x/4:x);

   // Aumento visualización, si corresponde
   byte dato=capturas[comienzo+e]*lupa;

   if(dato<ADC_SAT_ALTO)
   {
    byte d;
    if(dato>adc_offset) d=dato-adc_offset; else d=0;
    if(d>adc_fondo_escala) d=adc_fondo_escala;
    unsigned int factorY=((unsigned int)(ALTURA_MAX+1)<<8)/adc_fondo_escala;
    byte y_pixel=((unsigned int)d*factorY)>>8;
    if(y_pixel>ALTURA_MAX) y_pixel=ALTURA_MAX;
    byte filaInvertida=ALTURA_MAX-y_pixel;
    if(band&BAND_MODOLINEA)
    {
     if(!primerPunto)
     {
      byte yMin=(filaInvertida<yAnterior)?filaInvertida:yAnterior;
      byte yMax=(filaInvertida>yAnterior)?filaInvertida:yAnterior;
      for(byte y=yMin;y<=yMax;y++)
      {
       if((y/8)==pagina) byteSalida|=(1<<(y%8));
      }
     }
     yAnterior=filaInvertida;
     primerPunto=false;
    }
    else
    {
     if((filaInvertida/8)==pagina) byteSalida|=(1<<(filaInvertida%8));
    }
   }
   // GRILLA
   // Bit 0 mostrar grilla
   if(band&BAND_MOSTRARGRILLA)
   {
    if((x%20)==0)     // Lineas verticales
    {
     byteSalida|=0x88;
    }
    byte yBase=pagina*8;
    for(byte b=0;b<8;b++)
    {
     int yReal=yBase+b;
     int yRel=ALTURA_MAX-yReal;
     if((yRel%20)==0)     // Lineas horizontales
     {
      if((x%4)==0) byteSalida|=(1<<b);
     }
    }

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

 // Pone si está magnificado todo
 pantalla_print(lupa);

 // Que tensión seleccionada usa
 if(rangoActual==120)
 {
  pantalla_print('C');
 }
 else
 {
  pantalla_print((byte)rangoActual/10);
 }

 // Autoescala o manual
 pantalla_print((band&BAND_AUTOESCALA)?'A':'M');

 // Si está escalada o normal
 if(band&BAND_ESCALAR) pantalla_print('4'); else pantalla_print('N');

 pantalla_separacion(3);

 // Verdadero: Espera cruce, falso: Barrido libre
 pantalla_print((band&BAND_MODOGATILLO)?'G':'L');

 pantalla_separacion(0);

 printEnteros(ms,4);
 pantalla_print('u');
 pantalla_separacion(0);
 pantalla_print(' ');

 // Muestra la frecuencia. No muestra debajo de 0,5 Hz
 pantalla_separacion(1);
 if(hz_x10<=5)
 {
  pantalla_print(F("-----"));  // No hay
 }
 else
 {
  // Redondeo a 0,5 Hz
  printEnteros((hz_x10+5)/10,5);
 }
 pantalla_separacion(0);
 pantalla_print('H');
 pantalla_print(' ');

 if(vMax>=ADC_SAT_ALTO)
 {
  pantalla_print(F("  SAT"));
 }
 else
 {
  // rangoActual está en decivoltios
  // adc_fondo_escala tiene la calibración del ADC
  // vMax tiene el ADC máximo para esa medición
  printDecimal(((((unsigned int)vMax*rangoActual)/adc_fondo_escala)),2);
  pantalla_print('V');
 }
}

// Lectura del pulsador. Verdadero si es presionado. Solo es
// leído cuando termina de medir o cuando es frecuencia o cfg.
// Da 0 si no se apretó pulsador, 1 si se apretó prolongado y
// se mantiene. 255 es presión breve del pulsador 1
// 254 si es pulso breve del pulsador 2, 253 del pulsador 3.
// Si es 0 es falso, no hubo pulsación de cualquier tipo
byte leerPulsador(void)
{
 // Pido leer pulsadores cuando haya atendido una interrupción
 // cuando es generador porque va a cambiar el port B.
 // En modo osciloscopio, el pulsador es leído cuando termina
 // de medir o aborta.
 // No puedo usar a fincaptura(), ya que el port B espera estar
 // alterado antes de ser llamado. Aquí, simplemente si está
 // el modo generador, espero a pido para sincronizar con la
 // interrupción, para hacer todo
 if(modoActual==MODO_GENERADOR)
 {
  // Debo asegurarme que haya interrupciones
  // para que funcione pido.
  pido=true;
  while(pido);   // Espero a que atienda la interrupción
 }

 byte resultado=0;
 #if defined(__AVR_ATtiny85__) && !defined(SIN_CRISTAL) && PULSADORES!=1
 // Leo un pulsador en reset. Y funciona. Hasta 3 pulsadores.
 // Son leídos cuando termina de medir o cuando es frecuencia.
 // Guardo estado actual de ADMUX y configurar Referencia a
 // VCC (bits REFS en 0). Mantengo los bits de canal para
 // ADC0 (PB5) que son 0000 en los bits MUX
 byte Admux=ADMUX;
 ADMUX=(0<<REFS1)|(0<<REFS0)|(1<<ADLAR); // VCC ref y Justificación Izquierda (8 bits)

 // Inicio una conversión de descarte. El micro necesita purgar
 // la carga de la referencia anterior (1.1V)
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC)); // Esperar a que termine

 // Inicio la conversión real
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC));

 // Leo solo el registro alto para obtener 8 bits rápidos
 resultado=ADCH;

 // Doy una segunda lectura, por la inestabilidad de la
 // pulsación analógica.
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC));

 // Leo solo el registro alto para obtener 8 bits rápidos
 byte resultado2=ADCH;

 // Restauro la referencia a 1.1V
 // (REFS1=1, REFS0=0 para ATtiny85)
 ADMUX=Admux;

 // Pulsador 2 y 3. En todos los casos, se continúa con el 1
 // Los dos a la vez, provoca reset
 if(resultado<130 && resultado2<130)      resultado=2;
 else if(resultado<200 && resultado2<200) resultado=3;
 else resultado=0;
 #endif

 // Si son pulsadores directos cuando no hay cristal
 #if defined(__AVR_ATtiny85__)
  #if defined(SIN_CRISTAL)
  // PB4  Pulsador 2
  #if PULSADORES>1
  if(!(PINB & (1<<PB4))) resultado=2;
  #endif
  #if PULSADORES>2
  // PB3 PUL 3 entrada reloj externo
  if(!(PINB & (1<<PB3))) resultado=3;
  #endif
  #endif
 #else
  #if PULSADORES>1
  if(!(PIND & (1<<PD3))) resultado=2;
  #endif
  #if PULSADORES>2
  if(!(PIND & (1<<PD4))) resultado=3;
  #endif
 #endif

 // Busco el pulsador diferencial en ATtiny85 que es el pulsa-
 // dor 1. Con este y el pulsador 2, da pulsador 5
 // Con este y el pulsador 2, da pulsaador 5
 // Y junto con el pulsador 3 da el pulsador 4
 // Si se gira, los pulsadores 1 y 3 se intercambian
 // De paso da tiempo a estabilizar la referencia de 1,1V
 // Guardo estado del PORT B para no molestar al bus i2C
 // ya que lo uso para leer el pulsador
 #if defined(__AVR_ATtiny85__)
 byte ddr=DDRB;
 byte port=PORTB;

 // Alta impedancia, para leer el comparador interno.
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));

 ACSR&=~(1<<ACD);         // Habilito comparador

 // Busco pulsación
 bool pul=!(ACSR & (1<<ACO));

 #else

 bool pul=!(PIND & (1<<PD2));

 #endif

 // Busco un pulso breve, sólo si hubo un pulso inicial
 if(pul
 #if PULSADORES>1
 || resultado!=0        // Pulsode los otros
 #endif
 ) delay(PRESION_PULSADOR_BREVE);

 #if defined(__AVR_ATtiny85__)
 bool pulb=!(ACSR & (1<<ACO));   // Lo leo igual

 // Devuelvo estado del port
 DDRB=ddr;
 PORTB=port;

 #else
 
 bool pulb=!(PIND & (1<<PD2));

 #endif

 // Busco pulso breve de los otros pulsadores
 #if defined(__AVR_ATtiny85__)
  #if PULSADORES>1
  bool pulb2=!(PINB & (1<<PB4));   // Pulsador 2
  #endif
  #if PULSADORES>2
  bool pulb3=!(PINB & (1<<PB3));   // Pulsador 3
  #endif
 #else
  #if PULSADORES>1
  bool pulb2=!(PIND & (1<<PD3));   // Pulsador 2
  #endif
  #if PULSADORES>2
  bool pulb3=!(PIND & (1<<PD4));   // Pulsador 3
  #endif
 #endif

 // Vale si hubo un primer pulso. Si no lo hay, el segundo
 // vale como si fuera el primero
 if(pul || pulb
 #if PULSADORES>1
 || resultado!=0 || pulb2
 #endif
 #if PULSADORES>2
 || pulb3
 #endif
 )
 {
  // Si no hay segundo pulso, fue breve.
  // No es útil si son pulsadores 2 y 3 en reset
  if(pul && !pulb) return 255;           // P1 Pulso breve 255
  #if !defined(__AVR_ATtiny85__) || defined(SIN_CRISTAL)
  #if PULSADORES>1
  if(resultado==2 && !pulb2) return 254; // P2 Pulso breve 254
  #endif
  #if PULSADORES>2
  if(resultado==3 && !pulb3) return 253; // P3 Pulso breve 253
  #endif
  #endif

  #if PULSADORES!=1
  // Simulo pulsador 4 y 5, sin uso aún
  if(pul)
  {
   if(resultado==3) resultado=4;      // Pulsador 4
   else if(resultado==2) resultado=5; // Pulsador 5
   else resultado=1;         // Pulsador 1 Menos
  }
  return resultado;
  #else
  return 1;                  // Pulsador 1 Menos
  #endif
 }
 return 0;
}

// Devuelve la opción elegida.
byte menu(void)
{
 // Pulsador 1 actua como MENOS (breve 255)
 // Pulsador 2 es el central    (breve 254)
 // Pulsador 3 actúa como MÁS   (breve 253)
 // Pulsador 4, no usado
 // Pulsador 5, no usado
 // Pulsador 255, captura
 // Si se invierte el display, se invierte el significado de
 // más y menos, pero los códigos son los mismos.

 // Veo si hubo una pulsación para hacer menú. Si no, salgo
 byte cual=leerPulsador();

 // Las pulsaciones breves no hacen menú
 // Tampoco si es 0 o 4 o 5
 if(cual==0 || cual>=4) return cual;

 // Sigue presionado. Hago menú.
 // Pero ya hizo a PRESION_PULSADOR_BREVE. Debo restarlo de
 // la primera apretada

 const char* const* ptrMenu;
 byte inicio=0,fin;

 // Selección de tabla
 #if PULSADORES>1
 bool modOsc=false;
 bool modGen=false;
 #endif
 // Selección de tabla
 // (Mantengo esto simple para el compilador)
 if(modoActual==MODO_OSCILOSCOPIO)
 {
  ptrMenu=menuOsc;
  fin=(byte)MENU_OSC_PUL1_VOLVER;
  #if PULSADORES>1
  modOsc=true;
  #endif
 }
 else if(modoActual==MODO_GENERADOR)
 {
  ptrMenu=menuGen;
  fin=(byte)MENU_GEN_PUL1_VOLVER;
  #if PULSADORES>1
  modGen=true;
  #endif
 }
 else
 {
  ptrMenu=menuCfg;
  fin=(byte)MENU_CFG_PUL1_VOLVER;
 }

 // Ajuste de ventana según pulsador (Solo si hay más de uno)
 #if PULSADORES>1
 byte pMas=(band&BAND_GIRO)?1:3;
 if(cual!=pMas)       // pMenos implícito
 {
  // El inicio del bloque 2 es el fin del bloque 1
  inicio=fin;
  fin=(modOsc)?(byte)MENU_OSC_PUL2_VOLVER:(modGen)?(byte)MENU_GEN_PUL2_VOLVER:(byte)MENU_CFG_PUL2_VOLVER;
  #if PULSADORES>2
  // Excepción a cual!=pMas
  if(cual==2)
  {
   // Bloque 3
   inicio=fin;
   fin=(modOsc)?(byte)MENU_OSC_PUL3_VOLVER:(modGen)?(byte)MENU_GEN_PUL3_VOLVER:(byte)MENU_CFG_PUL3_VOLVER;
  }
  #endif
 }
 #endif

 // Veo si se mantiene apretado
 byte cant=fin-inicio;
 byte i=0;
 while(leerPulsador())
 {
  limpiaX2();
  if(++i>cant) i=1;
  // Acceso directo por puntero offset
  pantalla_println((__FlashStringHelper*)pgm_read_word(ptrMenu+inicio+i-1));

  // Le resto la pulsación breve para que todo tarde igual
  delay(PRESION_PULSADOR-(i==1?PRESION_PULSADOR_BREVE:0));
 }

 // Devolverá también la última opción [Volver]
 return inicio+i; 
}

typedef struct {
  byte escala;
  unsigned int recarga;
  unsigned long int frecuencia_mHz;
  bool perfecta=false;
} ResultadoGen; // ResultadoGen es un "Alias" oficial de tipo

// Prototipo necesario
ResultadoGen calculaGenerador(unsigned long int fDeseada_Hz);

// Función del prototipo
ResultadoGen calculaGenerador(unsigned long int fDeseada_Hz)
{
 ResultadoGen mejor;
 unsigned long int mejorError=0xFFFFFFFF;

 // Empiezo en la escala 2 ya que sirve para llegar a 25 kHz
 for(int esc=ESCALA_MINIMA+1;esc<ESCALA_MAXIMA;esc++)
 {
  configurarEscala(esc);    // Calcula tiempoReal_us;

  unsigned long int den=2UL*tiempoReal_us*fDeseada_Hz;

  // BUSCO SOLUCIÓN EXACTA
  if(1000000UL%den==0)
  {
   mejor.escala=esc;
   mejor.recarga=1000000UL/den;
   mejor.frecuencia_mHz=fDeseada_Hz*1000;
   mejor.perfecta=true;
   return mejor;                 // Es perfecta
  }

  // CALCULO APROXIMACIÓN
  unsigned long int rec=1000000UL/den;
  if(rec==0) rec=1;
  for(char k=-2;k<=2;k++)
  {
   signed long int r=rec+k;
   if(r<1) continue;

   unsigned long int f_mHz=1000000000UL/(2UL*tiempoReal_us*r);

   unsigned long int error=(f_mHz>fDeseada_Hz*1000)?(f_mHz-fDeseada_Hz*1000):(fDeseada_Hz*1000-f_mHz);

   if(error<mejorError)
   {
    mejorError=error;
    mejor.escala=esc;
    mejor.recarga=r;
    mejor.frecuencia_mHz=f_mHz;
    mejor.perfecta=false;       // Es aproximada
   }
  }
 }
 return mejor;
}

// Con cristal 8 MHz o 16 MHz, solo las frecuencias bajas cuya
// descomposición en factores primos 2 y 5, pueden ser exactas.
// En modo generador, las escalas empiezan en 10 µs y van de a
// un µs
void activarGenerador(unsigned int fDeseada_Hz)
{
 genFrec=fDeseada_Hz;     // Lo pongo en la global
 cli();

 // Preparo Pata 7 (PB2) como salida.
 // En arduino, PB0 ya es salida
 #if defined(__AVR_ATtiny85__)
 DDRB|=(1<<PB2);
 #endif

 modoActual=MODO_GENERADOR;

 ResultadoGen res=calculaGenerador(fDeseada_Hz);

 limpiaX2();

 // Pongo la frecuencia que pretendo
 pantalla_println(F("Deseada"));
 printEnteros(fDeseada_Hz,5);
 pantalla_println(F(",0 Hz"));

 // Obtenida (en mili Hz)
 pantalla_print(F("Obtenida "));
 pantalla_println(res.perfecta?"=":"#");
 printDecimal(res.frecuencia_mHz/100,5);
 ihz();

 // Pongo lo calculado
 recargaFrecuencia=res.recarga;
 configurarEscala(res.escala);

 habilitarT1();
}

void restaurarOsc(void)
{
 cli();
 modoActual=MODO_OSCILOSCOPIO;
 // Para ATTINY85. En arduino la pata de salida
 // siempre es salida
 #if defined(__AVR_ATtiny85__)
 DDRB&=~(1<<PB2);          // PB2 vuelve a ser entrada
 PORTB&=~(1<<PB2);         // Sin pullup
 #endif
 configurarEscala(escala); // Restaura la escala que tenía el osciloscopio
 limpia();
 sei();
}

void limpiaX2(void)
{
 pantalla_limpia();
 #if OLED<8
 pantalla_modoTipo(1);
 #else
 pantalla_modoTipo(2);
 #endif
 pantalla_separacion(1);
}

void limpia(void)
{
 pantalla_limpia();
 pantalla_modoTipo(0);
 pantalla_separacion(1);
}

void cargarFactorPorRango(void)
{
 byte* p_fs;

 if(rangoActual==RANGO_CAL3)
 { 
  p_fs=&ee_adc_fs_3;
 }
 else if(rangoActual==RANGO_CAL5)
 { 
  p_fs=&ee_adc_fs_5;
 }
 else if(rangoActual==RANGO_CALC)
 { 
  p_fs=&ee_adc_fs_C;
 } 
 else
 { 
  p_fs=&ee_adc_fs_1;
 }

 adc_fondo_escala=eeprom_read_byte(p_fs);
 
 // Guardo la selección
 eeprom_update_byte(&ee_rangoActual,rangoActual);
}

byte pulsar(void)
{
 byte sale;
 pantalla_print(F("\nPULSAR!"));
 while(leerPulsador());  // Espero a soltar del menú
 delay(250);
 do                      // Espero a apretar
 {
  sale=leerPulsador();
 } while(!sale);
 while(leerPulsador());  // Espero a soltar
 return sale;
}

void icali(void)
{
 pantalla_print(F("CALIBRADO"));
}
//7286
void calibrarRangoGenerico(byte rangoActivo, byte vref_mV)
{
 if(pulsar()>250)
 {
  limpiaX2();
  pantalla_print(F("Aborta!"));
  return;
 }
 limpiaX2();
 unsigned int suma=0;

 // Promediar 64 muestras
 for(byte i=0;i<64;i++)
 {
  ADCSRA|=(1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  suma+=ADCH;
 }

 if(rangoActivo==0)
 {
  pantalla_println(F("0V"));

  // Promedio
  adc_offset=suma/64;

  if(adc_offset>ADC_SAT_BAJO*3)         // Mal medido
  {
   adc_offset=ADC_SAT_BAJO;
   pantalla_println("NO");
  }
  else
  {
   // Pongo lo evaluado
   pantalla_print("ADC:");
   pantalla_println(adc_offset);
  }
  icali();
  eeprom_update_byte(&ee_adc_fs_0,adc_offset);
 }
 else
 {
  // A lo calibrado le quito el offset
  int tmp=(suma/64)-adc_offset;
  if(tmp<0) tmp=0;
  byte adc_cal=tmp;

  pantalla_print("ADC:");

  // Seguridad mínima. La cantidad de puntos del oled.
  // Y supongo un offset de 3
  if(adc_cal>=55+3)
  {
   // La indeterminación en el redondeo para calcular la
   // tensión medida, es de una cuenta del adc. Así que
   // quito uno, para dar precisión a la medición al calibrar.
   adc_cal--;

   // Guardar en EEPROM según rango
   switch(rangoActivo)
   {
    case RANGO_CAL1:
     eeprom_update_byte(&ee_adc_fs_1,adc_cal);
     break;
    case RANGO_CAL3:
     eeprom_update_byte(&ee_adc_fs_3,adc_cal);
     break;
    case RANGO_CAL5:
     eeprom_update_byte(&ee_adc_fs_5,adc_cal);
     break;
    default:
     eeprom_update_byte(&ee_adc_fs_C,adc_cal);
     break;
   }

   //  Guardo y establezco
   rangoActual=rangoActivo;
   eeprom_update_byte(&ee_rangoActual,rangoActual);

   // Pongo lo evaluado
   pantalla_println(adc_cal);
  }
  else
  {
   pantalla_println(adc_cal);
   pantalla_println("NO");
  }
  icali();
  pantalla_print(F("\nA:"));
  printDecimal(vref_mV,3);
  pantalla_print('V');
 }
}

void ihz(void)
{
 pantalla_println(F(" Hz"));
}

#ifdef SIN_CRISTAL

void calibrarFrecuencia(unsigned int frecRef_x10)
{
 // Debe estar en modo osciloscopio para poder medir
 restaurarOsc();

 limpiaX2();
 pantalla_println(F("CAL:"));
 pantalla_print(frecRef_x10/10);
 ihz();
 if(pulsar()>250)
 {
  limpiaX2();
  pantalla_print(F("Aborta"));
  return;
 }

 // Borro todo
 limpiaX2();

 byte minimo_error=255;
 byte maximo_error=0;
 byte osqui=OSCCAL;       // Guardo el que estaba
 unsigned int hz_x10=0;
 for(byte osccal=2;osccal<254;osccal++)
 {
  OSCCAL=osccal;
  delay(4);               // Estabiliza RC

  unsigned int suma=0;
  // Busco que dé una frecuencia valida
  for(byte i=0;i<8;i++)
  {
   byte Gato;
   // 300 ms por punto. Múltiplo de 50 y 60 Hz
   unsigned int r=analiza(30,Gato);
   if(aborta) break; // Si aborta,no guardo OSCCAL
   suma+=(unsigned int)r;
  }

  if(aborta) break; // Si aborta,no guardo OSCCAL

  // Analizo si da una frecuencia
  if(suma)
  {
   hz_x10=suma>>3;   // promedio /8

   // Voy mostrando la calibración
   pantalla_cursor(0,0);
   pantalla_print(F("CAL:"));
   pantalla_println(osccal);
   printDecimal(hz_x10,0);
   ihz();
   pantalla_print(' ');

   // Analizo para buscar el valor correcto
   if(hz_x10>=frecRef_x10) minimo_error=osccal;
   if(hz_x10<=frecRef_x10) maximo_error=osccal;

   // Si mínimo error pasa a ser menor que máximo error, o
   // son iguales, ya terminó.
   if(minimo_error<=maximo_error) break;
  }
 }
 // Aplicar y guardar, si no hubo error. Si no, dejo
 // el que estaba
 limpiaX2();
 if(minimo_error>maximo_error || aborta)
 {
  // Debo recuperar a OSCCAL
  OSCCAL=osqui;
  pantalla_println(F("NO"));
 }
 else
 {
  // Guardo el valor calibrado
  eeprom_update_byte(&ee_osccal,OSCCAL);
 }
 icali();
 pantalla_print(F("!\nCAL="));
 pantalla_println(OSCCAL);
}
#endif

void acercade(void)
{
 limpiaX2();
 pantalla_println(F(NOMBRE TIPOF));
 pantalla_separacion(0);
 pantalla_println(F(VERSION TIPOV "\n" FECHA));
 pantalla_separacion(1);
 pantalla_println(F(AUTOR));
 delay(2000);
}

void modoCFG(void)
{
 restaurarOsc();       // Pone como entrada a PB2
 modoActual=MODO_CFG;
 limpiaX2();
 pantalla_print(F("CONFIGURAR"));
}

// SETUP

void setup(void)
{
 #if !defined(__AVR_ATtiny85__)
 // CONFIGURACIÓN ARDUINO NANO / PRO MINI
 // Para que no queden flotantes, pongo a todas en INPUT_PULLUP
 // y luego excluyo las que necesito. Y de paso favorece a los
 // pulsadores.
 // No toco a A0 que es por donde se mide.
 // A6 y A7 deben cablearse, ya que no es un port real
 for(byte i=0;i<=13;i++)  pinMode(i,INPUT_PULLUP);
 for(byte i=A1;i<=A5;i++) pinMode(i,INPUT_PULLUP);

 // Apago led
 pinMode(13,OUTPUT);
 digitalWrite(13,LOW);

 #endif

 cli();

 #if defined(__AVR_ATtiny85__)

 // CONFIGURACIÓN ADC (PB1 / ADC1)
 ADMUX=0xA1;   // REFS1, ADLAR, MUX0 (Referencia 1.1V, Ajuste Izq, Canal ADC1)
 delay(20); 
 ADCSRA=0xC3;  // ADEN, ADSC y Prescaler /8 (Lanza la conversión de limpieza de una vez)
 while(ADCSRA & (1<<ADSC)); // Espera a que termine

 // Veo si se compila sin cristal, para agregar dos pulsadores
 // directos a PB3 y PB4
 #ifdef SIN_CRISTAL
 #if PULSADORES>2
 pinMode(3,INPUT_PULLUP); // PB3 / entrada reloj externo
 #endif
 #if PULSADORES>1
 pinMode(4,INPUT_PULLUP); // PB4
 #endif
 #endif

 #else

 // Configuro ADC para máxima velocidad inicial
 // Mantiene 1.1V y canal ADC0
 //const byte canal=0;
 //ADMUX=(1<<REFS1) | (1<<REFS0) | canal;
 ADMUX=(1<<REFS1) | (1<<REFS0);

 // ADCSRA: Habilito ADC e interrupciones
 // Prescaler inicial de 128 para estabilidad
 ADCSRA=(1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

 //Configuro Temporizador 1 (16 bits)
 TCCR1A=0; 
 TCCR1B=0;
 TCNT1=0;
    
 // Configuro pata de Generador de Funciones. Siempre activa
 // Pongo como salida a D8 (PB0)
 DDRB|=(1<<PB0);

 // Alinea el resultado para leer solo ADCH
 ADMUX|=(1<<ADLAR);

 #endif

 // Realizar una conversión de "limpieza" para asentar
 // la referencia
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC));

 // Si es la primera vez, a es 0
 unsigned int id=eeprom_read_word(&ee_id);
 band=(id!=EEPROM_ID?band:(eeprom_read_word(&ee_band)));

 pantalla_comienzo();
 pantalla_rotar((band&BAND_GIRO)?0:1); // 1 o 0 para rotar 180 grados
 pantalla_encendida();

 acercade();     // Espera pulsador a fábrica

 sei();

 // Muestreo de prueba para que no se trabe el arranque
 // Arranca como osciloscopio
 modoActual=MODO_OSCILOSCOPIO;

 // Reinicio valores si se presiona el pulsador 1 al inicio
 if(leerPulsador()==1)
 {
  limpiaX2();
  pantalla_println(F("A NUEVO"));

  // Provoco que se borre los datos de la eeprom
  eeprom_update_word(&ee_id,0xFFFF);

  // Espero a soltarlo
  while(leerPulsador());
 }

 // Inicializo EEPROM, si corresponde
 if(id!=EEPROM_ID)
 {
  // Condiciones iniciales
  band=(BAND_MODOGATILLO | BAND_AUTOESCALA);
  eeprom_update_word(&ee_id, EEPROM_ID);
  eeprom_update_byte(&ee_adc_fs_0,0);     // Cal 0V
  eeprom_update_byte(&ee_adc_fs_1,230);   // Para 1V
  eeprom_update_byte(&ee_adc_fs_3,140);   // Para 3,3V
  eeprom_update_byte(&ee_adc_fs_5,226);   // Para 5V
  eeprom_update_byte(&ee_adc_fs_C,240);   // Para 12V
  eeprom_update_byte(&ee_osccal,OSCCAL);  // Valor inicial
  eeprom_update_byte(&ee_rangoActual,33); // Actual 3,3 V
  eeprom_update_word(&ee_band,band);
 }

 // Traigo valores guardados
 rangoActual=eeprom_read_byte(&ee_rangoActual);
 cargarFactorPorRango();
 adc_offset=eeprom_read_byte(&ee_adc_fs_0);

 // Traigo calibración de frecuencia
 OSCCAL=eeprom_read_byte(&ee_osccal);

 // Para pruebas directas y entrar en modo generador
 //modoActual=MODO_GENERADOR;
 //activarGenerador(genFrec);  // Sigue en la que estaba
}

//  LOOP

void loop(void)
{
 if(modoActual==MODO_OSCILOSCOPIO)
 {
  // Mido según la escala actual
  byte Gato;
  unsigned long int hz_x10=analiza(escala,Gato);
  while(!aborta)
  {
   // Si hubo una detección incompleta,
   // insisto multiplicando la escala
   // Se activa solamente si hay autoescala
   // Si no hay señal, mostrará una línea
   // Insisto unas pocas veces
   byte i=0;
   while(Gato==255 && (band&BAND_AUTOESCALA) && !aborta && i<5)
   {
    // Multiplico la escala
    escala*=2;
    if(escala>ESCALA_MAXIMA)
    {
     escala=ESCALA_MAXIMA;
     break;               // No hay indicios de señal
    }

    // Si gato resulta diferente a 255, sale
    hz_x10=analiza(escala,Gato);
    i++;
   }

   // Si se abortó, no sigo
   if(aborta) break;

   // Si de nuevo solo hay indicios, no sigo con eso
   // ya que no es autoescala
   if(Gato==255) Gato=0;

   // Veo de pasar donde empieza si es con gatillo, y
   // si es libre, será cero.
   byte comienzo=0;
   if(band&BAND_MODOGATILLO) comienzo=Gato;

   // Al salir de aquí, ya tengo datos

   // Dibujo onda y pie. Redondeo la frecuencia en 0,5 Hz.
   // Siempre dibujo en base a lo que hay
   // Si se activó la captura, retengo e invierto
   actualizar(comienzo,hz_x10,tiempoReal_us);
   if(capan)
   {
    // La pantalla ahora es "negro sobre blanco"
    pantalla_invertir(true);

    // Espero a soltar y presionar el pulsador
    while(leerPulsador());
    while(!leerPulsador());

    // Vuelve a "blanco sobre negro"
    pantalla_invertir(false);
    delay(PRESION_PULSADOR);
    capan=false;               // Desactivo captura
   }

   // Para la próxima medición, hará autoescala, si se
   // selecciona así. Ajusto en base a mediciones previas
   // Trato de mostrar un ciclo completo
   if(band&BAND_AUTOESCALA)
   {
    if(hz_x10>5)  // Medio Hz es un límite
    {
     int antes=escala;
     // Autoescala para frecuencias mayores
     // Puntos del oled en el que quiero ver un ciclo completo
     escala=((unsigned long)(1000000UL/(unsigned long)hz_x10)/PUNTOS);
     if(abs(antes-escala)<3)   // Para que no tiemblen escalas
     {
      escala=antes;
     }
    }
    else
    {
     escala=ESCALA_MAXIMA;   // No hay entrada o es muy lenta
    }
   }
   break;
  }
  // Menú OSCILOSCOPIO
  byte accion=menu();
  switch(accion)
  {
   case 253:
    break;
   case 254:
    break;
   case 255:            // Pulso breve
    capan=true;
    limpiaX2();
    pantalla_print(F("Capturando"));
    delay(PRESION_PULSADOR);
    break;
   case MENU_OSC_AUTOESCALA:
    band|=BAND_AUTOESCALA;
    band&=~BAND_ESCALAR;
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS1:
    escala++;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS10:
    escala+=10;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS100:
    escala+=100;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS1:
    escala--;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS10:
    escala-=10;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS100:
    escala-=100;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAX:
    escala=ESCALA_MAXIMA;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MIN:
    escala=ESCALA_MINIMA;
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_LIBRE_AUTO:
    band^=BAND_MODOGATILLO;
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_GRILLA:
    band^=BAND_MOSTRARGRILLA;
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_LINEAS:
    band^=BAND_MODOLINEA;
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_ESCALAR:
    band^=BAND_ESCALAR;
    eeprom_update_word(&ee_band,band);
    break;
   case MENU_OSC_SEL_CAL1:
    rangoActual=RANGO_CAL1;
    cargarFactorPorRango();
    break;
   case MENU_OSC_SEL_CAL3:
    rangoActual=RANGO_CAL3;
    cargarFactorPorRango();
    break;
   case MENU_OSC_SEL_CAL5:
    rangoActual=RANGO_CAL5;
    cargarFactorPorRango();
    break;
   case MENU_OSC_SEL_CALC:
    rangoActual=RANGO_CALC;
    cargarFactorPorRango();
    break;
   case MENU_OSC_GENERADOR:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   case MENU_OSC_CONF:
    modoCFG();
    return;               // Va a loop()
  }
  if(escala<ESCALA_MINIMA) escala=ESCALA_MINIMA;
  if(escala>ESCALA_MAXIMA) escala=ESCALA_MAXIMA;
 }
 else if(modoActual==MODO_GENERADOR)
 {
  // Veo de hacer un menú GENERADOR al apretar un pulsador
  int g=genFrec;
  byte accion=menu();
  switch(accion)
  {
   case 253:
    break;
   case 254:
    break;
   case 255:
    break;
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
   case MENU_GEN_FRECEN50:
    genFrec=50;               // 50 Hz
    break;
   case MENU_GEN_FRECEN100:
    genFrec=100;              // 100 Hz
    break;
   case MENU_GEN_FRECEN500:
    genFrec=500;              // 500 Hz
    break;
   case MENU_GEN_FRECEN1000:
    genFrec=1000;             // 1000 Hz
    break;
   case MENU_GEN_FRECEN10000:
    genFrec=10000;             // 10000 Hz
    break;
   case MENU_GEN_OSCILOSCOPIO:
    restaurarOsc();
    break;
   case MENU_GEN_PUL1_VOLVER:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   #if PULSADORES>1
   case MENU_GEN_PUL2_VOLVER:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   #endif
   #if PULSADORES>2
   case MENU_GEN_PUL3_VOLVER:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   #endif
   case MENU_GEN_CONF:
    modoCFG();
    break;
  }
  if(genFrec<1) genFrec=1;
  if(genFrec>32000) genFrec=32000;
  if(g!=genFrec) activarGenerador(genFrec);
 }
 else
 {
  // Modo configurador
  byte accion=menu();
  switch(accion)
  {
   case 253:
    break;
   case 254:
    break;
   case 255:
    break;
   case MENU_CFG_CAL_0:
    calibrarRangoGenerico(RANGO_CAL0,0);
    delay(3000);
    modoCFG();
    break;
   #ifndef SIN_CRISTAL
   case MENU_CFG_CAL_1:
    calibrarRangoGenerico(RANGO_CAL1,10);
    delay(3000);
    modoCFG();
    break;
   #endif
   case MENU_CFG_CAL_3:
    calibrarRangoGenerico(RANGO_CAL3,33);
    delay(3000);
    modoCFG();
    break;
   case MENU_CFG_CAL_5:
    calibrarRangoGenerico(RANGO_CAL5,50);
    delay(3000);
    modoCFG();
    break;
   case MENU_CFG_CAL_C:
    calibrarRangoGenerico(RANGO_CALC,120);
    delay(3000);
    modoCFG();
    break;
   #ifdef SIN_CRISTAL
   case MENU_CFG_FREC50:
    calibrarFrecuencia(500);
    delay(3000);
    modoCFG();
    break;
   case MENU_CFG_FREC60:
    calibrarFrecuencia(600);
    delay(3000);
    modoCFG();
    break;
   #endif
   case MENU_CFG_OSCILOSCOPIO:
    restaurarOsc();
    break;
   case MENU_CFG_GENERADOR:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   case MENU_CFG_GIRO:
    band^=BAND_GIRO;
    pantalla_rotar((band&BAND_GIRO)?0:1);
    eeprom_update_word(&ee_band,band);
    restaurarOsc();
    break;
   case MENU_CFG_ACERCADE:
    acercade();
   case MENU_CFG_PUL1_VOLVER:
   #if PULSADORES>1
   case MENU_CFG_PUL2_VOLVER:
   #endif
   #if PULSADORES>2
   case MENU_CFG_PUL3_VOLVER:
   #endif
    modoCFG();
    break;
  }
 }
}
