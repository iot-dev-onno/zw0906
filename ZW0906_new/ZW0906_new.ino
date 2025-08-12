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
static bool busy = false;
const uint32_t HOLD_MATCH_MS = 1000;
const uint32_t HOLD_ENROLL_MS = 5000;

uint8_t pc;
//noInterrupts();
deatachInterrupt(INT1);
pc = presscount;
attachInterrupt(INT1);
//interrupts();

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
} else if ()

Serial.println(pc);

// termina loop 
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
void send_cmd(uint8_t cmd) {

  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  // build command package
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = (uint8_t)(DEVICE_ADDRESS & 0xFF);

  packet[6] = 0x01;  // Packet identifier: command packet

  packet[7] = (uint8_t)((length >> 8) & 0xFF);  // Packet length high byte
  packet[8] = length & 0xFF;  // Packet length low byte

  packet[9] = cmd;// Define command codes
  
  packet[10] = (checksum >> 8) & 0xFF;
  packet[11] = checksum & 0xFF;

  // Serial.println("send1:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }

  
}


// LED CONTROL
// sendCommand_led(CMD_PS_ControlBLN,0X03,0X07); checkout response lenght
void sendCommand_led(uint8_t cmd, uint8_t param1, uint8_t param2) {

  uint8_t response[32];

  for (int i = 0; i < 32; ++i) {
      response[i] = 0;
  }

  uint8_t index = 0;



  uint8_t packet[16];
  uint16_t length=7;
  uint16_t checksum =  1+length+cmd + param1 + param2 ;

  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = DEVICE_ADDRESS & 0xFF;

  packet[6] = 0x01; // package id

  packet[7] = (length >> 8) & 0xFF;  // packet lenght
  packet[8] = length & 0xFF;

  packet[9] = cmd; // script code

  packet[10] = param1; // function code

  packet[11] = param2; // starting color

  packet[12] = 0x00; // end color

  packet[13] = 0x00; // cycles

  packet[14] = (uint8_t)((checksum >> 8) & 0xFF);

  packet[15] = checksum & 0xFF;

  // Serial.println("send:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }
  uint32_t startTime = millis();
  while (millis() - startTime < 300) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  // printResponse(response, index);


}



// DELETE DATABASE
//send_clear_cmd(CMD_CLEAR_LIB); check for the byte that indicates an error
void send_clear_cmd(uint8_t cmd) {

  uint8_t response[32];

  for (int i = 0; i < 32; ++i) {
      response[i] = 0;
  }

  uint8_t index = 0;

  
  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  // build command package
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = (uint8_t)(DEVICE_ADDRESS & 0xFF);

  packet[6] = 0x01;  // Packet identifier: command packet

  packet[7] = (uint8_t)((length >> 8) & 0xFF);  // Packet length high byte
  packet[8] = length & 0xFF;  // Packet length low byte

  packet[9] = cmd;// Define command codes
  
  packet[10] = (checksum >> 8) & 0xFF;
  packet[11] = checksum & 0xFF;

  // Serial.println("send1:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }



  uint32_t startTime = millis();
  while (millis() - startTime < 1000) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  Serial.println("clear response:");
  printResponse(response, index);
  
}


// GET IMAGE
//send_get_image_cmd(CMD_GET_IMAGE); check for the byte about info return
void send_get_image_cmd(uint8_t cmd) {

  uint8_t response[32];

  for (int i = 0; i < 32; ++i) {
      response[i] = 0;
  }

  uint8_t index = 0;

  
  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  // build command package
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = (uint8_t)(DEVICE_ADDRESS & 0xFF);

  packet[6] = 0x01;  // Packet identifier: command packet

  packet[7] = (uint8_t)((length >> 8) & 0xFF);  // Packet length high byte
  packet[8] = length & 0xFF;  // Packet length low byte

  packet[9] = cmd;// Define command codes
  
  packet[10] = (checksum >> 8) & 0xFF;
  packet[11] = checksum & 0xFF;

  // Serial.println("send1:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }



  uint32_t startTime = millis();
  while (millis() - startTime < 300) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  // Serial.println("merge response:");
  // printResponse(response, index);
  
}



