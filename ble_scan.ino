// Bibliotecas para uso do bluetooth do ESP32
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Configurar o pino do servo motor
#define PIN_SERVO 13

// Endereço MAC da TAG que abre a porta
String tagAddress = "a1:2b:65:65:65:cd";
int minRSSI = -60;  // Nível do RSSI minimo para abrir a porta
int maxRSSI = -80;  // Nível do RSSI máximo antes de fechar após aberta a porta

// Configuração do Bluetooth
#define SCAN_TIME 3 // Tempo de escaneamento em segundos
BLEScan* pBLEScan;  // Ponteiro para o objeto do BLE scan

// Verifica se a porta está aberta (true) ou fechada (false)
bool doorStatus = false;

void setup(void) {
  Serial.begin(115200); // iniciar o monitor serial 
  while (!Serial);
    delay(10);

  // Configuração do servo (PWM)
  pinMode(PIN_SERVO, OUTPUT);

  // Configuração do bluetooth
  BLEDevice::init("");               // Inicializa o bluetooth
  pBLEScan = BLEDevice::getScan();   // Inicializa o objeto BLE scan
  pBLEScan->setActiveScan(true);     // Scan ativo é mais rápido, mas usa mais energia
  pBLEScan->setInterval(100);        // Determina o intervalo entre a busca para 100 ms
  pBLEScan->setWindow(99);           // Determina a duração da janela de busca para 99 ms

  Serial.println("Configuração completa!");
  delay(100);
}

void loop() {

  // Permite que usuário busque por outro endereço BLE
  String input = "";
  Serial.println("Enter the device address: ");

  // Verifica se há alguma entrada no Serial Monitor
  while (Serial.available() > 0){
    input = Serial.readString();  // Transforma a entrada em String
    input.trim();                 // Remove os espaços vazios

    if (input != ""){             // Verifica se há algum caracter na entrada
      tagAddress = input;
    }else{
      Serial.println(".");
      delay(100);
    }
  }

  // Busca por dispositivos BLE e armazena o resultados
  BLEScanResults* foundDevices = pBLEScan->start(SCAN_TIME);
  bool tagFound = false;

  int deviceRSSI = -100;

  Serial.printf("Devices found: %d\n", foundDevices->getCount());

  // Cria um loop para passar por todos dispositivos encontrados
  for (int i = 0; i < foundDevices->getCount(); i++) {
    // Usa a biblioteca para obter os dados do dispositivo
    BLEAdvertisedDevice device = foundDevices->getDevice(i);

    // Obtem o endereço e o RSSI do dispositivo
    String deviceAddress = device.getAddress().toString().c_str();
    deviceRSSI = device.getRSSI();

    // Mostra os dados dos dispositivos bluetooth
    Serial.printf("Device %d: Address: %s, RSSI: %d\n", i + 1, deviceAddress.c_str(), deviceRSSI);

    // Conferir o endereço do dispositivo é o mesmo da TAG e se RSSI é maior que o mínimo de ativação
    if (deviceAddress == tagAddress && deviceRSSI >= minRSSI) {
      Serial.println("TAG found!");
      tagFound = true;
      break;
    }
  }

  // Verifica se a tag foi encontrada e se a porta está fechada
  if (tagFound && !doorStatus) {
    door(1);
    doorStatus = true;
  
  // Verifica se a porta está aberta e se o RSSI da TAG é maior que o máximo 
  } else if (doorStatus && deviceRSSI <= maxRSSI) {
    door(0);
    doorStatus = false;
  }

  delay(100);
}

// Função para abrir ou fechar a porta
void door(int status) {
  if (status == 1) {
    Serial.println("Open door");
    servoAng(PIN_SERVO, 100);
    delay(5000);
  } else if (status == 0) {
    Serial.println("Close door");
    servoAng(PIN_SERVO, -100);
  }
}

// Função para mover o servo a partir da geração de sinais PWM
void servoAng(int pin, int ang){
  int pulse = map(ang, 0, 180, 1000, 2000);
  digitalWrite(pin, HIGH);
  delayMicroseconds(pulse);
  digitalWrite(pin, LOW);
  delay(20 - (pulse/1000));
}
