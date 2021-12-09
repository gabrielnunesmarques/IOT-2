#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ultrasonic.h>
#define INTERVALO_ENVIO_THINGSPEAK 30000


void envia_informacoes_thingspeak(String string_dados);

const char* ssid     = "SATC IOT";
const char* password = "IOT2021#";


//configuração da api thingspeak
char endereco_api_thingspeak[] = "api.thingspeak.com";
String chave_escrita_thingspeak = "CRMYDWI12CSJ4P80";
unsigned long last_connection_time;
WiFiClient client;

//definição de porta do server
ESP8266WebServer server(80);

//variavek que armazena o request do http
String header;

//variaveis para armazenar os estados dos pinos GPIO
const int output0 = 16; // D0 = bal_verde
const int output1 = 5; // D1 = bal_vermelho
const int output2 = 4; // D2 = pino_echo
const int output3 = 0; // D3 = pino_trigger
const int output4 = 2; // D4 = pp_led_verm
const int output5 = 14; // D5 = pp_led_amb
const int output6 = 12; // D6 = aer_vermelho
const int output7 = 13; // D7 = aer_azul
const int output8 = 15; // D8 = aer_verde
const int output9 = A0; // A0 = pinoLDR


//definicao da variavel led
const int led = 2;


//controle luminosidade
const int pinoLDR = A0; //GPIO A0

//led torre do aerodromo
 const int aer_vermelho = D6;
 const int aer_azul = D7;
 const int aer_verde = D8;

//led pista principal
const int pp_led_amb = D5; 
const int pp_led_verm = D4;
 
//led balisamento
const int bal_verde = D0; //MOSI_D7 
const int bal_vermelho = D1; //MISO_D6

//controle ultrasonico
#define pino_trigger D3 //scl_D1
#define pino_echo D2 //sda_D2

//definição dos pinos trigger e echo do sensor ultrasonico
Ultrasonic ultrasonic(pino_trigger, pino_echo);

//inicio do setup
void setup(void){
  pinMode(led, OUTPUT); //variavel led como seu pino em forma de saida
  digitalWrite(led, 0); //mantem a varial em LOW
  Serial.begin(115200); //inicia a serial em 115200
  WiFi.mode(WIFI_STA); //definição do modo de conexao wifi
  WiFi.begin(ssid, password); //inicia a conexao wifi lendos as informaões de nome e senha da rede
  Serial.println(""); //quebra de linha

  // Aguardar conexao wifi
  while (WiFi.status() != WL_CONNECTED) { //enquanto não estiver conectado
    delay(500); //delay de 500
    Serial.print("."); //vai printando no serial vários "." (pontos).
  }
  Serial.println(""); //quebra de linha
  Serial.print("Conectado a "); //printa que está conectado a rede referenciada na variavel ssid
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //printa o IP recebido na rede

  if (MDNS.begin("esp8266")) { //inicia o serviço de resolução de nomes para o esp8266 local
    Serial.println("MDNS responder started");//printa que inicio o serviço MDNS
  }

  server.on("/", handleRoot); //servidor online inicia a função handleroot

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound); //se o servidor não iniciar chama a função handleNotFound 

  pinMode(pinoLDR, INPUT); /pino do LDR como saída
  
  //Definição dos pinos do LED RGB  da Torre como saída
  pinMode(aer_vermelho , OUTPUT); 
  pinMode(aer_azul, OUTPUT);
  pinMode(aer_verde, OUTPUT);


  //Definição dos pinos dos LEDs da pista principal como saída
  pinMode(pp_led_amb, OUTPUT); 
  pinMode(pp_led_verm, OUTPUT);

  //Definição dos pinos dos LEDS do balizador como saída
  pinMode(bal_verde, OUTPUT); 
  pinMode(bal_vermelho, OUTPUT);
  
  server.begin(); //inicia porta serial
  Serial.println("HTTP server started"); //print que o servidor inicou
}

