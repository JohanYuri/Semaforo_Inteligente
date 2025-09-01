// #include <WiFi.h>
// #include <WebServer.h>
// #include <ArduinoJson.h>

// // --- CONFIGURACIÓN ---
// // Sustituye con las credenciales de tu red WiFi
// const char* ssid = "INFINITUMC40D";
// const char* password = "f7FZfdfe3M";

// // Asignación de pines para los relevadores
// #define PIN_LED_VERDE     21
// #define PIN_LED_AMARILLO  22
// #define PIN_LED_ROJO      23

// // Crear servidor web en el puerto 80
// WebServer server(80);

// // --- Variables para la rutina del semáforo ---
// bool rutinaActiva = false;
// unsigned long tiempoVerde = 5000;
// unsigned long tiempoAmarillo = 2000;
// unsigned long tiempoRojo = 8000;

// unsigned long tiempoInicioEstado = 0;
// unsigned long tiempoUltimoParpadeo = 0;
// bool estadoLedParpadeo = false;
// const unsigned long intervaloParpadeo = 500; // 500ms para parpadeo

// // ESTADOS: 0: Rojo, 1: Verde Fijo, 2: Verde Parpadeando, 3: Amarillo
// int estadoSemaforo = 0;

// // --- FUNCIONES DE CONTROL INDIVIDUAL ---
// void turnAllOff() {
//   digitalWrite(PIN_LED_VERDE, LOW);
//   digitalWrite(PIN_LED_AMARILLO, LOW);
//   digitalWrite(PIN_LED_ROJO, LOW);
// }

// void handleLedOn(int pin) {
//     turnAllOff();
//     digitalWrite(pin, HIGH);
//     rutinaActiva = false;
//     server.send(200, "text/plain", "Encendido");
// }

// // Nueva función para encender todos los LEDs a la vez
// void handleAllOn() {
//     turnAllOff(); // Aseguramos que nada parpadee o esté en un estado intermedio
//     digitalWrite(PIN_LED_VERDE, HIGH);
//     digitalWrite(PIN_LED_AMARILLO, HIGH);
//     digitalWrite(PIN_LED_ROJO, HIGH);
//     rutinaActiva = false;
//     server.send(200, "text/plain", "Todos encendidos");
//     Serial.println("Comando manual: Encender todos los LEDs");
// }

// void handleLedOff(int pin) {
//     digitalWrite(pin, LOW);
//     rutinaActiva = false; // Detener la rutina si un LED se apaga manualmente
//     server.send(200, "text/plain", "Apagado");
// }

// // --- ENDPOINT PARA OBTENER EL ESTADO EN FORMATO JSON ---
// void handleGetStatus() {
//   StaticJsonDocument<200> doc;
  
//   doc["routine_active"] = rutinaActiva;
  
//   if (rutinaActiva) {
//     doc["blinking_green"] = (estadoSemaforo == 2);
//     doc["green"] = (estadoSemaforo == 1);
//     doc["yellow"] = (estadoSemaforo == 3);
//     doc["red"] = (estadoSemaforo == 0);
//   } else {
//     doc["blinking_green"] = false;
//     doc["green"] = digitalRead(PIN_LED_VERDE) == HIGH;
//     doc["yellow"] = digitalRead(PIN_LED_AMARILLO) == HIGH;
//     doc["red"] = digitalRead(PIN_LED_ROJO) == HIGH;
//   }
  
//   String jsonResponse;
//   serializeJson(doc, jsonResponse);
//   server.send(200, "application/json", jsonResponse);
// }

// // --- FUNCIONES PARA LA RUTINA ---
// void handleStartRoutine() {
//     if (server.hasArg("verde")) tiempoVerde = server.arg("verde").toInt();
//     if (server.hasArg("amarillo")) tiempoAmarillo = server.arg("amarillo").toInt();
//     if (server.hasArg("rojo")) tiempoRojo = server.arg("rojo").toInt();

//     turnAllOff();
//     rutinaActiva = true;
//     estadoSemaforo = 0; // Empezamos desde rojo
//     tiempoInicioEstado = millis();
//     tiempoUltimoParpadeo = millis();

