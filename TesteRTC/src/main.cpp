#include <WiFi.h>
#include <NTPClient.h>
#include <time.h> 

// --- Configurações do Wi-Fi ---
const char* ssid = "...";         // <-- MUDAR
const char* password = "...";    // <-- MUDAR
const long timezoneOffset = -10800; // -3 horas (Brasil) = -10800 segundos

// Um cliente UDP para o NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timezoneOffset, 60000); 

bool isTimeSynced = false; // Flag para rastrear se a hora foi sincronizada (online)

// -----------------------------------------------------

// Função para configurar o RTC interno
void setupTimezone() {
    // Configura o fuso horário e inicia o serviço de tempo com o NTP
    // O ESP32 usa as bibliotecas de tempo C (time.h) para manter o relógio
    configTime(timezoneOffset, 0, "pool.ntp.org", "time.nist.gov");
}

// Função para exibir a hora atual
void printLocalTime() {
    struct tm timeinfo;
    
    // Obter a hora do sistema (que é mantida pelo RTC interno)
    if(!getLocalTime(&timeinfo)){
        Serial.println("Falha ao obter a hora (RTC interno indisponível ou não configurado).");
        return;
    }
    
    char timeString[50];
    strftime(timeString, sizeof(timeString), "%H:%M:%S %d/%b/%Y (%a)", &timeinfo);
    Serial.print("Hora Atual: ");
    Serial.println(timeString);
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- Iniciando Projeto RTC/WiFi ---");
    setupTimezone(); // Inicializa o fuso horário.

    // 1. Tentar conectar ao WiFi
    Serial.print("Conectando-se ao WiFi...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    // 2. Tentar sincronizar a hora via NTP
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado ao WiFi.");
        timeClient.begin();
        
        Serial.print("Sincronizando hora via NTP...");
        if(timeClient.forceUpdate()) {
            Serial.println("\nSincronização NTP bem-sucedida.");
            // O comando forceUpdate() já atualiza a hora do sistema (RTC interno)
            isTimeSynced = true;
        } else {
            Serial.println("\nFalha na sincronização NTP.");
        }
    } else {
        Serial.println("\nFalha na conexão WiFi.");
    }
    
    Serial.println("--- Status Inicial do Setup ---");
    if (isTimeSynced) {
        Serial.println("Modo Inicial: ONLINE (Hora sincronizada via NTP)");
    } else {
        // Se falhou na conexão ou sincronização, assume que está offline.
        // Se a hora estava salva do último boot, ela será usada aqui.
        Serial.println("Modo Inicial: OFFLINE (Tentando usar a última hora do RTC)");
    }
    printLocalTime();
}

void loop() {
    delay(5000); // Espera 5 segundos

    Serial.println("---");
    
    // VERIFICAÇÃO DE MODO
    if (WiFi.status() == WL_CONNECTED) {
        // Se está conectado, tenta atualizar a hora no loop (opcional)
        timeClient.update(); 
        Serial.println("Modo: ONLINE (Atualizado via NTP)");
    } else {
        Serial.println("Modo: OFFLINE (Puxando do RTC interno)");
    }
    
    printLocalTime();
}