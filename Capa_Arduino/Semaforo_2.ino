#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURACIÓN ---
const char* ssid = "INFINITUMC40D";
const char* password = "f7FZfdfe3M";

// Asignación de pines para los relevadores
#define PIN_LED_VERDE     21
#define PIN_LED_AMARILLO  22
#define PIN_LED_ROJO      23

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

// --- FUNCIONES DE CONTROL INDIVIDUAL ---
void handleLedOn(int pin) {
    // Apagar todos los LEDs primero para evitar conflictos
    // digitalWrite(PIN_LED_VERDE, LOW);
    // digitalWrite(PIN_LED_AMARILLO, LOW);
    // digitalWrite(PIN_LED_ROJO, LOW);

    // Encender el LED solicitado
    digitalWrite(pin, HIGH);
    rutinaActiva = false;
    estadoSemaforo = -1; // Estado manual
    server.send(200, "text/plain", "Encendido");
}

void handleLedOff(int pin) {
    digitalWrite(pin, LOW);
    server.send(200, "text/plain", "Apagado");
}

// --- Función para obtener estado individual de LED ---
void handleLedStatus(int pin) {
    if (digitalRead(pin) == HIGH) {
        server.send(200, "text/plain", "Encendido");
    } else {
        server.send(200, "text/plain", "Apagado");
    }
}

// --- FUNCIONES PARA LA RUTINA ---
void handleStartRoutine() {
    if (server.hasArg("verde")) tiempoVerde = server.arg("verde").toInt();
    if (server.hasArg("amarillo")) tiempoAmarillo = server.arg("amarillo").toInt();
    if (server.hasArg("rojo")) tiempoRojo = server.arg("rojo").toInt();

    // Apagar todos los LEDs antes de iniciar la rutina
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, LOW);

    rutinaActiva = true;
    estadoSemaforo = 0; // Empezamos desde rojo
    tiempoInicioEstado = millis();
    tiempoUltimoParpadeo = millis();

    Serial.println("Rutina iniciada - Estado: ROJO");
    server.send(200, "text/plain", "Rutina iniciada");
}

void handleStopRoutine() {
    rutinaActiva = false;
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, LOW);
    estadoSemaforo = -1; // Estado manual
    server.send(200, "text/plain", "Rutina detenida");
}

// --- ENDPOINT PARA OBTENER EL ESTADO ---
void handleGetStatus() {
    String status = "ninguno";

    if (rutinaActiva) {
        switch (estadoSemaforo) {
            case 0: status = "rojo"; break;
            case 1: status = "verde"; break;
            case 2: status = "verde_parpadeando"; break;
            case 3: status = "amarillo"; break;
        }
    } else {
        // Si no está en rutina, comprobamos cada LED individualmente
        if (digitalRead(PIN_LED_ROJO) == HIGH &&
            digitalRead(PIN_LED_AMARILLO) == HIGH &&
            digitalRead(PIN_LED_VERDE) == HIGH) {
            status = "todos";
        } else if (digitalRead(PIN_LED_VERDE) == HIGH) {
            status = "verde";
        } else if (digitalRead(PIN_LED_AMARILLO) == HIGH) {
            status = "amarillo";
        } else if (digitalRead(PIN_LED_ROJO) == HIGH) {
            status = "rojo";
        }
    }

    server.send(200, "text/plain", status);
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

    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, LOW);

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
    server.on("/led/verde/status", [](){ handleLedStatus(PIN_LED_VERDE); });

    server.on("/led/amarillo/on", [](){ handleLedOn(PIN_LED_AMARILLO); });
    server.on("/led/amarillo/off", [](){ handleLedOff(PIN_LED_AMARILLO); });
    server.on("/led/amarillo/status", [](){ handleLedStatus(PIN_LED_AMARILLO); });

    server.on("/led/rojo/on", [](){ handleLedOn(PIN_LED_ROJO); });
    server.on("/led/rojo/off", [](){ handleLedOff(PIN_LED_ROJO); });
    server.on("/led/rojo/status", [](){ handleLedStatus(PIN_LED_ROJO); });

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

        switch (estadoSemaforo) {
            case 0: // ROJO
                digitalWrite(PIN_LED_ROJO, HIGH);
                digitalWrite(PIN_LED_VERDE, LOW);
                digitalWrite(PIN_LED_AMARILLO, LOW);
                if (tiempoTranscurrido >= tiempoRojo) {
                    estadoSemaforo = 1;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a VERDE");
                }
                break;

            case 1: // VERDE FIJO
                digitalWrite(PIN_LED_VERDE, HIGH);
                digitalWrite(PIN_LED_AMARILLO, LOW);
                digitalWrite(PIN_LED_ROJO, LOW);
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
                }
                if (tiempoTranscurrido >= 3000) {
                    digitalWrite(PIN_LED_VERDE, LOW);
                    estadoSemaforo = 3;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a AMARILLO");
                }
                break;

            case 3: // AMARILLO
                digitalWrite(PIN_LED_AMARILLO, HIGH);
                digitalWrite(PIN_LED_VERDE, LOW);
                digitalWrite(PIN_LED_ROJO, LOW);
                if (tiempoTranscurrido >= tiempoAmarillo) {
                    digitalWrite(PIN_LED_AMARILLO, LOW);
                    estadoSemaforo = 0;
                    tiempoInicioEstado = tiempoActual;
                    Serial.println("Cambiando a ROJO");
                }
                break;
        }
    }
}
