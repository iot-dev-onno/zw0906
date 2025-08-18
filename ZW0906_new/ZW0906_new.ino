// tests done in 906 

// HOW TO REGISTER NEW FINGERPRINTS? DEFINE A MVP

                        // TYPE        // FUNCTION
uint8_t ledPin = PA8;   // d output    // to led    
uint8_t INT1 = PB5;     // d input     // for reed switch / fingerprint sensor interrupt
uint8_t INT2 = PB12;    // d input     // for opto
uint8_t on_off = PA15;  // d output    // to gps module input 
uint8_t gps_v_en = PA0; // d output    // to mosfet gate / to turn off fingerprint sensor to achieve low power
uint8_t vbat_en = PA1;  // d output    // to bjt base
uint8_t adc4 = PA10;    // a input     // for battery voltage divider 
uint8_t adc1 = PB3;     // a input     // for analog sensor
uint8_t sen_v_en = PA9; // d output    // to mosfet gate 


// nuevas variables
uint8_t count_int = 0;
volatile uint8_t presscount = 0;
volatile uint32_t lastActivityMs = 0;   // tiempo del último press (ms)


const uint8_t HEADER_HIGH = 0xEF;
const uint8_t HEADER_LOW = 0x01;
const uint32_t DEVICE_ADDRESS = 0xFFFFFFFF;


// Define command codes 
const uint8_t CMD_GET_IMAGE = 0x01;  // Capture image
const uint8_t CMD_GEN_CHAR = 0x02;   // Generate feature,               Table 3-7 Generate feature instruction package format
const uint8_t CMD_MATCH = 0x03;   // Precise fingerprint match,         Table 3-9 Accurately compare two fingerprint feature instruction package formats
const uint8_t CMD_SEARCH = 0x04;   // Search fingerprint,               Table 3-11 Search fingerprint instruction packet format
const uint8_t CMD_REG_MODEL = 0x05;  // Merge features                  Table 3-13 Merge feature command packet format
const uint8_t CMD_STORE_CHAR = 0x06; // Store template                  Table 3-15 Save template command package format
const uint8_t CMD_CLEAR_LIB = 0x0D;   // Clear fingerprint library      Table 3-27 Clear fingerprint library command package format
const uint8_t CMD_READ_SYSPARA = 0x0F; // Read module basic parameters  

const uint8_t CMD_PS_ControlBLN = 0x3C; // Led control light, add page and document number
//function code
//Dec 3 normally open
//Dec 4 normally close

//starting color
//Bin 0rgb
//red: Bin 0100, green: Bin 0010 ,blue: Bin 0001

//endindg color
//Bin 0rgb
//red: Bin 0100, green: Bin 0010 ,blue: Bin 0001

//cycle
//not applicable with normally open/close function





// define buffer id
uint8_t BUFFER_ID = 0;

// define template storage location
const uint16_t TEMPLATE_ID = 1;

// global variables
bool isInit = false;
//bool press = false;
volatile bool press = false;

struct values {
  int confirmation;
  int page;
  int score;
} response_codes;

void blink1()
{
    press = true;
    if (presscount < 255){
      presscount ++;
    }
    else if (presscount>2){
      presscount = 0;
    }
    
}


// function declaration
// void command_use(void);
// int read_FP_info(void);
// void sendCommand(uint8_t cmd, uint8_t param1 = 0, uint16_t param2 = 0);
// void sendCommand_led(uint8_t cmd, uint8_t param1 = 0, uint8_t param2 = 0);
// void send_clear_cmd(uint8_t cmd);
// values sendCommand1(uint8_t cmd, uint8_t param1, uint16_t param2, uint16_t param3);

// bool receiveResponse();
// void printResponse(uint8_t *response, uint8_t length);

bool waitHoldRearm(uint8_t pin, uint32_t holdMs, uint32_t releaseDebounceMs = 50);
bool match();  // <<--- NUEVA
// --- PROTOTIPOS DE UTILIDADES (deben ir antes de usarlas)
inline void drainSerial1Rx(uint32_t idle_us = 2000);
bool readFpPacket(uint8_t *buf, size_t bufmax, size_t &outlen, uint32_t timeout_ms = 500);


