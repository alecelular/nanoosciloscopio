# Detalhes de funcionamento

👉 Versión en castellano [Funcionamiento_es.md](Funcionamiento_es.md) —
👉 English version [Funcionamiento_en.md](Funcionamiento_en.md)

Este documento complementa o [README](README_br.md): explica como o
equipamento é usado na prática — o seletor, o conector de
acessórios, os botões e o procedimento de calibração — a partir de
como o firmware realmente os implementa.

> Os nomes de tela citados neste documento são os que aparecem no
> visor quando compilado com `#define IDIOMA_BR`. O firmware também
> compila em espanhol (padrão, `IDIOMA_ES`) ou inglês
> (`#define IDIOMA_EN`) perto do início do `.ino`.

## Seletor, conector de entrada e acessórios

O "seletor" não é um componente à parte: é a leitura analógica de
um único pino compartilhado:

- **ATtiny85**: pino 1 (RESET/PB5/ADC0).
- **ATmega328P**: A1 (PC1).

Esse pino é lido entre medições (ou ao terminar uma) para saber qual
acessório está conectado, de acordo com a resistência que o
acessório coloca entre esse pino e o terra.

A entrada de sinal (ou saída do gerador) usa um conector barato tipo
USB, não por seguir a norma USB, mas porque é fácil de encontrar e
oferece os 4 terminais necessários:

- 2 de alimentação (um é VCC, disponível também para um sensor
  externo de temperatura/umidade).
- 1 de seletor.
- 1 de sinal: entrada do osciloscópio, saída do gerador, entrada do
  frequencímetro ou dados do sensor, conforme o acessório.

Cada acessório traz, de forma externa, o resistor de seletor
correspondente à sua função e, se for para o osciloscópio, o divisor
resistivo daquela escala na linha de sinal. Trocar de faixa ou de
função é simplesmente trocar de acessório.

### Tabela do seletor (valores de fábrica)

O pino do seletor é lido com referência ao VCC. Estes são os
valores nominais de fábrica (os que `selValores[]` traz na primeira
vez que o equipamento liga, antes de qualquer calibração):

| Resistência | ADC nominal de fábrica | Função |
|---|---|---|
| Curto para o terra (0 Ω) | 151 | Osciloscópio **X** |
| 5,6 kΩ | 172 | Osciloscópio **Y** |
| 10 kΩ | 186 | Osciloscópio **Z** |
| 22 kΩ | 203 | Gerador |
| 47 kΩ | 221 | Frequencímetro |
| 150 kΩ | 241 | Sensor de temperatura/umidade |
| Circuito aberto (sem resistor) | ≥250 | **Sem acessório conectado** (modo espera) |

### Como o firmware detecta qual acessório está conectado

O circuito aberto (≥250 contagens) é descartado primeiro e sempre
significa "sem acessório", sem comparar contra mais nada.

Para o restante dos casos, o firmware **não** usa janelas fixas em
torno de um valor nominal. Em vez disso, guarda na EEPROM
(`ee_r[0..5]`) o valor de ADC medido na última vez em que cada uma
das 6 posições foi calibrada (de fábrica, esses 6 valores são os da
tabela acima). Ao ler o seletor, o firmware percorre as 6 posições
calibradas e escolhe a que estiver **mais próxima** da leitura
atual. Se a distância até a mais próxima ainda for maior que 5
contagens (`SEL_TOL`), a leitura é considerada fora de qualquer
posição conhecida e informada como "Seletor inválido".

Na prática isso significa que, se duas posições calibradas
ficassem próximas demais entre si, vence a que estiver
objetivamente mais perto da leitura — o ideal é deixar pelo menos
~10-12 contagens de separação entre os valores calibrados de cada
posição, para que não haja ambiguidade.

## Comportamento ao ligar

Após a tela de apresentação (nome/versão), o firmware lê os botões:

| Ao ligar | Resultado |
|---|---|
| Nenhum botão | O modo é determinado pelo seletor (acessório conectado) |
| Botão **"+" (mais)** pressionado | Mostra "Reset" com confirmação SIM/NÃO; se confirmado com "+", apaga toda a EEPROM e a restaura aos valores de fábrica, e **em seguida entra direto no modo CONFIG** |
| Botão **"-" (menos)** pressionado | Entra direto no modo CONFIG (calibração), sem resetar nada |

## Uso no modo osciloscópio

