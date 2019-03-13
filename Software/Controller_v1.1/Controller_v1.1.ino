// (С) MyHome 2019
// Тестовый скетч контроллера MyHome Controller
// Мигает светодиодами на всех портах и каждые 30 сек посылоет сведение о собственной батарее 
#define MY_DEBUG
#define DEBUG
//#define MY_DISABLED_SERIAL

// Enable passive mode
#define MY_PASSIVE_NODE

#define PIN_NRF_POWER     8
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_NODE_ID        250
#define MY_RF24_CE_PIN    9
#define MY_RF24_CS_PIN    10

#define MY_RF24_POWER_PIN (PIN_NRF_POWER)
#define RF24_CHANNEL      76
//#define MY_RF24_DATARATE (RF24_1MBPS)

#define MY_RADIO_NRF24
//#define MY_RF24_PA_LEVEL (RF24_PA_MAX)

#define PIN_LED1 2
#define PIN_LED2 7
#define PIN_LED3 14
#define PIN_LED4 19
#include <MySensors.h>  

// Define sensor node childs
#define CHILD_ID_VCC     0
#define CHILD_ID_A0      1
#define ERROR_VALUE -100
#define TIME_SLEEP       300000

uint8_t PINS1[] = {2,3,4,5,6,7,24,20,21};
uint8_t PINS2[] = {14,15,16,17,18,19,25,26,23};

MyMessage msgVcc(CHILD_ID_VCC, V_VOLTAGE);

//uint8_t st = 255;

float readVcc();

void _ReInit();
void(* resetFunc) (void) = 0; // Reset MC function 

void before(){
// Проверка светодиодов на плате питания
  for( int i=0; i<9; i++){
    pinMode(PINS1[i],OUTPUT);
    pinMode(PINS2[i],OUTPUT);
  }
  for( int i=0; i<3; i++){
     for(int j=0; j<9; j++){
        digitalWrite(PINS1[j],HIGH);  
        digitalWrite(PINS2[j],HIGH);         
     }
     delay(200);
     for(int j=0; j<9; j++){
        digitalWrite(PINS1[j],LOW);  
        digitalWrite(PINS2[j],LOW);         
     }
     delay(200);
  }

  
  pinMode(PIN_NRF_POWER,OUTPUT);
  digitalWrite(PIN_NRF_POWER,HIGH);
//  delay(200);
//     for (uint16_t i=0; i<EEPROM_LOCAL_CONFIG_ADDRESS; i++) {
//      hwWriteConfig(i,0xFF);
//     }
  
}

void setup(){
}

void presentation()
{
	sendSketchInfo("TEST1", "1.1");
	present(CHILD_ID_VCC, S_BINARY,"V");
}

uint32_t ms1 =0, ms2 = 0;
int n = 0;

void loop(){
  uint32_t ms = millis();
  if( ms1 == 0 || ( ms - ms1 )>200 ){
     ms1 = ms;
     digitalWrite(PINS1[n],LOW);
     digitalWrite(PINS2[n],LOW);
     n++;if(n >= 9 )n = 0;
     digitalWrite(PINS1[n],HIGH);
     digitalWrite(PINS2[n],HIGH);
     
  }

  if( ms2 == 0 || ( ms - ms2 )>30000 ){
     ms2 = ms;
     float vcc = readVcc();
     send(msgVcc.set(vcc,2));
     
  }


}


/**
 * Считываем напряжение питания
 */
float readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (1100L * 1023)/result;
  float ret = ((float)result)/1000;
  return ret;
}

/**
 * Реинициализация NRF-ки (выдрано из библиотеки)
 * Без него не выходит из режима низкого энергопотребления
 */
void _ReInit(){
    // Save static parent ID in eeprom (used by bootloader)
  hwWriteConfig(EEPROM_PARENT_NODE_ID_ADDRESS, MY_PARENT_NODE_ID);
  // Initialise transport layer
  transportInitialise();
  // Register transport=ready callback
  transportRegisterReadyCallback(_callbackTransportReady);
  // wait until transport is ready
  (void)transportWaitUntilReady(MY_TRANSPORT_WAIT_READY_MS);
}
