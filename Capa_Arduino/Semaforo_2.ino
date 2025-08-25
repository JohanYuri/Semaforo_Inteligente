#include <WiFi.h>
#include <WebServer.h>

// Configuración de red Wi-Fi
// const char *ssid = "BUAP_Trabajadores";  "Mega_2.4G_5DCF";     "Pixel 6a Johan";
// const char *password = "BuaPW0rk.2017";  "fz3EGs2f";             "johanyuri123";
// Configuración de red Wi-Fi para WOKWI
const char *ssid = "Pixel 6a Johan";
const char *password = "johanyuri123"; // Sin contraseña

// Pines para el SEMÁFORO 1 (LEDs directos)
#define PIN_ROJO_1       5    // Pin para LED Rojo directo
#define PIN_AMARILLO_1   19   // Pin para LED Amarillo directo  
#define PIN_VERDE_1      18   // Pin para LED Verde directo

// Pines para el SEMÁFORO 2 (con relés)
#define PIN_ROJO_2       23   // Pin para relé Rojo
#define PIN_AMARILLO_2   22   // Pin para relé Amarillo
#define PIN_VERDE_2      21   // Pin para relé Verde

// Tiempos de duración (milisegundos)
#define TIEMPO_VERDE     5000   // 5 segundos
#define TIEMPO_AMARILLO  1500   // 1.5 segundos
#define TIEMPO_ROJO      5000   // 5 segundos

WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Configurar pines como salidas - SEMÁFORO 1 (LEDs directos)
  pinMode(PIN_ROJO_1, OUTPUT);
  pinMode(PIN_AMARILLO_1, OUTPUT);
  pinMode(PIN_VERDE_1, OUTPUT);
  
  // Configurar pines como salidas - SEMÁFORO 2 (con relés)
  pinMode(PIN_ROJO_2, OUTPUT);
  pinMode(PIN_AMARILLO_2, OUTPUT);
  pinMode(PIN_VERDE_2, OUTPUT);

  // Iniciar apagados
  apagarTodos();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  server.begin();
}

void loop() {
  server.handleClient();
  ejecutarSecuenciaSemaforos();
}

void ejecutarSecuenciaSemaforos() {
  // FASE 1: Semáforo 1 en VERDE, Semáforo 2 en ROJO
  digitalWrite(PIN_VERDE_1, HIGH);    // LED Verde directo ON
  digitalWrite(PIN_ROJO_2, HIGH);     // Relé Rojo ON (bombilla roja)
  delay(TIEMPO_VERDE);
  
  // Semáforo 1 parpadea verde (precaución)
  for(int i = 0; i < 3; i++) {
    digitalWrite(PIN_VERDE_1, LOW);
    delay(500);
    digitalWrite(PIN_VERDE_1, HIGH);
    delay(500);
  }
  
  // FASE 2: Semáforo 1 en AMARILLO, Semáforo 2 sigue en ROJO
  digitalWrite(PIN_VERDE_1, LOW);
  digitalWrite(PIN_AMARILLO_1, HIGH); // LED Amarillo directo ON
  delay(TIEMPO_AMARILLO);
  
  // FASE 3: Semáforo 1 en ROJO, Semáforo 2 en VERDE
  digitalWrite(PIN_AMARILLO_1, LOW);
  digitalWrite(PIN_ROJO_1, HIGH);     // LED Rojo directo ON
  digitalWrite(PIN_ROJO_2, LOW);      // Relé Rojo OFF
  digitalWrite(PIN_VERDE_2, HIGH);    // Relé Verde ON (bombilla verde)
  delay(TIEMPO_VERDE);
  
  // Semáforo 2 parpadea verde (precaución)
  for(int i = 0; i < 3; i++) {
    digitalWrite(PIN_VERDE_2, LOW);
    delay(500);
    digitalWrite(PIN_VERDE_2, HIGH);
    delay(500);
  }
  
  // FASE 4: Semáforo 2 en AMARILLO, Semáforo 1 sigue en ROJO
  digitalWrite(PIN_VERDE_2, LOW);
  digitalWrite(PIN_AMARILLO_2, HIGH); // Relé Amarillo ON (bombilla amarilla)
  delay(TIEMPO_AMARILLO);
  
  // Preparar siguiente ciclo
  digitalWrite(PIN_AMARILLO_2, LOW);
  digitalWrite(PIN_ROJO_1, LOW);
}

void apagarTodos() {
  // Semáforo 1 (LEDs directos)
  digitalWrite(PIN_ROJO_1, LOW);
  digitalWrite(PIN_AMARILLO_1, LOW);
  digitalWrite(PIN_VERDE_1, LOW);
  
  // Semáforo 2 (con relés)
  digitalWrite(PIN_ROJO_2, LOW);
  digitalWrite(PIN_AMARILLO_2, LOW);
  digitalWrite(PIN_VERDE_2, LOW);
}