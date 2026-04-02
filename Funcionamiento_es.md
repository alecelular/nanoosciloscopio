# Principio de funcionamiento

(In Spanish, use a translator for other languages. If in doubt, the Spanish text is the correct version.)

## Sección osciloscopio

Se hace una muestra de la señal de entrada por cada interrupción del temporizador 1. Se crean escalas de tiempo que van desde **1 (10 µs)** hasta **816 (8160 µs)**, que marcan el ritmo de muestreo y se representan a lo largo de los **128 puntos de ancho de la pantalla**.

En cuanto a la entrada analógica:

* Se usa la referencia interna de **1,1 V** para mayor estabilidad.
* Un divisor resistivo adapta la señal de entrada para no superar, por seguridad, **1 V**.

La pantalla:

* Usa **56 puntos de altura** para graficar
* Los **8 restantes** para información (frecuencia, valor pico, etc.)

Esto aplica a pantallas **128x64**.
Para **128x32**, la onda se representa con menos puntos.

Configuración:

```
#define OLED 8   // 128x64
#define OLED 4   // 128x32
```

Caso particular:

* Se puede usar `OLED 7 (128x56)` si hay líneas dañadas al final, o si se quiere reservar espacio para más información

### Información en pantalla

Se muestra en este orden:

1. Magnificación: `x1`, `x2`, `x3`
2. Escala de entrada: `1`, `3`, `5`, `C` (1V, 3,3V, 5V, 12V)
3. Modo: `A` (auto) o `M` (manual)
4. Zoom: `4` (activo) o `N`
5. Trigger: `G` (gatillo) o `L` (libre)
6. Tiempo por punto (µs)
7. Frecuencia
8. Tensión máxima

---

## Pantalla e I2C (bitbang)

No se usan bibliotecas estándar porque ocupaban E/S necesarias.

Configuración:

* SDA → PB0 / AIN0
* SCL → PB1 / AIN1

Se implementa I2C por software (**bitbang**).

Notas:

* Se dejaron rutinas extra para otros dispositivos I2C
* Cuidado con resistencias pull-up duplicadas
* El controlador SSD1306 no incluye pull-ups, por eso se agregan externamente

### Fuente de caracteres

Limitada para ahorrar memoria:

```
#define FUENTE_MAX 0x5A
```

* Solo hasta `'Z'`
* Minúsculas se convierten a mayúsculas

Se puede ampliar (por ejemplo `0x7F`) si hay espacio disponible.

---

## Pulsador 1 (principal)

Se detecta usando el **comparador interno**, compartido con SDA y SCL.

Funcionamiento:

* Se genera un desbalance de tensión entre ambas líneas
* Al presionar el botón, cambia el estado del comparador

Ventajas:

* No interfiere con la medición
* Permite interrumpir la medición o capturar pantalla

---

## Pulsadores 2 y 3 (opcionales)

Opciones de implementación:

### Sin cristal

Se pueden usar E/S libres (no recomendado por precisión).

### Usando RESET como entrada analógica

* Se leen al final del muestreo
* No pueden leerse simultáneamente con la señal

Características:

* Detección más lenta en escalas grandes
* Cada botón genera un valor analógico distinto

Extra:

* Pulsar 2 y 3 juntos provoca un reset

Configuración:

```
#define PULSADORES 3   // o 1 si no se usan
```

---

## Interfaz de usuario

* Pantalla invertida → invierte botones 1 y 3
* Botón central → selecciona opciones

---

## Calibración

### Con cristal

* Solo se calibran las tensiones de entrada

### Sin cristal

* Se calibran:

  * Tensiones
  * Frecuencia (menús de 50 Hz y 60 Hz)

---

## Compilación

Se recomienda usar:

* ATTinyCore 1.4.1 / 1.5.2
* Sin `millis()`

Esto maximiza el uso de memoria flash.

---

## Compatibilidad con ATmega328P

Compatible con:

* Arduino Nano
* Pro Mini
* Otros basados en ATmega328P

Características:

* E/S adaptadas automáticamente
* Pulsadores 2 y 3 con E/S propias
* Navegación de menú más rápida

Aun así, la idea principal es usar **ATtiny85**.

---

## Frecuencia de trabajo

* A **16 MHz**:

  * Escala mínima: **5 µs**

* A **8 MHz**:

  * Escala mínima: **10 µs**

---

## Sección generador

Permite generar una onda cuadrada entre **1 Hz y más de 20 kHz**.

Características:

* Usa el Timer1 en un modo diferente
* Genera la frecuencia más aproximada posible a la solicitada

Indicadores:

* `=` → frecuencia exacta
* `#` → frecuencia aproximada

Notas:

* No funciona simultáneamente con el modo osciloscopio
* En ATmega328P, la salida está separada de la entrada

---

## Conexionado

El esquema está en la carpeta `hardware`:

* PDF del circuito (ATtiny85)
* Lista de componentes
* PCB en formato JPG

Detalles:

* Placa de aproximadamente **5 x 5 cm**
* Permite armar **dos equipos**

Incluye:

* Distribución de componentes
* Imágenes del montaje en Fotos

---

## Alimentación

Se utiliza un módulo basado en TP4056 (4 o 6 terminales).

Notas:

* Puede requerir un puente adicional según la versión
* Se recomienda usar batería en los siguientes casos:

### 8 MHz

* Funciona correctamente con batería

### 16 MHz

* Requiere mayor tensión
* Podría funcionar si la batería está completamente cargada

---

## Programación

Se agregó un conector inferior para reprogramar sin desmontar el microcontrolador.

Método:

* Arduino como ISP

Los detalles están en los archivos `.ino`.

---

## Otros detalles

* Hay más configuraciones disponibles mediante `#define` en el código
* El proyecto está optimizado para:

  * Bajo costo
  * Cantidad mínima de componentes

Autonomía:

* Batería de 250 mAh → más de 12 horas de uso

Recomendación:

* La versión de un pulsador es igual de funcional que la de tres, pero ocupa menos código
---

## Gracias

Espero que lo disfruten.

Este proyecto llevó aproximadamente dos meses entre diseño y programación.

Pueden existir mejoras u optimizaciones. Cualquier aporte es bienvenido.