Assim como no CONFIG, navega-se mantendo um botão pressionado (o
item muda a cada ~0,8 s) e soltando no que se quer executar:

- Mantendo **"+"**: `Autoaj.` → `<+1>` → `<+10>` → `<MAX>` →
  `Expandir` → `Grade` → `<Sair>` (e repete).
- Mantendo **"-"**: `Retem` → `Livre/Auto` → `<-1>` → `<-10>` →
  `<MIN>` → `Vetor/Ponto` → `Borda` → `<Sair>` (e repete).

O que cada um faz:

| Opção | Efeito |
|---|---|
| `Autoaj.` | Liga/desliga o ajuste automático da escala de tempo conforme a frequência detectada. Ao ativar, força o modo disparado. |
| `<+1>` / `<+10>` | Aumenta manualmente a escala de tempo (mais tempo por divisão) em 1 ou 10 passos válidos. Desliga Autoaj. e Expandir. |
| `<-1>` / `<-10>` | O mesmo, mas diminui a escala (menos tempo por divisão, varredura mais rápida). |
| `<MAX> / <MIN>` | Vai direto para a escala mais lenta ou mais rápida disponível. |
| `Expandir` | Alterna uma ampliação (zoom ×4) sobre a parte visível da captura. |
| `Grade` | Mostra ou oculta as linhas de referência (verticais a cada 20 px e horizontais em 0/25/50/75/100 %). |
| `Vetor/Ponto` | Alterna entre traço contínuo (linhas) e apenas os pontos amostrados. |
| `Borda` | Alterna o disparo entre borda de subida ou de descida. |
| `Livre/Auto` | Alterna entre varredura livre (sem esperar cruzamento) e modo disparado (espera um cruzamento pelo nível médio para sincronizar a onda). |
| `Retem` | Congela a tela (inverte o vídeo para indicar isso) até que qualquer botão seja pressionado; depois continua normalmente. |
| `<Sair>` | Sai do menu sem alterar nada. |

Todas essas opções (exceto Retem) ficam salvas na EEPROM assim que
são escolhidas, então permanecem após desligar e ligar. A escala de
tempo em si (em que passo "<+/-N>" ficou) não é salva: ao reiniciar,
volta ao que o padrão/autoescala deixar.

### Filtro de ruído na detecção de frequência

Antes de calcular uma frequência ou localizar o ponto de disparo, o
firmware descarta a captura se a amplitude pico a pico for menor
que `AMPLITUD_MINIMA_DETECCION` contagens de ADC (4 por padrão).
Isso evita que um pouco de ruído de fundo, sem nada conectado à
entrada, seja interpretado como um sinal real de alta frequência.
Quando isso acontece, a frequência mostrada é "-----" e a varredura
passa a ser livre desde o início da captura, como se não houvesse
sinal.

### Linha de status (modo osciloscópio)

Na parte inferior da tela é mostrado, da esquerda para a direita:

- **Lupa (1/2/3)**: fator de ampliação automática aplicado à onda
  quando sua amplitude é pequena em relação à escala total.
- **A/M**: Autoajuste ou Manual.
- **N/4**: Normal ou Expandido (bandeira "Expandir" do menu).
- **L / + / -**: modo de disparo. 'L' = varredura livre contínua,
  '+' = disparo por borda de subida, '-' = disparo por borda de
  descida.
- **Tempo (núm. + u)**: tempo real por divisão em microssegundos.
- **Frequência (núm. + H)**: frequência detectada, com uma casa
  decimal. Mostra "-----" se for menor que 0,5 Hz ou se não foi
  detectado um ciclo completo.
- **Amplitude (núm. + %) ou SAT**: porcentagem da amplitude de pico
  em relação à escala total. Mostra "SAT" se o sinal exceder a
  escala (satura).
- **Faixa (X/Y/Z)**: faixa de tensão de entrada selecionada.

## Uso no modo gerador

Mesma mecânica de navegação (segurar e soltar):

- Mantendo **"+"**: `<+1>` → `<+10>` → `<+100>` → `<+1000>` →
  `100 Hz` → `25 kHz` → `<Sair>` (e repete).
- Mantendo **"-"**: `<-1>` → `<-10>` → `<-100>` → `<-1000>` →
  `1 kHz` → `10 kHz` → `<Sair>` (e repete).