void printHex(uint8_t* data, uint8_t len) {
  for(uint8_t i=0; i<len; i++){
    if(data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}




void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(INT1, INPUT);
  pinMode(INT2, INPUT);

  pinMode(on_off, OUTPUT);
  pinMode(gps_v_en, OUTPUT);
  pinMode(vbat_en, OUTPUT);
  pinMode(sen_v_en, OUTPUT);


  Serial.begin(57600);
  Serial1.begin(57600);

  delay(5000);

  digitalWrite(ledPin, HIGH);	// LED turn on when input pin value is LOW
  delay(200);
  digitalWrite(ledPin, LOW);	// LED turn off when input pin value is HIGH
  delay(200);

  digitalWrite(ledPin, HIGH);	// LED turn on when input pin value is LOW
  delay(200);
  digitalWrite(ledPin, LOW);	// LED turn off when input pin value is HIGH
  delay(200);

  digitalWrite(gps_v_en, LOW); // keep off (HIGH) for low power 
  digitalWrite(sen_v_en, LOW); // keep on for fingerprint sensor
  delay(500);
  
  attachInterrupt(INT1, blink1, RISING);
    


}
/*
void loop() {
//Serial.println(presscount);

// parte para tener el encabezado correcto
if (count_int<2){
  send_get_image_cmd(CMD_GET_IMAGE);
  send_cmd2(CMD_GEN_CHAR,0x06);
  response_codes = sendCommand1(CMD_SEARCH, 1, 1, 1);
  Serial.printf("confirmation code: %02X \r\n",response_codes.confirmation);
  Serial.printf("page number:       %02X \r\n",response_codes.page);
  Serial.printf("score:             %02X \r\n",response_codes.score);
  Serial.printf("\r\n");
  if(response_codes.confirmation == 0x09){
    sendCommand_led(CMD_PS_ControlBLN,0X03,B0100);
    sendCommand_led(CMD_PS_ControlBLN,0X03,0B000);
  };
  if(response_codes.confirmation == 0x00 && response_codes.score > 0x08){
    sendCommand_led(CMD_PS_ControlBLN,0X03,B0010);
    sendCommand_led(CMD_PS_ControlBLN,0X03,0B000);
  }
  count_int++;
};

/*
if(press){
  detachInterrupt(INT1);
  press = false;
  digitalWrite(gps_v_en, LOW); // keep off (HIGH) for low power 

  // falta asegurar el orden de los bytes
  // read_FP_info();

  delay(300);// sensor startup time;

  // sendCommand_led(CMD_PS_ControlBLN,0X03,B0001);
  // sendCommand_led(CMD_PS_ControlBLN,0X03,0B000);  


  send_get_image_cmd(CMD_GET_IMAGE);

  send_cmd2(CMD_GEN_CHAR,0x06);

  //search command, default values
  response_codes = sendCommand1(CMD_SEARCH, 1, 1, 1);

  Serial.printf("confirmation code: %02X \r\n",response_codes.confirmation);
  Serial.printf("page number:       %02X \r\n",response_codes.page);
  Serial.printf("score:             %02X \r\n",response_codes.score);
  Serial.printf("\r\n");


  if(response_codes.confirmation == 0x09){
    sendCommand_led(CMD_PS_ControlBLN,0X03,B0100);
    sendCommand_led(CMD_PS_ControlBLN,0X03,0B000);
  };

  if(response_codes.confirmation == 0x00 && response_codes.score > 0x08){
    sendCommand_led(CMD_PS_ControlBLN,0X03,B0010);
    sendCommand_led(CMD_PS_ControlBLN,0X03,0B000);


  };

  // digitalWrite(gps_v_en, HIGH); // keep off (HIGH) for low power 

  delay(50);
  attachInterrupt(INT1, blink1, RISING);

  }




// send_clear_cmd(CMD_CLEAR_LIB);
*/
// // register fingerprint

//   send_get_image_cmd(CMD_GET_IMAGE);
//   delay(100);
  
//   send_cmd2(CMD_GEN_CHAR,0x06);
//   delay(100);

//   merge_feature_cmd(CMD_REG_MODEL);
//   delay(100);

//   sendCommand(CMD_STORE_CHAR,6,3); // command store, buffer id, page id, juan 1,2. sam 3
//   delay(1000);



// //search fingerprint
//   send_get_image_cmd(CMD_GET_IMAGE); // do not printout the response
//   delay(50);

//   send_cmd2(CMD_GEN_CHAR,0x06);      // do not printout the response
//   delay(50);

//   //search command, default values

//   sendCommand1(CMD_SEARCH, 1, 1, 1); // printout the response
//   delay(50);


//
/*
static bool busy = false;
const uint32_t HOLD_MATCH_MS = 1000;
const uint32_t HOLD_ENROLL_MS = 3000;

uint8_t pc;
pc = presscount;
*/

/*
if (!busy && pc >= 2 && waitHoldRearm(INT1, HOLD_ENROLL_MS)) {
  busy = true;

  detachInterrupt(INT1);
  digitalWrite(gps_v_en, LOW);  // LOW FOR POWER UP THE MODULE
  delay(300);
  enrroll();
  //resetPressCounter();  
  presscount = 0;
  press = false;
  digitalWrite(gps_v_en, HIGH); // keep off (HIGH) for low power 
  delay(50);
  attachInterrupt(INT1, blink1, RISING);
  busy = false;
  delay(300);
}
// Luego MATCH (exactamente 1 presión y 1 s)
else if (!busy && pc == 1 && waitHoldRearm(INT1, HOLD_MATCH_MS)) {
  busy = true;

  detachInterrupt(INT1);
  digitalWrite(gps_v_en, LOW);
  delay(300);
  send_get_image_cmd(CMD_GET_IMAGE);
  send_cmd2(CMD_GEN_CHAR, 0x06);
  response_codes = sendCommand1(CMD_SEARCH, 1, 1, 1);

  Serial.printf("confirmation code: %02X \r\n", response_codes.confirmation);
  Serial.printf("page number:       %02X \r\n", response_codes.page);
  Serial.printf("score:             %02X \r\n", response_codes.score);
  Serial.printf("\r\n");

  // usa literales binarios estándar (recomendado):
  if (response_codes.confirmation == 0x09) {
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0100);
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
  }
  if (response_codes.confirmation == 0x00 && response_codes.score > 0x08) {
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0010);
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
  }

  // reset contador
  //resetPressCounter();
  presscount = 0;
  press = false;
  digitalWrite(gps_v_en, HIGH); // keep off (HIGH) for low power 
  delay(50);
  attachInterrupt(INT1, blink1, RISING);
  busy = false;
  delay(300);
} *//*
// --- PRIMERO: MATCH (1 pulso + hold) ---
  if (!busy && pc <= 1 && waitHoldRearm(INT1, HOLD_MATCH_MS, 150)) {
    busy = true;

    detachInterrupt(INT1);
    digitalWrite(gps_v_en, LOW);
    delay(300);

    send_get_image_cmd(CMD_GET_IMAGE);
    send_cmd2(CMD_GEN_CHAR, 0x06);
    response_codes = sendCommand1(CMD_SEARCH, 1, 1, 1);

    Serial.printf("confirmation code: %02X \r\n", response_codes.confirmation);
    Serial.printf("page number:       %02X \r\n", response_codes.page);
    Serial.printf("score:             %02X \r\n", response_codes.score);
    Serial.printf("\r\n");

    if (response_codes.confirmation == 0x09) {
      sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0100);
      sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
    }
    if (response_codes.confirmation == 0x00 && response_codes.score > 0x08) {
      sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0010);
      sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
    }

    noInterrupts(); presscount = 0; press = false; interrupts();
    digitalWrite(gps_v_en, HIGH);
    delay(50);
    attachInterrupt(INT1, blink1, RISING);
    busy = false;
    delay(200);
  }
  // --- LUEGO: ENROLL (>=2 pulsos + hold) ---
  else if (!busy && pc >= 2 && waitHoldRearm(INT1, HOLD_ENROLL_MS, 150)) {
    busy = true;

    detachInterrupt(INT1);
    digitalWrite(gps_v_en, LOW);
    delay(300);
    enrroll();

    noInterrupts(); presscount = 0; press = false; interrupts();
    digitalWrite(gps_v_en, HIGH);
    delay(50);
    attachInterrupt(INT1, blink1, RISING);
    busy = false;
    delay(200);
    Serial.printf("confirmation code: %02X \r\n", response_codes.confirmation);

  }

//Serial.println(pc);

// termina loop 
}*/
void loop() {
  static bool busy = false;
  const uint32_t HOLD_MATCH_MS  = 1000;
  const uint32_t HOLD_ENROLL_MS = 3000;

  uint8_t pc = presscount;

  if (digitalRead(INT1) == HIGH) {
    lastActivityMs = millis();
  } else if (!busy && (millis() - lastActivityMs >= 2000)) {
    noInterrupts();
    presscount = 0;
    press = false;
    interrupts();
  }

  // --- PRIMERO: MATCH (>=1 pulso + hold) ---
  if (!busy && pc <= 1 && waitHoldRearm(INT1, HOLD_MATCH_MS, 150)) {
    busy = true;
    bool ok = match(); // <<--- aquí se hace todo el MATCH y LED
    noInterrupts(); presscount = 0; press = false; interrupts();
    busy = false;
    delay(200);
    Serial.print("Estoy terminando de hacer MATCH\r\n");
  }
  // --- LUEGO: ENROLL (>=2 pulsos + hold) ---
  else if (!busy && pc >= 2 && waitHoldRearm(INT1, HOLD_ENROLL_MS, 150)) {
    busy = true;
    detachInterrupt(INT1);
    digitalWrite(gps_v_en, LOW);  // power ON
    delay(300);
    enrroll();
    noInterrupts(); presscount = 0; press = false; interrupts();
    digitalWrite(gps_v_en, HIGH); // power OFF
    delay(50);
    attachInterrupt(INT1, blink1, RISING);
    busy = false;
    delay(200);
    Serial.printf("confirmation code: %02X \r\n", response_codes.confirmation);
    Serial.print("Estoy terminando de hacer enroll\r\n");
  }
  //Serial.println(pc);

}




int read_FP_info(void)
{
  uint8_t response[32];
  uint8_t index = 0;
  uint32_t startTime = millis();
  

  Serial.println();
  Serial.println("------------------------------------------");
  Serial.println("FPM info:");
  Serial1.flush();
  send_cmd(CMD_READ_SYSPARA);
  

  while (millis() - startTime < 500) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 28) break;
    }
  }

  printResponse(response, index);

  if (index >= 28){// && response[10] == 0x00) {
    uint16_t register_cnt = (uint16_t)((response[11]<<8) | response[12]); //    number of registration  00 05 
    uint16_t fp_temp_size = (uint16_t)((response[13]<<8) | response[14]); // fingerprint template size 20 00
    uint16_t fp_lib_size  = (uint16_t)((response[15]<<8) | response[16]); //fingerprint database size 00 64
    uint16_t score_level  = (uint16_t)((response[17]<<8) | response[18]); //Score level  00 03
    uint32_t device_addr  = (uint32_t)((response[19]<<24) | (response[20]<<16)| (response[21]<<8) | response[22]); //Device address  FF FF FF FF 
    uint16_t data_pack_size = (uint16_t)((response[23]<<8) | response[24]); //Packet size 00 03 

    if(     0 == data_pack_size) {
      data_pack_size = 32;
    }
    else if(1 == data_pack_size) {
      data_pack_size = 64;
    }
    else if(2 == data_pack_size) {
      data_pack_size = 128;
    }
    else if(3 == data_pack_size) {
      data_pack_size = 256;
    }
    uint16_t baud_set = (uint16_t)((response[25]<<8) | response[26]); //baud rate setting 00  06 
    
    Serial.print("register cnt:");
    Serial.println(register_cnt);
    Serial.print("temp size:0x");
    Serial.println(fp_temp_size,HEX);
    Serial.print("lib size:");
    Serial.println(fp_lib_size);
    Serial.print("level:");
    Serial.println(score_level);
    Serial.print("device address:0x");
    Serial.println(device_addr,HEX);
    Serial.print("data size:");
    Serial.println(data_pack_size);
    Serial.print("baud:");
    Serial.println(baud_set*9600);

    return 1;
  } else {
    return 0; 
  }
}