//     Serial.println("Rutina iniciada - Estado: ROJO");
//     server.send(200, "text/plain", "Rutina iniciada");
// }

// void handleStopRoutine() {
//     rutinaActiva = false;
//     turnAllOff();
//     server.send(200, "text/plain", "Rutina detenida");
// }

// void handleNotFound() {
//     server.send(404, "text/plain", "No encontrado");
// }

// // --- SETUP ---
// void setup() {
//     Serial.begin(115200);

//     pinMode(PIN_LED_VERDE, OUTPUT);
//     pinMode(PIN_LED_AMARILLO, OUTPUT);
//     pinMode(PIN_LED_ROJO, OUTPUT);

//     turnAllOff();

//     WiFi.begin(ssid, password);
//     Serial.print("Conectando a WiFi");
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println("\n¡Conectado!");
//     Serial.print("Dirección IP: ");
//     Serial.println(WiFi.localIP());

//     // Rutas de LEDs
//     server.on("/led/verde/on", [](){ handleLedOn(PIN_LED_VERDE); });
//     server.on("/led/verde/off", [](){ handleLedOff(PIN_LED_VERDE); });
//     server.on("/led/amarillo/on", [](){ handleLedOn(PIN_LED_AMARILLO); });
//     server.on("/led/amarillo/off", [](){ handleLedOff(PIN_LED_AMARILLO); });
//     server.on("/led/rojo/on", [](){ handleLedOn(PIN_LED_ROJO); });
//     server.on("/led/rojo/off", [](){ handleLedOff(PIN_LED_ROJO); });
//     server.on("/led/all/on", [](){ handleAllOn();});

//     // Rutas rutina y estado
//     server.on("/rutina/start", handleStartRoutine);
//     server.on("/rutina/stop", handleStopRoutine);
//     server.on("/status", handleGetStatus);
//     server.onNotFound(handleNotFound);

//     server.begin();
//     Serial.println("Servidor HTTP iniciado");
// }

// // --- LOOP ---
// void loop() {
//     server.handleClient();

//     if (rutinaActiva) {
//         unsigned long tiempoActual = millis();
//         unsigned long tiempoTranscurrido = tiempoActual - tiempoInicioEstado;

//         switch (estadoSemaforo) {
//             case 0: // ROJO
//                 digitalWrite(PIN_LED_ROJO, HIGH);
//                 digitalWrite(PIN_LED_VERDE, LOW);
//                 digitalWrite(PIN_LED_AMARILLO, LOW);
//                 if (tiempoTranscurrido >= tiempoRojo) {
//                     estadoSemaforo = 1;
//                     tiempoInicioEstado = tiempoActual;
//                     Serial.println("Cambiando a VERDE");
//                 }
//                 break;

//             case 1: // VERDE FIJO
//                 digitalWrite(PIN_LED_VERDE, HIGH);
//                 digitalWrite(PIN_LED_AMARILLO, LOW);
//                 digitalWrite(PIN_LED_ROJO, LOW);
//                 if (tiempoTranscurrido >= tiempoVerde) {
//                     estadoSemaforo = 2;
//                     tiempoInicioEstado = tiempoActual;
//                     tiempoUltimoParpadeo = tiempoActual;
//                     Serial.println("Cambiando a VERDE PARPADEANDO");
//                 }
//                 break;

//             case 2: // VERDE PARPADEANDO
//                 digitalWrite(PIN_LED_AMARILLO, LOW); // Nos aseguramos que el resto de LEDs estén apagados
//                 digitalWrite(PIN_LED_ROJO, LOW);
//                 if (tiempoActual - tiempoUltimoParpadeo >= intervaloParpadeo) {
//                     tiempoUltimoParpadeo = tiempoActual;
//                     estadoLedParpadeo = !estadoLedParpadeo;
//                     digitalWrite(PIN_LED_VERDE, estadoLedParpadeo);
//                 }
//                 if (tiempoTranscurrido >= 3000) { // El parpadeo dura 3 segundos
//                     digitalWrite(PIN_LED_VERDE, LOW);
//                     estadoSemaforo = 3;
//                     tiempoInicioEstado = tiempoActual;
//                     Serial.println("Cambiando a AMARILLO");
//                 }
//                 break;

