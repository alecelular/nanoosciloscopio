# Nanoosciloscopio

Este proyecto detalla el principio de funcionamiento, el proceso de construcción y las soluciones implementadas para el desarrollo de un osciloscopio de dimensiones reducidas basado en microcontroladores.

## 🚀 Introducción

La documentación y el código están centrados principalmente en el **ATtiny85**. Sin embargo, el sistema ha sido adaptado para ser compatible con el **ATmega328P**. 

> **Nota:** Existe una descripción tentativa para el uso de un **ATtiny84**, aunque por el momento no se incluye código específico adaptado para este modelo.

---

## 🛠️ Generalidades y Compilación

### Requisitos de Software
Para compilar el código en el **ATtiny85**, se utilizó el **IDE de Arduino 1.8.19** con las siguientes especificaciones indispensables:
* **Core:** [ATtinyCore 1.4.1 / 1.5.2](http://drazzy.com/package_drazzy.com_index.json)
* **Opciones de compilación:** 
    * `No millis()` (Obligatorio para maximizar Flash).
    * `LTO habilitado`.
    * `Sin bootloader`.

### Gestión de Memoria y Frecuencia
* **Sin Cristal Externo (8 MHz interno):** El código alcanza casi el límite máximo de la memoria Flash del ATtiny85. Requiere calibración de frecuencia manual (50/60 Hz).
* **Con Cristal Externo:** El tamaño del código se reduce notablemente al no requerir rutinas de calibración de frecuencia, permitiendo más funcionalidades.
* **ATmega328P:** En este caso, la memoria Flash y RAM son sobrantes, permitiendo cualquier reforma adicional sin restricciones de espacio.

---

## 📺 Control de la Pantalla (OLED SSD1306)

El sistema utiliza pantallas basadas en el controlador **SSD1306** (128x64 o 128x32). 

### Configuración mediante `#define OLED`
La resolución y el área visible se ajustan mediante valores numéricos:
* **128x64:** Valores entre `5` y `8`, donde `8` es el normal. (El valor `7` deja un renglón libre al pie para info adicional).
* **128x32:** Valores entre `2` y `4`, donde `4` es el normal. (El valor `3` deja un renglón libre al pie).

*Sugerencia de hardware:* Reducir el valor del define permite "recuperar" pantallas que tengan líneas dañadas en los extremos, desplazando la visualización a la zona funcional.

### Implementación I2C Bitbang
Debido a que el uso de un cristal externo consume pines críticos, no se utilizan las bibliotecas I2C estándar. Se implementó un protocolo **Bitbang** propio:
* **SDA:** PB0 (AIN0)
* **SCL:** PB1 (AIN1)
* **Limitación de caracteres:** Para ahorrar Flash, el juego de caracteres está limitado hasta el código `0x5A` (solo mayúsculas). Esto se puede modificar cambiando `FUENTE_MAX` en `I2C.ino`.

---

## 🕹️ Interfaz de Usuario y Pulsadores

La cantidad de pulsadores se define mediante `#define PULSADORES` (1, 2 o 3).

### Pulsador 1 (Principal)
Se detecta mediante el **comparador interno** analizando el desbalance de tensiones en las líneas SDA/SCL. 
* No interfiere con la pantalla.
* Permite hacer "capturas de pantalla" o abortar mediciones de base de tiempo alta para acceder al menú.

### Pulsadores 2 y 3 (Opcionales)
* **Sin Cristal:** Pueden usarse pines dedicados.
* **Con Cristal:** Se utiliza el pin de **RESET** en modo analógico.
    * Se lee el valor de voltaje al final de cada ciclo de medición.
    * Diferentes resistencias permiten identificar qué pulsador fue presionado.
    * Si se presionan ambos a la vez, se provoca el reinicio del procesador.

---

## 🔧 Calibración

Dependiendo de la configuración de hardware, los pasos de calibración varían:

1.  **Con Cristal:** Solo es necesario calibrar las tensiones máximas de entrada (1V, 3,3V, 5V y 12V) desde el menú.
2.  **Sin Cristal:** Además de la tensión, se debe realizar la calibración de frecuencia (menús de 50 Hz y 60 Hz). La precisión es menor debido a la inestabilidad del oscilador interno.

> **Importante:** La inversión de pantalla en el menú también invierte la lógica de los pulsadores 1 y 3 para mantener la coherencia en la navegación.

---

## 🔍 Sección Osciloscopio

El sistema realiza un muestreo de la señal por cada interrupción del **Temporizador 1**. 
* **Escalas de tiempo:** Desde **1 (10 µs)** hasta **816 (8160 µs)** por punto, representados en los 128 píxeles de ancho.
* **Resolución mejorada:** Si se utiliza un cristal de **16 MHz**, la escala mínima baja a **5 µs** (ATtiny85) o **6 µs** (ATmega328P).
* **Referencia de tensión:** Se usa la referencia interna de **2,56V** (ATtiny85) o **1,1V** (ATmega328P). Por seguridad, el divisor resistivo debe ajustar la entrada para no superar los 2.3V.

### Información en Pantalla (Interface)
La pantalla (128x64) usa 56 píxeles para el gráfico y 8 para datos técnicos. La cadena de estado sigue este orden:
1. **Magnificación:** `x1`, `x2`, `x3` (para señales de baja tensión).
2. **Rango de entrada:** `1` (1V), `3` (3,3V), `5` (5V), `C` (12V).
3. **Escala:** `A` (Autoescala) o `M` (Manual).
4. **Detalle:** `4` (escala x4) o `N` (normal).
5. **Disparo:** `G` (Gatillo/Trigger ascendente) o `L` (Libre).
6. **Métricas:** Microsegundos por punto, frecuencia actual y tensión máxima (Vpp).

---

## ⚡ Sección Generador de Funciones

Permite generar ondas cuadradas entre **1 Hz y más de 20 kHz**. 
* **Funcionamiento:** Reconfigura el Temporizador 1 y utiliza el pin de entrada como salida.
* **Precisión:** Si la frecuencia es exacta, se muestra el símbolo `=`. Si es una aproximación, se muestra `#`.
* **ATmega328P:** La salida se redirige a un pin independiente (**D8/PB0**) para facilitar el uso de circuitos de salida personalizados.

---

## 📈 Sección Frecuencímetro

Mide frecuencias de ondas cuadradas (nivel lógico 0 a VCC) inyectadas en el pin de entrada, alcanzando fácilmente **1 MHz**.
* **ATtiny85:** Utiliza el contador interno **T0**. Es fundamental compilar sin `millis()` para evitar conflictos con el contador de tiempo.
* **ATmega328P:** Utiliza el contador **T1** en el pin **D5**, lo que permite separar la entrada de medición de la de frecuencia.
* *Nota:* Para señales menores a 10 kHz que no sean cuadradas, se recomienda usar el **Modo Osciloscopio**.

---

## ⚙️ Configuración y Calibración

En este menú se definen parámetros permanentes:
* Rotación de pantalla (orientación).
* Selección de escala de tensión para calibración.
* Calibración de frecuencia (solo si no se usa cristal externo).

---

## 🔌 Hardware y Conexionado

### Diseño de Placas
* Existen diseños de PCB (aprox. 5x5 cm) para versiones de **1 o 3 pulsadores** (archivos `.jpg`).
* **Alimentación:** Diseñado para baterías de litio con cargador **TP4056**. 
    * *Importante:* Para usar 16 MHz se requiere una batería bien cargada o una tensión ligeramente superior a la estándar de Li-ion, según la tolerancia del microcontrolador.
* **Programación:** Se incluye un conector en la parte inferior para reprogramar mediante **"Arduino as ISP"** sin retirar el chip.

### Detalles de Pines (ATtiny85)
* **PB2:** Entrada/Salida principal con resistencia a masa para el divisor de tensión. Se recomienda ingenio para crear puntas de prueba intercambiables si se requiere alta impedancia.

---

## 📝 Notas Finales

Este proyecto nace de la búsqueda de la mínima cantidad de componentes posibles. 
* **Autonomía:** Con una batería de **250 mA**, el equipo funciona más de **12 horas**.
* **Código Limpio:** No se utilizan bibliotecas externas, solo los encabezados básicos del procesador incluidos en el IDE de Arduino.

---

### Contacto y Créditos
**Autor:** Alejandro F. Fernández  
**Email:** [nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)  
**Año:** 2026

*Espero que disfruten este proyecto tanto como yo disfruté sus dos meses de desarrollo.*