// Send command
// Send command (NO lee respuesta; usado por read_FP_info)
void send_cmd(uint8_t cmd) {
  drainSerial1Rx();                         // << nuevo

  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  packet[0] = HEADER_HIGH; packet[1] = HEADER_LOW;
  packet[2] = (DEVICE_ADDRESS >> 24) & 0xFF;
  packet[3] = (DEVICE_ADDRESS >> 16) & 0xFF;
  packet[4] = (DEVICE_ADDRESS >> 8)  & 0xFF;
  packet[5] =  DEVICE_ADDRESS        & 0xFF;
  packet[6] = 0x01;
  packet[7] = (length >> 8) & 0xFF;  packet[8] = length & 0xFF;
  packet[9] = cmd;
  packet[10] = (checksum >> 8) & 0xFF; packet[11] = checksum & 0xFF;

  for (int i = 0; i < 12; i++) Serial1.write(packet[i]);
}



// LED CONTROL
// sendCommand_led(CMD_PS_ControlBLN,0X03,0X07); checkout response lenght
// LED CONTROL
void sendCommand_led(uint8_t cmd, uint8_t param1, uint8_t param2) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[32]; size_t rlen=0;

  uint8_t packet[16];
  uint16_t length=7;
  uint16_t checksum =  1+length+cmd + param1 + param2 ;

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=param1;
  packet[11]=param2;
  packet[12]=0x00;  // end color
  packet[13]=0x00;  // cycles
  packet[14]=(checksum>>8)&0xFF; packet[15]=checksum&0xFF;

  for (int i=0;i<16;i++) Serial1.write(packet[i]);

  // leer ACK alineado (opcional imprimir)
  readFpPacket(response, sizeof(response), rlen, 300);
  // printResponse(response, (uint8_t)rlen);
}




