# PeakDetection

**PeakDetection** é uma biblioteca Arduino para detecção de pico em tempo real em dados de sensores.

[🇺🇸 Translate](https://github.com/leandcesar/PeakDetection/blob/master/README.md) | [🇫🇷 Traduire](https://github.com/leandcesar/PeakDetection/blob/master/locale/README.fr.md)

## Algoritmo

Baseia-se no [princípio de dispersão](https://pt.wikipedia.org/wiki/Teoria_da_dispers%C3%A3o): se um novo data point é um número `x` de [desvios-padrão](https://pt.wikipedia.org/wiki/Desvio_padr%C3%A3o) de uma [média móvel](https://pt.wikipedia.org/wiki/M%C3%A9dia_m%C3%B3vel), o algoritmo sinaliza (também chamado de z-score).

O algoritmo tem 3 entradas:

* `lag`: é o atraso da janela em movimento (tamanho da média móvel). Esse parâmetro determina o quanto seus dados serão suavizados e o quanto o algoritmo é adaptável a mudanças na média de longo prazo dos dados. Quanto mais estacionário for o seu dado, mais atrasos você deve incluir. Se os seus dados contiverem tendências que variam com o tempo, considere a rapidez com que você deseja que o algoritmo se adapte a essas tendências.

* `threshold`: este parâmetro é o número de desvios-padrão da média móvel acima da qual o algoritmo classificará um novo data point como sendo um sinal. Este parâmetro deve ser definido com base em quantos sinais você espera. O threshold, portanto, influencia diretamente a sensibilidade do algoritmo e, portanto, a frequência com que o algoritmo sinaliza.

* `influence`: é o z-score no qual o algoritmo sinaliza. Este parâmetro determina a influência dos sinais no threshold de detecção do algoritmo. Se colocar em 0, os sinais não terão influência sobre o threshold, de forma que os sinais futuros serão detectados com base em um threshold que é calculado com uma média e um desvio-padrão que não é influenciado por sinais anteriores. Você deve colocar esse parâmetro entre 0 e 1, dependendo da medida em que os sinais podem influenciar sistematicamente a tendência de variação de tempo dos dados.

## Biblioteca

* `begin()`: inicializa o objeto `PeakDetection` e as configurações para os parâmetros de ajustes. Se nenhum parâmetro for definido, o padrão permanece. Parâmetros: `lag` (padrão=32), `threshold` (padrão=2), `influence` (padrão=0.5).

* `add()`: adiciona um novo data point ao algoritmo, calcula o desvio-padrão e a média móvel.

* `getPeak()`: retorna o status do pico do último data point adicionado. {-1, 0, 1}, representando abaixo, dentro ou acima do threshold do desvio padrão, respectivamente.

* `getFilt()`: retorna o último data point filtrado pela média móvel.

## Instalação

Para usar essa biblioteca:

1. Baixe o zip e descompacte o arquivo baixado.
2. Copie a pasta para a pasta de bibliotecas do Arduino (`C:/Users/username/Documents/Arduino/libraries`).
3. Renomeie para `PeakDetection`.

## Exemplo

```C++
#include <PeakDetection.h>                       // importa a biblioteca

PeakDetection peakDetection;                     // cria um objeto PeakDetection

void setup() {
  Serial.begin(9600);                            // define a taxa de dados para a comunicação serial
  pinMode(A0, INPUT);                            // pino analógico usado para conectar o sensor
  peakDetection.begin(48, 3, 0.6);               // define o lag, threshold e influence
}

void loop() {
    double data = (double)analogRead(A0)/512-1;  // lê o valor do sensor e converte em um intervalo entre -1 e 1
    peakDetection.add(data);                     // adiciona um novo data point
    int peak = peakDetection.getPeak();          // retorna 0, 1 ou -1
    double filtered = peakDetection.getFilt();   // média móvel
    Serial.print(data);                          // imprime o data point
    Serial.print(",");
    Serial.print(peak);                          // imprime o status do pico
    Serial.print(",");
    Serial.println(filtered);                    // imprime a média móvel
}
```

![Exemplo](https://github.com/leandcesar/PeakDetection/blob/master/docs/assets/example.gif)

## Agradecimentos

* [StackOverFlow](https://stackoverflow.com/questions/22583391/peak-signal-detection-in-realtime-timeseries-data)