void send_cmd2(uint8_t cmd, uint8_t param1 ) {

  uint8_t response[32];

  for (int i = 0; i < 32; ++i) {
      response[i] = 0;
  }

  uint8_t index = 0;

  uint8_t packet[13];
  uint16_t length=4;
  uint16_t checksum =  1+length+cmd + param1;

  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (DEVICE_ADDRESS >> 24) & 0xFF;
  packet[3] = (DEVICE_ADDRESS >> 16) & 0xFF;
  packet[4] = (DEVICE_ADDRESS >> 8) & 0xFF;
  packet[5] = DEVICE_ADDRESS & 0xFF;

  packet[6] = 0x01;  

  packet[7] = (length >> 8) & 0xFF;  
  packet[8] = length & 0xFF;

  packet[9] = cmd;

  packet[10] = param1;

  packet[11] = (checksum >> 8) & 0xFF;
  packet[12] = checksum & 0xFF;

  // Serial.println("send2:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }


  uint32_t startTime = millis();
  while (millis() - startTime < 300) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  // Serial.println("genchar response:");
  // printResponse(response, index);


}


// REGISTER IMAGE
// merge_feature_cmd(CMD_REG_MODEL); 3.3.1.5 Merge feature PS_RegModel
void merge_feature_cmd(uint8_t cmd) {




  uint8_t response[32];

  for (int i = 0; i < 32; ++i) {
      response[i] = 0;
  }

  uint8_t index = 0;

  
  uint8_t packet[12];
  uint16_t length=3;
  uint16_t checksum =  1+length+cmd;

  // build command package
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = (uint8_t)(DEVICE_ADDRESS & 0xFF);

  packet[6] = 0x01;  // Packet identifier: command packet

  packet[7] = (uint8_t)((length >> 8) & 0xFF);  // Packet length high byte
  packet[8] = length & 0xFF;  // Packet length low byte

  packet[9] = cmd;// Define command codes
  
  packet[10] = (checksum >> 8) & 0xFF;
  packet[11] = checksum & 0xFF;

  // Serial.println("send1:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }



  uint32_t startTime = millis();
  while (millis() - startTime < 1000) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  Serial.println("merge response:");
  printResponse(response, index);
  
}

// SEND COMMAND 2 VARIABLES
void sendCommand(uint8_t cmd, uint8_t param1, uint16_t param2) {

  uint8_t response[32];
  uint8_t index = 0;

  uint8_t packet[15];
  uint16_t length=6;
  uint16_t checksum =  1+length+cmd + param1 + (param2 >> 8) + (param2 & 0xFF);

  // 构建指令包
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (uint8_t)((DEVICE_ADDRESS >> 24) & 0xFF);
  packet[3] = (uint8_t)((DEVICE_ADDRESS >> 16) & 0xFF);
  packet[4] = (uint8_t)((DEVICE_ADDRESS >> 8) & 0xFF);
  packet[5] = DEVICE_ADDRESS & 0xFF;

  packet[6] = 0x01;  // 包标识：命令包

  packet[7] = (length >> 8) & 0xFF;  // 包长度高字节
  packet[8] = length & 0xFF;  // 包长度低字节

  packet[9] = cmd;

  packet[10] = param1;

  packet[11] = (uint8_t)((param2 >> 8) & 0xFF);
  packet[12] = param2 & 0xFF;

  packet[13] = (uint8_t)((checksum >> 8) & 0xFF);
  packet[14] = checksum & 0xFF;

  // Serial.println("send:");
  // printHex(packet,(2+4+3+length));

  for (int i = 0; i < (2+4+3+length); i++) {
    Serial1.write(packet[i]);
  }

  uint32_t startTime = millis();
  while (millis() - startTime < 500) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
      if (index >= 12) break; // check for index lenght per particular command response
    }
  }

  Serial.println("sent command  response:");
  printResponse(response, index);
  

}