// DELETE DATABASE
//send_clear_cmd(CMD_CLEAR_LIB); check for the byte that indicates an error
// DELETE DATABASE
void send_clear_cmd(uint8_t cmd) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[32]; size_t rlen=0;

  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=(checksum>>8)&0xFF; packet[11]=checksum&0xFF;

  for (int i=0;i<12;i++) Serial1.write(packet[i]);

  if (readFpPacket(response, sizeof(response), rlen, 1000)) {
    Serial.println("clear response:");
    printResponse(response, (uint8_t)rlen);
  }
}



// GET IMAGE
//send_get_image_cmd(CMD_GET_IMAGE); check for the byte about info return
// GET IMAGE
void send_get_image_cmd(uint8_t cmd) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[32]; size_t rlen=0;

  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=(checksum>>8)&0xFF; packet[11]=checksum&0xFF;

  for (int i=0;i<12;i++) Serial1.write(packet[i]);

  // ACK corto; no imprimimos para no saturar
  readFpPacket(response, sizeof(response), rlen, 300);
}




void send_cmd2(uint8_t cmd, uint8_t param1 ) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[32]; size_t rlen=0;

  uint8_t packet[13];
  uint16_t length=4;
  uint16_t checksum =  1+length+cmd + param1;

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=param1;
  packet[11]=(checksum>>8)&0xFF; packet[12]=checksum&0xFF;

  for (int i=0;i<13;i++) Serial1.write(packet[i]);

  readFpPacket(response, sizeof(response), rlen, 300);
  // printResponse(response, (uint8_t)rlen);
}



