<h1><strong>Relógio com LED</strong></h1>
<strong>Dev's: Andre Ribeiro, Carlos Trevisan, Felipe Bis, Ian Degaspari</strong>

<h2><strong>Descrição</strong></h2>

Um relógio com 86 leds, onde as cores são baseadas no horário durante o dia.

<h2><strong>Funcionamento</strong></h2>

A comunicação com os leds é feita a partir de um ESP32. Utiliza-se o ESP32 para se conectar com a internt e poder capturar a hora, minutos, e segundos em tempo real com base no WI-FI.

O Relógio possui 86 leds onde a representação da hora é feita com fileiras de 4 leds formando dois digitos, conforme a imagem abaixo:

![alt text](https://github.com/IanDegaspari/led_clock/blob/main/connections.png)

Já os minutos e segundos são representados por duas fileiras de 15 leds, portando, cada led representa 4 minutos, com isso é feito um esquema de cores para representar os 4 minutos completo, se o do minutos, ou segundo estiver da cor da hora, esse minuto/segundo está completo totalizando 4 minutos, caso contrário, ele representa 1, 2, ou 3 mintuos/segundos.

Ao longo do dia a cor do relógio vai mudando, a noite a paleta de cor se aproxima do azul/roxo, onde 00h temos o roxo, durante o dia as cores se aproximam do amarelo/vermelho, onde o vermelho estará presente as 12h.

Videos:

![alt text](videos/demonstracao1.gif)
![alt text](videos/demonstracao4.gif)
![alt text](videos/demonstracao5.gif)