"`<+/-N>`" ajusta a frequência de saída em passos de 1, 10, 100 ou
1000 Hz; as opções de frequência fixa ("100 Hz", "25 kHz", etc.)
pulam direto para uma frequência definida. A faixa vai de 1 Hz a
25 kHz (satura nesses extremos). Ao confirmar qualquer mudança, a
tela mostra a frequência pedida e a que realmente pode ser gerada
(marcada com "=" se exata, ou "#" se for a aproximação mais próxima
possível com o hardware). `<Sair>` não muda nada, apenas reaplica a
frequência atual.

## Uso no modo frequencímetro

O valor medido é mostrado na tela em Hz, atualizando-se
automaticamente cada vez que a janela de medição se completa.

## Modo CONFIG (calibração)

Há apenas dois botões físicos, usados de duas formas conforme o
contexto:

- **Navegar o menu**: mantém-se um botão pressionado; a cada ~0,8 s
  a tela avança para o próximo item de uma lista; solta-se quando o
  item desejado aparece, o que o executa.
  - Mantendo **"+"**: `Cal 0` → `Cal X` → `Cal Y` → `Cal Z` →
    *(somente em builds sem cristal: `CAL 50Hz`)* → `POR` (Sobre) →
    `<Sair>` (e repete).
  - Mantendo **"-"**: `"X 0/151"` → `"Y 5k6/172"` → `"Z 10k/186"` →
    `"G 22k/203"` → `"F 47k/221"` → `"S 150k/241"` → `<Sair>` (e
    repete).
- **Confirmar/abortar** (tela "`<SIM/NAO>`"): "+" confirma (SIM),
  "-" aborta (NÃO).

### Calibrar uma escala de tensão (Cal X / Cal Y / Cal Z)

1. Projete o divisor resistivo do acessório para que, na tensão
   máxima que se quer medir, o ponto médio do divisor entregue uma
   tensão um pouco abaixo da referência interna do ADC no modo
   osciloscópio:
   - **ATtiny85**: referência especial de 2,56 V (não precisa de
     capacitor externo no AREF, o que libera esse pino); mire em não
     ultrapassar ~2,3 V — esse é o mínimo garantido pelo fabricante
     para essa referência, apesar da dispersão de fabricação entre
     unidades.
   - **ATmega328P**: referência de 1,1 V (com capacitor externo no
     AREF, já que dispõe de mais pinos); mire em não ultrapassar
     ~1 V (não os 2,3 V acima, que são só para o ATtiny85) — esse
     1 V é o mínimo garantido para essa referência de 1,1 V.
2. Aplique essa tensão máxima conhecida na entrada, com o
   acessório/seletor correspondente já conectado.
3. Entre no modo CONFIG ("-" ao ligar) e escolha Cal X, Cal Y ou
   Cal Z (segure "+" até vê-lo, depois solte).
4. A tela mostra "Ant:" (valor salvo) e "Novo:" (valor medido).
   Confirme com "+" (SIM) para salvá-lo como o novo 100% dessa
   escala, ou "-" (NÃO) para descartar.
   - Só é salvo se a leitura ficar dentro de uma faixa razoável (nem
     saturada, nem baixa demais); caso contrário, é rejeitado mesmo
     confirmando com "SIM".
5. Repita para cada escala em uso. O divisor não precisa ser exato:
   a calibração absorve a tolerância real dos resistores.

**Exemplo** para um acessório de 12 V com ATtiny85 (mirando em
2,3 V): relação necessária ≈ 12/2,3 ≈ 5,2:1 — por exemplo,
R_top=42 kΩ e R_bottom=10 kΩ dá 12 V × 10/52 ≈ 2,31 V. O valor exato
dos resistores não é crítico, já que o passo 4 calibra contra a
tensão realmente aplicada.

Se a medição for feita até a própria tensão de referência, não é
necessário divisor: conecta-se o sinal direto na entrada.

### Calibrar a tolerância dos resistores do seletor

Se o seletor de um acessório não cair na posição esperada (por
exemplo, por dispersão de tolerância do resistor real usado, ou
porque você trocou esse resistor por outro valor), pode ser
recalibrado:

1. Conecte o acessório com esse resistor de seletor.
2. Entre no modo CONFIG e escolha, mantendo "-", a opção
   correspondente ("X 0/151" / "Y 5k6/172" / "Z 10k/186" / "G 22k/203" /
   "F 47k/221" / "S 150k/241").