//             case 3: // AMARILLO
//                 digitalWrite(PIN_LED_AMARILLO, HIGH);
//                 digitalWrite(PIN_LED_VERDE, LOW);
//                 digitalWrite(PIN_LED_ROJO, LOW);
//                 if (tiempoTranscurrido >= tiempoAmarillo) {
//                     digitalWrite(PIN_LED_AMARILLO, LOW);
//                     estadoSemaforo = 0;
//                     tiempoInicioEstado = tiempoActual;
//                     Serial.println("Cambiando a ROJO");
//                 }
//                 break;
//         }
//     }
// }
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --- CONFIGURACIÓN ---
// Sustituye con las credenciales de tu red WiFi
const char* ssid = "INFINITUMC40D";
const char* password = "f7FZfdfe3M";

// Asignación de pines para los relevadores
#define PIN_LED_VERDE 21
#define PIN_LED_AMARILLO 22
#define PIN_LED_ROJO 23

// Crear servidor web en el puerto 80
WebServer server(80);

// --- Variables para la rutina del semáforo ---
bool rutinaActiva = false;
unsigned long tiempoVerde = 5000;
unsigned long tiempoAmarillo = 2000;
unsigned long tiempoRojo = 8000;

unsigned long tiempoInicioEstado = 0;
unsigned long tiempoUltimoParpadeo = 0;
bool estadoLedParpadeo = false;
const unsigned long intervaloParpadeo = 500; // 500ms para parpadeo

// ESTADOS: 0: Rojo, 1: Verde Fijo, 2: Verde Parpadeando, 3: Amarillo
int estadoSemaforo = 0;

// Variables globales para rastrear el estado de cada pin (solución al problema)
// La posición en el array corresponde al pin: 0: Verde, 1: Amarillo, 2: Rojo
bool pinStates[] = {false, false, false};
const int pinMap[] = {PIN_LED_VERDE, PIN_LED_AMARILLO, PIN_LED_ROJO};

// --- FUNCIONES DE CONTROL INDIVIDUAL ---
void turnAllOff() {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, LOW);
    pinStates[0] = false;
    pinStates[1] = false;
    pinStates[2] = false;
}

void handleLedOn(int pin) {
    turnAllOff();
    digitalWrite(pin, HIGH);
    if (pin == PIN_LED_VERDE) pinStates[0] = true;
    if (pin == PIN_LED_AMARILLO) pinStates[1] = true;
    if (pin == PIN_LED_ROJO) pinStates[2] = true;

    Serial.printf("Comando manual: Encendiendo pin %d\n", pin);
    rutinaActiva = false;
    server.send(200, "text/plain", "Encendido");
}

// Nueva función para encender todos los LEDs a la vez
void handleAllOn() {
    turnAllOff();
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_AMARILLO, HIGH);
    digitalWrite(PIN_LED_ROJO, HIGH);
    pinStates[0] = true;
    pinStates[1] = true;
    pinStates[2] = true;
    rutinaActiva = false;
    server.send(200, "text/plain", "Todos encendidos");
    Serial.println("Comando manual: Encender todos los LEDs");
}

void handleLedOff(int pin) {
    digitalWrite(pin, LOW);
    if (pin == PIN_LED_VERDE) pinStates[0] = false;
    if (pin == PIN_LED_AMARILLO) pinStates[1] = false;
    if (pin == PIN_LED_ROJO) pinStates[2] = false;
    rutinaActiva = false; // Detener la rutina si un LED se apaga manualmente
    server.send(200, "text/plain", "Apagado");
}

