//Емкостной сенсор влажности почвы
// Enable debug prints
#define MY_DEBUG
#define DEBUG



// Enable passive mode
#define MY_PASSIVE_NODE

#define PIN_NRF_POWER     8
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_NODE_ID        99
#define MY_RF24_CE_PIN    9
#define MY_RF24_CS_PIN    10
//#define MY_RF24_POWER_PIN (PIN_NRF_POWER)
#define RF24_CHANNEL      76
//#define MY_RF24_DATARATE (RF24_1MBPS)
#define PIN_VCC           15
#define PIN_A             A0
#define PIN_LED           7

#define MY_RADIO_NRF24
//#define MY_RF24_PA_LEVEL (RF24_PA_MIN)

//#include <avr/wdt.h>
#include <MySensors.h>  

// Define sensor node childs
#define CHILD_ID_VCC     0
#define CHILD_ID_A0      1
#define CHILD_ID_TEMP    2
#define ERROR_VALUE -100
#define TIME_SLEEP       300000
#define VCC_MAX           3000
#define VCC_MIN           2500

MyMessage msgVcc(CHILD_ID_VCC, V_VOLTAGE);
MyMessage msgA0(CHILD_ID_A0, V_STATUS);
//MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);


int readVcc();
void _ReInit();

void(* resetFunc) (void) = 0; // Reset MC function 

void before(){
  pinMode(PIN_VCC,OUTPUT);
  digitalWrite(PIN_VCC,LOW);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,HIGH);

  pinMode(PIN_NRF_POWER,OUTPUT);
  digitalWrite(PIN_NRF_POWER,HIGH);
  delay(200);
  digitalWrite(PIN_LED,LOW);
//  _ReInit();
}

void setup()
{
#ifdef DEBUG  
   Serial.begin(115200);
   Serial.println("MySensor Module");
#endif   
//     for (uint16_t i=0; i<EEPROM_LOCAL_CONFIG_ADDRESS; i++) {
//      hwWriteConfig(i,0xFF);
//     }
//    }
#ifdef DEBUG 
//   Serial.println("Clearing done.");
#endif
//  Serial.print("setupt ");
//  Serial.println(millis());
  
  
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
	sendSketchInfo("HSC1", "1.1");

	// Register binary input sensor to sensor_node (they will be created as child devices)
	// You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
	// If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
	present(CHILD_ID_VCC, S_BINARY,"V");
	present(CHILD_ID_A0, S_HUM,"");
#ifdef DEBUG
  Serial.print("present ");
  Serial.println(millis());
#endif  
}

// Loop will iterate on changes on the BUTTON_PINs
void loop(){
 
  _ReInit();
  int vcc = readVcc(); 
  long vcc_proc = 0;
  if( vcc > VCC_MAX )vcc_proc = 100;
  else if( vcc < VCC_MIN )vcc_proc = 0;
  else vcc_proc = (((long)vcc-VCC_MIN)*100)/(VCC_MAX-VCC_MIN);
  
  uint16_t a0 = AnalogMeasure();
  
  sendBatteryLevel(vcc_proc);
  send(msgA0.set(a0));
  send(msgVcc.set(vcc));
#ifdef DEBUG  
  Serial.print("a0=");
  Serial.print(a0);
  Serial.print(" vcc=");
  Serial.println(vcc);
#endif    
  digitalWrite(PIN_LED,HIGH);
  delay(100);
  digitalWrite(PIN_LED,LOW);
  sleep( TIME_SLEEP );
}


/**
 * Считываем напряжение питания
 */
int readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (1100L * 1023)/result;
  return((int)result);
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


uint16_t AnalogMeasure(){
   uint32_t a0 = 0;
   uint16_t n  = 0;
   analogReference(DEFAULT);
   digitalWrite(PIN_VCC,HIGH);
   delay(300);
   for( int i=0; i<5; i++){
      uint16_t a = analogRead(PIN_A);
//      Serial.println(a);
      a0+=a;
      n++;
      delay(1);    
   }
   digitalWrite(PIN_VCC,LOW);
 
   a0 /= n;
   return((uint16_t)a0);  
}