3. É mostrado "Ant:"/"Novo:"; confirmar com "+" só salva o novo
   valor se ele cair **estritamente entre 128 e 250 contagens de
   ADC** (evita aceitar um valor claramente inválido, por exemplo
   quase em curto ou quase em circuito aberto). Não há uma janela
   fixa em torno do nominal esperado: uma vez salvo, esse valor
   passa a ser o novo ponto de referência para essa posição, e o
   firmware a detecta depois por proximidade (ver "Como o firmware
   detecta qual acessório está conectado" acima).

Osciloscópio X não aparece nessa lista: por usar um curto para o
terra fixo por projeto, não tem tolerância de fabricação para
calibrar.

### Calibrar o oscilador interno (somente builds sem cristal)

O ATtiny85 pode ser compilado sem cristal externo, usando seu
oscilador RC interno a 8 MHz (ver [README_br.md](README_br.md),
seção "Uso sem cristal"). O RC interno não é preciso de fábrica nem
estável com a temperatura, então o firmware oferece uma calibração
contra uma frequência de referência conhecida: a rede elétrica, a
50 Hz ou 60 Hz conforme a compilação (constante `CAL50` no `.ino`:
definida calibra contra 50 Hz, comentada calibra contra 60 Hz).

Esta opção de menu (`CAL 50Hz` ou `CAL 60Hz`, conforme a
compilação) **só aparece** em builds sem cristal; numa compilação
com cristal externo ela não está disponível nem é necessária.

**Antes de começar:**
- O equipamento deve estar no modo osciloscópio, com um sinal
  conhecido de 50 ou 60 Hz já conectado à entrada (por exemplo, a
  frequência da rede elétrica, captada através do acessório e do
  divisor resistivo de alguma das escalas do osciloscópio).

**Procedimento:**
1. Entre no modo CONFIG ("-" ao ligar).
2. Segure "+" até ver "CAL 50Hz" (ou "CAL 60Hz") e solte.
3. Confirme com "+" (ou aborte com "-", que cancela sem alterar
   nada).
4. O equipamento mede em janelas de 600 ms (múltiplo tanto de 50 Hz
   quanto de 60 Hz) e percorre automaticamente todos os valores
   possíveis de `OSCCAL` (de 2 a 253), procurando o ponto em que a
   frequência medida cruza o valor esperado (500 ou 600, em décimos
   de Hz, conforme `CAL50`).
5. Se encontrar um cruzamento válido, salva o novo `OSCCAL` na
   EEPROM e mostra "PRONTO". Se não encontrar (por exemplo, se não
   houver sinal real conectado), restaura o `OSCCAL` que tinha antes
   de começar e mostra "Aborta", sem salvar nada.

O valor calibrado fica na EEPROM e só é recuperado ao ligar o
equipamento, então sobrevive a um desligar/ligar. Vale a pena
refazer a calibração se a temperatura ambiente mudar muito, ou se o
ATtiny85 for reprogramado (o que pode alterar o `OSCCAL` de
fábrica).

> Essa calibração é independente das escalas de tempo do
> osciloscópio: `configurarEscala()` já tem tabulada a relação entre
> passos de escala e microssegundos reais para 8 MHz (com ou sem
> cristal). Calibrar o `OSCCAL` não muda essa tabela, só corrige o
> quão rápido realmente roda o clock interno do ATtiny85.

### Voltar aos valores de fábrica

Manter "+" ao ligar e confirmar com "SIM" apaga toda a EEPROM
(todas as calibrações de tensão e de seletor) e a deixa nos valores
de fábrica, entrando em seguida direto no modo CONFIG para
recalibrar.

## Sensor de temperatura / umidade

- Detecta automaticamente DHT11/22/12/DS18B20.

Por isso o conector tipo USB também traz VCC: para poder alimentar
esse sensor externo.

## Conector de reprogramação (ATtiny85)

A placa inclui um conector para reprogramar o ATtiny85 sem
dessoldá-lo (atualizações de firmware), ou para lhe dar outro uso.
Segue o pinout padrão de programação ISP (por exemplo, com
"Arduino as ISP"):

| Sinal ISP | Pino ATtiny85 |
|---|---|
| RESET | pino 1 |
| VCC | pino 8 |
| SCK | pino 7 (PB2) |
| MISO | pino 6 (PB1) |
| MOSI | pino 5 (PB0) |
| GND | pino 4 |
