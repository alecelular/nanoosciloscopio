# Nano-osciloscópio com ATtiny85

Osciloscópio digital compacto baseado no ATtiny85, com display OLED,
gerador de funções, frequencímetro e modo sensor de temperatura
integrados, projetado para funcionar com uma quantidade mínima de
componentes (apenas 5 pinos de I/O no ATtiny85). Também compila,
sem alterar a lógica do programa, para **ATmega328P** (Arduino
Nano / Pro Mini).

👉 Versión en castellano [README.md](README.md)

👉 English version [README_en.md](README_en.md)

👉 Versão em português a seguir

> **Nota:** este repositório substitui o protótipo anterior deste
> mesmo projeto. A versão base (**V3.3**) tem uma placa de circuito
> impresso própria (**NOS41**), um esquema de botões diferente do
> protótipo original, e adiciona o modo sensor de temperatura.
> Versões posteriores, até a atual (**V3.5**), somam suporte a
> cristais de 12 e 20 MHz, uso sem cristal (oscilador RC interno
> calibrado contra a frequência da rede elétrica) e uma detecção de
> acessório por "vizinho mais próximo calibrado", (**NOS43**) embora
> o anterior também sirva, desde que sem esses diodos.
>
> Ver
> [Funcionamiento_br.md](Funcionamiento_br.md) para os detalhes.

Equipamento montado mostrando a frequência da rede de 50 Hz.
[![NOS41 mostrando 50 Hz da rede](Fotos/50Hz.jpg)](Fotos/50Hz.jpg)

Detalhes da montagem:
[![Protótipo NOS41 com bateria](Fotos/ATtiny85/IMG_20260527_131120.jpg)](Fotos/ATtiny85/IMG_20260527_131120.jpg)