// --- ENDPOINT PARA OBTENER EL ESTADO EN FORMATO JSON ---
void handleGetStatus() {
    StaticJsonDocument<200> doc;

    doc["routine_active"] = rutinaActiva;

    if (rutinaActiva) {
        doc["blinking_green"] = (estadoSemaforo == 2);
        doc["green"] = (estadoSemaforo == 1 || estadoSemaforo == 2);
        doc["yellow"] = (estadoSemaforo == 3);
        doc["red"] = (estadoSemaforo == 0);
    } else {
        doc["blinking_green"] = false;
        doc["green"] = pinStates[0];
        doc["yellow"] = pinStates[1];
        doc["red"] = pinStates[2];
    }  
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

// --- FUNCIONES PARA LA RUTINA ---
void handleStartRoutine() {
    if (server.hasArg("verde")) tiempoVerde = server.arg("verde").toInt();
    if (server.hasArg("amarillo")) tiempoAmarillo = server.arg("amarillo").toInt();
    if (server.hasArg("rojo")) tiempoRojo = server.arg("rojo").toInt();

    turnAllOff();
    rutinaActiva = true;
    estadoSemaforo = 0; // Empezamos desde rojo
    tiempoInicioEstado = millis();
    tiempoUltimoParpadeo = millis();

    Serial.println("Rutina iniciada - Estado: ROJO");
    server.send(200, "text/plain", "Rutina iniciada");
}

void handleStopRoutine() {
    rutinaActiva = false;
    turnAllOff();
    server.send(200, "text/plain", "Rutina detenida");
}

void handleNotFound() {
    server.send(404, "text/plain", "No encontrado");
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED_VERDE, OUTPUT);
    pinMode(PIN_LED_AMARILLO, OUTPUT);
    pinMode(PIN_LED_ROJO, OUTPUT);

    turnAllOff();

    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n¡Conectado!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());

    // Rutas de LEDs
    server.on("/led/verde/on", [](){ handleLedOn(PIN_LED_VERDE); });
    server.on("/led/verde/off", [](){ handleLedOff(PIN_LED_VERDE); });
    server.on("/led/amarillo/on", [](){ handleLedOn(PIN_LED_AMARILLO); });
    server.on("/led/amarillo/off", [](){ handleLedOff(PIN_LED_AMARILLO); });
    server.on("/led/rojo/on", [](){ handleLedOn(PIN_LED_ROJO); });
    server.on("/led/rojo/off", [](){ handleLedOff(PIN_LED_ROJO); });
    server.on("/led/all/on", [](){ handleAllOn();});

    // Rutas rutina y estado
    server.on("/rutina/start", handleStartRoutine);
    server.on("/rutina/stop", handleStopRoutine);
    server.on("/status", handleGetStatus);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

// --- LOOP ---
void loop() {
    server.handleClient();

    if (rutinaActiva) {
        unsigned long tiempoActual = millis();
        unsigned long tiempoTranscurrido = tiempoActual - tiempoInicioEstado;

        // La rutina también debe actualizar las variables de estado
        turnAllOff(); // Apagar todos para evitar solapamientos y encender solo el actual
        switch (estadoSemaforo) {
            case 0: // ROJO
                digitalWrite(PIN_LED_ROJO, HIGH);
                pinStates[2] = true;
                if (tiempoTranscurrido >= tiempoRojo) {
                    estadoSemaforo = 1;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a VERDE");
                }
                break;

            case 1: // VERDE FIJO
                digitalWrite(PIN_LED_VERDE, HIGH);
                pinStates[0] = true;
                if (tiempoTranscurrido >= tiempoVerde) {
                    estadoSemaforo = 2;
                    tiempoInicioEstado = tiempoActual;
                    tiempoUltimoParpadeo = tiempoActual;
                    Serial.println("Cambiando a VERDE PARPADEANDO");
                }
                break;

            case 2: // VERDE PARPADEANDO
                if (tiempoActual - tiempoUltimoParpadeo >= intervaloParpadeo) {
                    tiempoUltimoParpadeo = tiempoActual;
                    estadoLedParpadeo = !estadoLedParpadeo;
                    digitalWrite(PIN_LED_VERDE, estadoLedParpadeo);
                    pinStates[0] = estadoLedParpadeo;
                }
                if (tiempoTranscurrido >= 3000) { // El parpadeo dura 3 segundos
                    digitalWrite(PIN_LED_VERDE, LOW);
                    pinStates[0] = false;
                    estadoSemaforo = 3;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a AMARILLO");
                }
                break;

            case 3: // AMARILLO
                digitalWrite(PIN_LED_AMARILLO, HIGH);
                pinStates[1] = true;
                if (tiempoTranscurrido >= tiempoAmarillo) {
                    estadoSemaforo = 0;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a ROJO");
                }
                break;
        }
    }
}