// REGISTER IMAGE
// merge_feature_cmd(CMD_REG_MODEL); 3.3.1.5 Merge feature PS_RegModel
// REGISTER IMAGE
void merge_feature_cmd(uint8_t cmd) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[32]; size_t rlen=0;

  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=(checksum>>8)&0xFF; packet[11]=checksum&0xFF;

  for (int i=0;i<12;i++) Serial1.write(packet[i]);

  if (readFpPacket(response, sizeof(response), rlen, 1000)) {
    Serial.println("merge response:");
    printResponse(response, (uint8_t)rlen);
  }
}


// SEND COMMAND 2 VARIABLES
// SEND COMMAND 2 VARIABLES
void sendCommand(uint8_t cmd, uint8_t param1, uint16_t param2) {
  drainSerial1Rx();                         // << nuevo

  uint8_t response[64]; size_t rlen=0;

  uint8_t packet[15];
  uint16_t length=6;
  uint16_t checksum =  1+length+cmd + param1 + (param2 >> 8) + (param2 & 0xFF);

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=param1;
  packet[11]=(param2>>8)&0xFF; packet[12]=param2&0xFF;
  packet[13]=(checksum>>8)&0xFF; packet[14]=checksum&0xFF;

  for (int i=0;i<15;i++) Serial1.write(packet[i]);

  if (readFpPacket(response, sizeof(response), rlen, 500)) {
    Serial.println("sent command  response:");
    printResponse(response, (uint8_t)rlen);
  }
}

