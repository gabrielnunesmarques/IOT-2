# IOT-2
ABP IOT 2 - 2021

# Sistema de gestão e controle das luzes de um aeroporto e balizador de aeronave no pit

Este projeto foi integrado a plataforma de IOT ThingSpeak e pode ser acesso pelo link: https://thingspeak.com/channels/1602478/sharing

## Ideologia

A ideia deste projeto é assegurar que as luzes das pistas do aeroportos esteja operando de forma correta e automatizadas.

Os equipamentos necessários para o funcionamento do projeto são:

* WeMos D1 com ESP8266
* Sensor Ultrasônico HC-SR04
* (1x) LED RGB do tipo catodo comum para a luz da Torre de controle
* (2x) LED Vermelho do tipo catodo comum para iluminação da pista e sistema de balizamento
* (1x) LED Verde do tipo catodo comum para iluminação no sistema de balizamento
* (1x) LED AMBAR do tipo catodo comum para iluminação da pista

Quando algum aeroporto está operando em baixa visibilidade ou de modo noturno, denominamos a situação de IRF ou VRF. Desta forma o aeroporto 
assume a responsabilidade de manter a luz da torre de controle e da pista principal piscando intermitantemente em BRANCO e VERMELHO e AMBAR E VERMELHO respectivamente.

Neste projeto também é realizado o desenvolvimento de um balizador de aeronaves. Para que os pilotos possam parar exatamente no ponto correto, afim de evitar transtornos
com a ponte de embarque e de diminuir a necessidade de um auxiliar para balizamento.

Para executar corretamente o programa, altere no código o valor das variaveis "SSID" e "password" para os dados da sua rede WiFi.

Após compilar o código pela primeira vez, verifique no monitor serial "BAUDE RATE DE 115200" o IP que sua rede alocou para o ESP.

Verifique e acompanhe os resultados no servidor Web.

Pinagem:

D0 = Led Verde do Balizador
D1 = Led Vermelho do Balizador
D2 = Pino Echo do HC-SR04
D3 = Pino Trigger do HC-SR04
D4 = Led Vermelho da pista principal
D5 = Led Ambar da pista principal
D6 = Pino RED do led RGB da Torre
D7 = Pin Blue do led RGB da Torre
D8 = Pino Green led RGB da Torre
A0 = Pino do sensor LDR
  