Protótipo com Arduino Nano e display de 128x32
[![Arduino nano](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)

---

## Especificações

- **MCU:** ATtiny85 (principal), adaptável ao ATmega328P.
- **Display:** OLED SSD1306 (128x64 / 128x32) (//#define SH1106) / SH1106 (#define SH1106) por I2C bit-banged
  (sem `Wire.h`, sem biblioteca gráfica externa).
- **Escala de tempo:** ajustável, calibrada especificamente para
  cristais de 8, 12, 16 e 20 MHz (com correção de fase nos que não
  resultam em um múltiplo inteiro de µs), e também para o modo sem
  cristal (RC interno, somente a 8 MHz).
- **Resolução:** 8 bits (ADC interno do AVR).
- **Seleção de modo:** um único pino ADC com seletor resistivo —
  sem pinos dedicados por função.
- **Idioma dos menus:** espanhol por padrão; também compila em
  inglês ou português.
- **Funções integradas:**
  * Osciloscópio (autoescala, disparo por borda, modo linha ou
    ponto, varredura livre ou disparada, "esticar" x4, congelar
    captura, filtro de ruído por amplitude mínima).
  * Gerador de funções (onda quadrada, 1 Hz a 25 kHz).
  * Frequencímetro (até ~1 MHz em sinais quadrados).
  * Sensor de temperatura (DS18B20 ou DHT11/DHT22, autodetecta).
- Menu completo na tela para calibração, sem PC conectado,
  incluindo a calibração do próprio clock interno quando compilado
  sem cristal.

---

## Hardware necessário

- ATtiny85 (principal), adaptável ao ATmega328P. Para medições de
  tempo/frequência precisas o ideal é um **cristal externo**; o
  equipamento também pode ser compilado sem cristal (oscilador RC
  interno, ver abaixo), ao custo de precisão e de calibração manual.
- OLED SSD1306/SH1106 (128x64 ou 128x32, configurável por `#define`).
- Placa de circuito impresso própria: **NOS43** (SSD1306)— ver
  [`hardware/`](hardware/) (fonte em `nos43.xcf`).
- 2 botões.
- Opcional: módulo de carga tipo TP4056 + bateria de lítio 3,7 V,
  interruptor.

### Pinos — ATtiny85

| Pino | Função               | Sinal        |
|------|----------------------|--------------|
| 1    | RESET / Seletor      | PB5/ADC0     |
| 2    | Cristal A            | PB3/ADC3     |
| 3    | Cristal B            | PB4/ADC2     |
| 4    | TERRA                | 0 V          |
| 5    | SDA' (I2C bitbang)   | PB0/AIN0     |
| 6    | SCL' (I2C bitbang)   | PB1/AIN1     |
| 7    | Sai/Ent/Freq/Sensor  | PB2/ADC1/T0  |
| 8    | Alimentação          | VCC          |

> Numa compilação sem cristal, os pinos 2 e 3 (Cristal A/B) ficam
> livres para outro uso, já que não é preciso conectar nada neles.

### Pinos — ATmega328P (Arduino Nano, testado)

| Função             | Pino  | Sinal |
|--------------------|-------|-------|
| Sai/Ent/Sensor      | A0    | PC0   |
| Frequencímetro      | D5    | PD5   |
| SDA' (bitbang)      | D9    | PB1   |
| SCL' (bitbang)      | D10   | PB2   |
| Botão 1             | D2    | PD2   |
| Botão 2             | D7    | PD7   |
| Seletor             | A1    | PC1   |
| Capacitor           | AREF  | VREF  |

> ⚠️ No ATmega328P, a saída do **gerador de funções usa o mesmo pino
> A0/PC0** da entrada do osciloscópio e do sensor — não um pino
> independente D8/PB0 como foi descrito em algum momento. Ver
> `activarGenerador()` no código: no ATmega328P alterna
> `PINC=(1<<PC0)` (configurado como saída com `DDRC|=(1<<PC0)`), e
> só o ATtiny85 usa um pino próprio (PB2), por ser o único
> disponível além do seletor e do barramento I2C.

Esquemas completos em [`hardware/`](hardware/):
- `Esquema_Nano-Osciloscopio_ATtiny85.pdf` / `.json`
- `Esquema_Nano-Osciloscopio_ATmega328P.pdf` / `.json`
- `nos43.xcf` — desenho da placa NOS43.

### Seletor de modo (um único resistor faz tudo)

| Resistência | ADC aprox. | Modo              |
|-------------|-----------:|-------------------|
| Curto para o terra (0 Ω) | ~151 | Osciloscópio 'X' |
| 5,6 kΩ      | ~172       | Osciloscópio 'Y' |
| 10 kΩ       | ~186       | Osciloscópio 'Z' |
| 22 kΩ       | ~203       | Gerador          |
| 47 kΩ       | ~221       | Frequencímetro   |
| 150 kΩ      | ~241       | Sensor           |
| Aberto (∞)  | ~250-255   | Sem acessório (modo espera) |

Os valores acima são os de fábrica. Ficam guardados na EEPROM e
podem ser recalibrados pelo próprio menu (todos exceto Osciloscópio
X, que é um curto para o terra fixo por projeto); depois de
calibrada uma posição, o firmware deixa de compará-la contra uma
janela fixa e passa a detectar o acessório conectado pela
proximidade ao valor calibrado mais próximo entre as 6 posições.
Detalhes completos, com o algoritmo de detecção e as janelas de
tolerância, em [Funcionamiento_br.md](Funcionamiento_br.md).

---

## 📷 Evolução do protótipo

As fotos em [`Fotos/ATtiny85/`](Fotos/ATtiny85/) documentam três
etapas do desenvolvimento:

1. **Placa NOS41 gravada** (14/05) — a placa recém-feita, ainda sem
   componentes.
2. **Protótipo anterior em protoboard** (15/05) — uma montagem
   anterior em placa perfurada, usada para testar a lógica antes de
   passar para a placa definitiva.
3. **NOS41 montada** (27/05 em diante) — a placa final com todos os
   componentes soldados, OLED e bateria.

Em [`Fotos/Arduino_NANO_128x32/`](Fotos/Arduino_NANO_128x32/) há
também fotos da montagem de testes num Arduino Nano com OLED de
128x32.

---

## ⚡ Seção Gerador de Funções

Permite gerar ondas quadradas entre **1 Hz e mais de 20 kHz**.
* **Funcionamento:** Reconfigura o Timer e usa o pino de entrada
  como saída.
* **Precisão:** Se a frequência é exata, mostra o símbolo `=`. Se é
  uma aproximação, mostra `#`.
* **ATmega328P:** A saída é gerada no **mesmo pino de entrada/sensor
  (A0/PC0)** — não em um pino independente — para não somar um pino
  dedicado além dos já usados pelo osciloscópio e sensor.

## 📈 Seção Frequencímetro

Mede frequências de ondas quadradas (nível lógico 0 a VCC)
injetadas no pino de entrada, alcançando facilmente **1 MHz**.
* **ATtiny85:** Usa o contador interno **T0**. É fundamental
  compilar sem `millis()` para evitar conflitos com o contador de
  tempo.
* **ATmega328P:** Usa o contador **T1** no pino **D5**, o que
  permite separar a entrada de medição da de frequência.
* *Nota:* Para sinais menores que 10 kHz que não sejam quadrados,
  recomenda-se usar o **Modo Osciloscópio**.

---

## 🚀 Introdução

A documentação e o código estão centrados principalmente no
**ATtiny85**. Porém, o sistema foi adaptado para ser compatível com
o **ATmega328P**.

> **Nota:** Existe uma descrição tentativa para o uso de um
> **ATtiny84**, embora por enquanto não haja código específico
> adaptado para esse modelo.

### Requisitos de software

Para compilar o código no **ATtiny85**, foi usado o **IDE do
Arduino 1.8.19** com as seguintes configurações indispensáveis:
* **Core:** [ATtinyCore 1.5.2](http://drazzy.com/package_drazzy.com_index.json)
* **Opções de compilação:**
    * `No millis()` (Obrigatório para maximizar Flash e evitar
      conflitos com os contadores usados pelo frequencímetro e
      gerador).
    * `LTO habilitado`.
    * `Sem bootloader`.

Para **ATmega328P**, selecione diretamente essa placa (Arduino
Nano / Pro Mini) no IDE — o mesmo `.ino` detecta o microcontrolador
por `#if defined(__AVR_ATtiny85__)` e ajusta pinos, timers e
periféricos automaticamente.

### Seleção de cristal / fonte de clock

O firmware valida internamente as escalas de tempo do osciloscópio
para **8, 12, 16 e 20 MHz**, então no ATtinyCore você pode escolher
qualquer um desses quatro cristais conforme sua necessidade (menu
**Tools → Clock**):

| Cristal | Indicado para                                         | Vcc mínima segura |
|---------|---------------------------------------------------------|---------------------|
| 8 MHz   | Alimentação a bateria (menor consumo)                    | ~2,7 V              |
| 12 MHz  | Meio-termo                                                | ~3,3 V              |
| 16 MHz  | Alimentação com fonte externa de 5 V (maior precisão/velocidade) | ~4,5 V     |
| 20 MHz  | Velocidade máxima, exige Vcc próximo do nominal          | ~5,0 V              |

### Uso sem cristal (oscilador RC interno) — somente ATtiny85

O equipamento também pode ser compilado sem cristal externo,
usando o oscilador RC interno do ATtiny85 a 8 MHz (menu **Clock
Source → Internal 8 MHz** no ATtinyCore, ou qualquer compilação em
que o core defina `CLOCK_SOURCE==0`). Isso libera os dois pinos do
cristal (2 e 3) para outro uso, mas o RC interno não é preciso de
fábrica nem estável com a temperatura, então precisa ser calibrado
contra uma frequência conhecida (rede elétrica de 50 ou 60 Hz) pelo
próprio menu do equipamento. O procedimento passo a passo está em
[Funcionamiento_br.md](Funcionamiento_br.md).

> ⚠️ Essa opção só compila a 8 MHz: em qualquer outra frequência com
> `CLOCK_SOURCE==0` o `.ino` gera um erro de compilação de propósito
> ("No puede usarse sin Cristal que no sea a 8 MHz y solo para
> pruebas"), porque a calibração só faz sentido nessa velocidade.

### Habilitar o oscilador RC interno no ATmega328P (MiniCore)

O core nativo do Arduino para ATmega328P não tem opção de fonte de
clock: sempre assume cristal externo. Para também compilar um
Arduino Nano / Pro Mini (ATmega328P) sem cristal, é preciso o core
[MiniCore](https://github.com/MCUdude/MiniCore) e adicionar uma
linha ao seu `boards.txt`, por exemplo:

```
~/.arduino15/packages/MiniCore/hardware/avr/<versão>/boards.txt
```

Localizando o bloco da opção "Internal 8 MHz" dessa placa e
adicionando, ao final desse bloco, uma linha que passe a flag de
compilação equivalente:

```
328.menu.clock.8MHz_external.build.extra_flags=-DCLOCK_SOURCE=0
```

> O nome exato da chave (`328.menu.clock.<algo>`) pode variar
> conforme a versão do MiniCore instalada. O ideal é abrir o seu
> próprio `boards.txt`, localizar o bloco real de "Internal 8 MHz" e
> adicionar a linha ali, em vez de assumir que a chave acima
> coincide letra por letra com a sua instalação.

### Idioma dos menus

Por padrão o firmware compila com os textos de menu em espanhol
(`IDIOMA_ES`, implícito). Para compilá-lo em inglês ou português,
adicione uma destas linhas perto do início do `.ino`, antes do
resto das definições:

```cpp
#define IDIOMA_EN   // Inglês
#define IDIOMA_BR   // Português
```

### Ajuste fino de detecção de sinal

`AMPLITUD_MINIMA_DETECCION` (no `.ino`, 4 contagens de ADC por
padrão) é o limiar mínimo de amplitude para que o firmware considere
que há um sinal real e não apenas ruído de fundo, tanto para
calcular a frequência quanto para localizar o ponto de disparo. Se
o equipamento "inventa" uma frequência com a entrada desconectada,
convém aumentar esse valor; se em vez disso ele ignora sinais reais
muito pequenos, convém diminuí-lo.

### Colocando em funcionamento

1. Carregue o código de [`src/`](src/) (`NOS_V3.5.ino` +
   `I2C.ino`).
2. Programe o ATtiny85 via ISP (ver tabela de pinos ISP no início
   do `.ino`), ou envie direto se estiver usando um Arduino
   Nano/Pro Mini.
3. Calibre as tensões de entrada pelo menu de configuração.
4. Conecte o sinal a medir e ajuste os parâmetros pelo próprio
   equipamento.

---

## Estrutura do repositório

```
.
├── src/
│   ├── NOS_V3.5.ino     # Programa principal: ADC, timers, menus, modos
│   └── I2C.ino          # Driver I2C por software (bit-banging) para o OLED
├── hardware/
│   ├── Esquema_Nano-Osciloscopio_ATtiny85.pdf/.json
│   ├── Esquema_Nano-Osciloscopio_ATmega328P.pdf/.json
│   └── nos43.xcf         # Fonte gráfica (GIMP) relacionada ao desenho
├── Fotos/
│   ├── ATtiny85/              # Protótipo NOS41/NOS43 montado
│   └── Arduino_NANO_128x32/   # Protótipo de testes sobre Arduino Nano
├── README.md
├── README_en.md
├── README_br.md
├── Funcionamiento_es.md
├── Funcionamiento_en.md
├── Funcionamiento_br.md
├── LICENSE_es
└── LICENSE_en
```

---

## Limitações conhecidas

- Sem cristal externo, a precisão de tempo/frequência depende de
  uma calibração manual do oscilador RC interno (somente ATtiny85,
  somente a 8 MHz) e pode variar com a temperatura.
- Disparo (trigger) básico, sem memória de aquisição prolongada.
- Suporte ao ATmega328P testado em menor extensão que o ATtiny85.

## Contribuições

São aceitas sugestões, correções e variantes de hardware via
issues ou pull requests. Agradecemos relatos de melhorias ou erros.

## Autor

Alejandro F. Fernández (alecelular)
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## Licença

Uso não comercial — ver [`LICENSE_es`](LICENSE_es) /
[`LICENSE_en`](LICENSE_en).

Se quiser usar comercialmente, entre em contato:
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## Apoie o projeto

Se isso foi útil para você, pode me pagar um café:
[![Me pague um café](https://cdn.cafecito.app/img/buttons/button_1.svg)](https://cafecito.app/rsp148)

---

*Espero que vocês aproveitem este projeto tanto quanto eu aproveitei
desenvolvê-lo.*