// 1 COMMAND, 3 VARIABLES (SEARCH/MATCH)
values sendCommand1(uint8_t cmd, uint8_t param1, uint16_t param2, uint16_t param3) {
  values localStruct{0,0,0};

  drainSerial1Rx();                         // << nuevo

  uint8_t response[64]; size_t rlen=0;

  uint8_t packet[17];
  uint16_t length=8;
  uint16_t checksum =  1+length+cmd + param1
                    + (param2 >> 8) + (param2 & 0xFF)
                    + (param3 >> 8) + (param3 & 0xFF);

  packet[0]=HEADER_HIGH; packet[1]=HEADER_LOW;
  packet[2]=(DEVICE_ADDRESS>>24)&0xFF;
  packet[3]=(DEVICE_ADDRESS>>16)&0xFF;
  packet[4]=(DEVICE_ADDRESS>>8)&0xFF;
  packet[5]= DEVICE_ADDRESS&0xFF;
  packet[6]=0x01;
  packet[7]=(length>>8)&0xFF; packet[8]=length&0xFF;
  packet[9]=cmd;
  packet[10]=param1;
  packet[11]=(param2>>8)&0xFF; packet[12]=param2&0xFF;
  packet[13]=(param3>>8)&0xFF; packet[14]=param3&0xFF;
  packet[15]=(checksum>>8)&0xFF; packet[16]=checksum&0xFF;

  for (int i=0;i<17;i++) Serial1.write(packet[i]);
  Serial1.flush();

  if (!readFpPacket(response, sizeof(response), rlen, 500)) {
    Serial.println("sent command  response: <timeout/sin cabecera>");
    return localStruct;
  }

  Serial.println("sent command  response:");
  printResponse(response, (uint8_t)rlen);

  // Offset de contenido dentro del ACK:
  const uint8_t OFF = 2 + 4 + 1 + 2; // EF01 + addr + PID + Length
  // SEARCH devuelve: [confirm(1)] [pageID(2)] [matchScore(2)]
  localStruct.confirmation = response[OFF + 0];
  localStruct.page         = response[OFF + 2]; // low byte para mantener tu interfaz
  localStruct.score        = response[OFF + 4]; // low byte

  return localStruct;
}



bool receiveResponse() {
  uint8_t response[64]; size_t rlen=0;
  if (!readFpPacket(response, sizeof(response), rlen, 200)) return false;
  printResponse(response, (uint8_t)rlen);
  return (rlen >= 12);
}