void loop(void){

    char fields_a_serem_enviados[100] = {0}; //campos a serem enviados para o ThingSpeak
    float distancia_lida = 0.0;
    float luminosidade_lida = 0.0;
    
     if (client.connected()) //se cliente estiver conectado
    {
        client.stop(); //inicia contagem de desconexao
        Serial.println("- Desconectado do ThingSpeak"); //apresenta mensagem de desconexao
        Serial.println(); //quebra de linha
    }

        
    server.handleClient(); //função auxiliar de apresentação no servidor
  
     //CONTROLE DAS LUZES
    float lumi = 0; //declaracao da variavel lumi
    lumi = analogRead(pinoLDR); //variavel recebe valor da leitura do pinoLDR
    if((lumi) > 600) { //se a resistencia for maior que 600
      pp_irf(); //estarta a função auxiliar para acionamento da pista em irf
      delay(100);
      aer_irf_ligado(); //estarta a função auxiliar para acionamento da torre em irf
      delay(100);
    }
    else{ //caso contrario deve desligar os leds de informação IRF mas manter o led da pista principal aceso na cor AMBAR
      aer_irf_desligado(); 
      digitalWrite(pp_led_amb, HIGH);
    }

    //CONTROLE DE BALIZAMENTO
    float cmMsec;  //variavel para armazenar a distancia em centimentros
    float DistM; //variavel para armazenar a distancia em metros
    long microsec = ultrasonic.timing(); //funcao do ultrasonico para medicao
    cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM); //conversao dos valores lidos pelo ultrasonico para resultado em centimetros
    DistM = cmMsec / 100; //converção de centimetros para metros
    Serial.print(" - Distancia em centimetros: "); //print da distancia lida em centimetros
    Serial.print(cmMsec); //print da distancia lida em centimetros
    Serial.println(); //quebra de linha

    //logica do balizamento
    if (cmMsec > 20) { //se a leitura for maior do que 20 centimetros deve ligar o led vermelho e desligar o led verde caso esteja ligado
      digitalWrite(bal_vermelho, HIGH);
      digitalWrite(bal_verde, LOW);
      Serial.print("  Muito Distante"); //apresenta mensagem na tela de que distância é muito grande.
      Serial.println();
    }

    
    if (cmMsec < 8 ) { //se a distancia for menor do que 8 centimetros luz vermelha deve permancer ligada
      digitalWrite(bal_vermelho, HIGH); 
      digitalWrite(bal_verde, LOW);
      Serial.print("  Muito próximo do sensor"); //apresenta mensagem na tela de que distância é muito próxima do sensor
      Serial.println();
    }

      if ((cmMsec > 15) && (cmMsec <= 20)){  //distancia entre 15 e 20 luz vermelha deve piscar e informar na tela para que se aproxime devagar
      digitalWrite(bal_verde, LOW);
      digitalWrite(bal_vermelho, HIGH);
      delay(300);
      digitalWrite(bal_vermelho, LOW);
      delay(300);
      Serial.print("  Apróxime-se devagar");
      Serial.println();
    }

    
    if ((cmMsec > 8) && (cmMsec <= 12)){ //distancia entre 8 e 12 luz vermelha deve piscar e apresentar mensagem para se afastar devagar
      digitalWrite(bal_verde, LOW);
      digitalWrite(bal_vermelho, HIGH);
      delay(300);
      digitalWrite(bal_vermelho, LOW);
      delay(300);
      Serial.print("  Afaste-se devagar");
      Serial.println();
    }
   
    if ((cmMsec > 12) && (cmMsec <= 15)){  //distancia entre 12 e 15 luz verde deve permancer acesa e
      digitalWrite(bal_verde, HIGH);
      digitalWrite(bal_vermelho, LOW);
      Serial.print("  PARE! A posição está correta!"); //informa na tela que a distância está correta
      Serial.println();
    } 

    // Verifica se é o momento de enviar dados para o ThingSpeak 
    if( millis() - last_connection_time > INTERVALO_ENVIO_THINGSPEAK )
    {
        distancia_lida = DistM; //informa variavel que será enviada pela API
        luminosidade_lida = lumi; //informa variavel que será enviada pela API
        sprintf(fields_a_serem_enviados,"field1=%.2f&field2=%.2f&field3=%.2f", distancia_lida, luminosidade_lida); //informa a variavel que será enviada pela API
        envia_informacoes_thingspeak(fields_a_serem_enviados); //envia as informações pela a API
    }
 
    delay(1000);
}


void handleRoot() { //função apos conexao bem sucessida do cliente

  float cmMsec;
  float DistM;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  digitalWrite(led, 1);
  String textoHTML;

  textoHTML = "Resistencia do LDR: ";
  textoHTML += analogRead(A0);
  textoHTML += "";
  textoHTML += " A distancia medida e:  , metros";
  textoHTML += (cmMsec);
     
  server.send(200, "text/html", textoHTML);
  digitalWrite(led, 0);
}

void handleNotFound(){ //função apos conexao mal sucessida do cliente
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}


//funções auxiliares

//Função aeroporto não operando em IRF
void aer_irf_desligado(){
  digitalWrite(aer_vermelho, LOW);
  digitalWrite(aer_azul, LOW);
  digitalWrite(aer_verde, LOW);
  digitalWrite(pp_led_verm, LOW);
  }

//Função aeroporto operando em IRF
void aer_irf_ligado(){
  digitalWrite(aer_vermelho, 255);
  digitalWrite(aer_azul, 255);
  digitalWrite(aer_verde, 255);
  delay(100);
  digitalWrite(aer_vermelho, 255);
  digitalWrite(aer_azul, 0);
  digitalWrite(aer_verde, 0);
  }

//Função pista principal do aeroporto operando em IRF
void pp_irf(){ //principal em irf_vrf
  digitalWrite(pp_led_amb, HIGH);
  digitalWrite(pp_led_verm, LOW);
  delay(100);
  digitalWrite(pp_led_verm, HIGH);
  digitalWrite(pp_led_amb, LOW);
}

void envia_informacoes_thingspeak(String string_dados) //função de envio dos dados ao Thingspeak
{
    if (client.connect(endereco_api_thingspeak, 80))
    {
        /* faz a requisição HTTP ao ThingSpeak */
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+chave_escrita_thingspeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(string_dados.length());
        client.print("\n\n");
        client.print(string_dados);
         
        last_connection_time = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
        Serial.println();
    }
}