// 1 COMMAND, 3 VARIABLES
values sendCommand1(uint8_t cmd, uint8_t param1, uint16_t param2, uint16_t param3) {
  values localStruct;

  uint8_t response[32];

  for (int i = 0; i < 32; ++i) { // clear response array
      response[i] = 0;
  }



  uint8_t packet[17];
  uint16_t length=8;
  uint16_t checksum =  1+length+cmd + param1 + (param2 >> 8) + (param2 & 0xFF) + (param3 >> 8) + (param3 & 0xFF);

  
  packet[0] = HEADER_HIGH;
  packet[1] = HEADER_LOW;

  packet[2] = (DEVICE_ADDRESS >> 24) & 0xFF;
  packet[3] = (DEVICE_ADDRESS >> 16) & 0xFF;
  packet[4] = (DEVICE_ADDRESS >> 8) & 0xFF;
  packet[5] = DEVICE_ADDRESS & 0xFF;

  packet[6] = 0x01;  

  packet[7] = (length >> 8) & 0xFF;  
  packet[8] = length & 0xFF;  

  packet[9] = cmd;

  packet[10] = param1;

  packet[11] = (param2 >> 8) & 0xFF;
  packet[12] = param2 & 0xFF;

  packet[13] = (param3 >> 8) & 0xFF;
  packet[14] = param3 & 0xFF;

  packet[15] = (checksum >> 8) & 0xFF;
  packet[16] = checksum & 0xFF;

  // Serial.println("send:");
  // printHex(packet,(2+4+3+length));
  

  for (int i = 0; i < 17; i++) {// to 17
    Serial1.write(packet[i]);
  }
  Serial1.flush(); 

  uint8_t index = 0;
  uint32_t startTime = millis();

  while (millis() - startTime < 500) {
    if (Serial1.available()>0) {
      response[index] = Serial1.read();
      if (index >= 16){break;}  // check for index lenght per particular command response
      index=index+1;
    }
  }

  Serial.println("sent command  response:");
  printResponse(response, 16);

  // Serial.println();
  // Serial.println("------------------------------------------");  
  // Serial.println();

  // Serial.printf("confirmation code: %02X \r\n",response[9]);
  // Serial.printf("page number:       %02X%02X \r\n",response[10],response[11]);
  // Serial.printf("score:             %02X%02X \r\n",response[12],response[13]);

  localStruct.confirmation = response[9];
  localStruct.page = response[11];
  localStruct.score = response[13];

  return localStruct;

}







bool receiveResponse() {
  uint8_t response[50];
  uint8_t index = 0;

  uint32_t startTime = millis();

  while (millis() - startTime < 200) {
    if (Serial1.available()) {
      response[index++] = Serial1.read();
    }
  }

  printResponse(response, index);

  if (index >= 12) {
    return true;  
  } else {
    return false; 
  }
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

bool waitHold(uint8_t pin, uint32_t holdMs){
  static bool waiting = false;
  static uint32_t t0 = 0;

  if (!waiting){
    noInterrupts();
    bool edge = press;
    if (edge){
      press = false;
      waiting = true;
      t0 = millis();
    }
    interrupts();
  }
  if (waiting) {
    if (digitalRead(pin)==HIGH){
      if (millis() - t0 >= holdMs){
        waiting = false;
        return true;
      }
    } else{
      waiting = false;
    }
  }
  return false;
}

// Devuelve true una sola vez cuando el pin se mantuvo HIGH por holdMs.
// Incluye: rearmado por LOW y rising por software como respaldo.
// releaseDebounceMs: cuánto tiempo debe estar LOW para rearmar.

bool waitHoldRearm(uint8_t pin, uint32_t holdMs, uint32_t releaseDebounceMs) {
  static bool waiting = false;
  static bool rearm = false;
  static uint32_t t0 = 0;
  static uint32_t tLow = 0;
  static int last = LOW;

  int cur = digitalRead(pin);

  // 0) Rearme LOW estable
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

  // 1) Flanco por ISR o por software
  bool edge = false;
  noInterrupts();
  if (press) { edge = true; press = false; }  // << quitar 'extern'
  interrupts();

  if (!edge && last == LOW && cur == HIGH) edge = true;

  // 2) Arrancar hold
  if (!waiting && edge) {
    waiting = true;
    t0 = millis();
  }

  // 3) Verificar hold continuo
  if (waiting) {
    if (cur == HIGH) {
      if (millis() - t0 >= holdMs) {
        waiting = false;
        rearm = true;     // exigir LOW antes del siguiente ciclo
        last = cur;
        return true;
      }
    } else {
      waiting = false;    // se soltó antes de tiempo
    }
  }

  last = cur;
  return false;
}

void enrroll(){
    // register fingerprint

    send_get_image_cmd(CMD_GET_IMAGE);
    delay(100);
    
    send_cmd2(CMD_GEN_CHAR,0x06);
    delay(100);

    merge_feature_cmd(CMD_REG_MODEL);
    delay(100);

    sendCommand(CMD_STORE_CHAR,6,3); // command store, buffer id, page id, juan 1,2. sam 3
    delay(1000);
}

inline void resetPressCounter() {
  noInterrupts();
  presscount = 0;
  press = false;        // opcional: limpia también el flag del edge
  interrupts();
}