// print response packet
void printResponse(uint8_t *response, uint8_t length) {
  Serial.print("Response:");
  for (int i = 0; i < length; i++) {
    if(response[i] < 0x10) Serial.print('0');
    Serial.print(response[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// --- DETECTOR DE HOLD CON REARME ---
bool waitHoldRearm(uint8_t pin, uint32_t holdMs, uint32_t releaseDebounceMs) {
  static bool waiting = false;
  static bool rearm = false;
  static uint32_t t0 = 0;
  static uint32_t tLow = 0;
  static int last = LOW;

  int cur = digitalRead(pin);

  if (rearm) {
    if (cur == LOW) {
      if (tLow == 0) tLow = millis();
      if (millis() - tLow >= releaseDebounceMs) {
        rearm = false;
        tLow = 0;
      }
    } else {
      tLow = 0;
    }
    last = cur;
    return false;
  }

  bool edge = false;
  noInterrupts();
  if (press) { edge = true; press = false; }
  interrupts();

  if (!edge && last == LOW && cur == HIGH) edge = true;

  if (!waiting && edge) { waiting = true; t0 = millis(); }

  if (waiting) {
    if (cur == HIGH) {
      if (millis() - t0 >= holdMs) {
        waiting = false;
        rearm = true;
        last = cur;
        return true;
      }
    } else {
      waiting = false;
    }
  }

  last = cur;
  return false;
}

void enrroll(){
  send_get_image_cmd(CMD_GET_IMAGE);
  delay(100);
  send_cmd2(CMD_GEN_CHAR,0x06);
  delay(100);
  merge_feature_cmd(CMD_REG_MODEL);
  delay(100);
  sendCommand(CMD_STORE_CHAR,6,3); // buffer id=6, page id=3 (ajústalo a tu lógica)
  delay(1000);
}

// ================== MATCH (NUEVO) ==================
bool match() {
  // Aísla la secuencia completa de MATCH + feedback LED
  detachInterrupt(INT1);
  digitalWrite(gps_v_en, LOW);   // power ON sensor
  delay(300);

  send_get_image_cmd(CMD_GET_IMAGE);
  send_cmd2(CMD_GEN_CHAR, 0x06);
  values r = sendCommand1(CMD_SEARCH, 1, 1, 1);

  Serial.printf("confirmation code: %02X \r\n", r.confirmation);
  Serial.printf("page number:       %02X \r\n", r.page);
  Serial.printf("score:             %02X \r\n", r.score);
  Serial.printf("\r\n");

  // LED: rojo si no encontrado (0x09), verde si OK (0x00 y score > 0x08)
  if (r.confirmation == 0x09) {
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0100);
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
  } else if (r.confirmation == 0x00 && r.score > 0x08) {
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0010);
    sendCommand_led(CMD_PS_ControlBLN, 0x03, 0b0000);
  }

  // Actualiza el global para compatibilidad con tus prints fuera
  response_codes = r;

  digitalWrite(gps_v_en, HIGH);  // power OFF sensor (ahorro)
  delay(50);
  attachInterrupt(INT1, blink1, RISING);

  return (r.confirmation == 0x00 && r.score > 0x08);
}

// --- PURGAR RX ANTES DE ENVIAR ---
inline void drainSerial1Rx(uint32_t idle_us) {
  uint32_t t = micros();
  while (micros() - t < idle_us) {
    while (Serial1.available()) { Serial1.read(); t = micros(); }
  }
}
// Lee un paquete alineando a EF 01 y usando Length.
bool readFpPacket(uint8_t *buf, size_t bufmax, size_t &outlen, uint32_t timeout_ms) {
  outlen = 0;
  uint32_t t0 = millis();

  // Buscar EF 01
  int state = 0; // 0: buscando EF, 1: esperando 01
  while (millis() - t0 < timeout_ms) {
    int c = Serial1.read();
    if (c < 0) continue;
    uint8_t b = (uint8_t)c;

    if (state == 0) {
      if (b == 0xEF) { buf[outlen++] = b; state = 1; }
    } else {
      if (b == 0x01) { buf[outlen++] = b; break; }
      state = (b == 0xEF) ? 1 : 0;
      outlen = (state == 1) ? (buf[0] = 0xEF, 1) : 0;
    }
  }
  if (outlen < 2) return false;

  // Leer Address(4) + PID(1) + Length(2)
  while (outlen < 2 + 4 + 1 + 2) {
    if (millis() - t0 >= timeout_ms) return false;
    int c = Serial1.read(); if (c < 0) continue;
    buf[outlen++] = (uint8_t)c;
  }

  // Length = contenido + checksum(2)
  uint16_t L = ((uint16_t)buf[2+4+1] << 8) | buf[2+4+1+1];
  size_t total = 2 + 4 + 1 + 2 + (size_t)L;
  if (total > bufmax) return false;

  // Leer el resto
  while (outlen < total) {
    if (millis() - t0 >= timeout_ms) return false;
    int c = Serial1.read(); if (c < 0) continue;
    buf[outlen++] = (uint8_t)c;
  }
  return true;
}



