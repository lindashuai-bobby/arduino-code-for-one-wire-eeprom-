#define UNIO_device_address       0xA0
#define UNIO_STARTHEADER 0x55
#define UNIO_READ        0x03
#define UNIO_CRRD        0x06
#define UNIO_WRITE       0x6c
#define UNIO_WREN        0x96
#define UNIO_WRDI        0x91
#define UNIO_RDSR        0x05
#define UNIO_WRSR        0x6e
#define UNIO_ERAL        0x6d
#define UNIO_SETAL       0x67

#define SAK 1
#define noSAK 0

#define TSTBY 605 // StandBy-Pulse 600µs + 5µs offset
#define TSS 15 // StartHeader setup time 10µs + 5µs offset
#define THDR 10//10 // StartHeader low pulse time 5µs + 5µs offset
#define TE 80//60//100 // BitPeriod time 10µs + 5µs offset
#define TE_half (TE>>1) // BitPeriod time 10µs + 5µs offset
#define TE_quarter (TE>>2) // BitPeriod time 10µs + 5µs offset
#define THDL (TE*10) // BitPeriod time 10µs + 5µs offset
#define TWC 5005 // BitPeriod time 10000µs + 5µs offset

#define test_pin 6
#define SCIO_pin 2

byte test_array[256]
//={0,};
={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
  45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,
  94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,
  132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,
  169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,
  206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
  243,244,245,246,247,248,249,250,251,252,253,254,255
  };
  
void bitOne();
void bitZero();
void pulseTHDR();
void pulseStandBy();
void pulseTss();      
void pulseTWC();
void pulseStartHeader();
void pulseAddress();
bool MAK();
bool noMAK();
bool isSAK();
void transmitByte(byte value);
bool connect1();
bool nextCmd();
bool readAddress(int addr, byte *array, int cnt);
bool writeAddress(int addr, byte *array);
byte isWriteEnabled();
byte readByte();
bool enableWrite();
bool disableWrite();
bool isReadyToWrite();
bool transmitAddress(int addr);
int getBit(byte value, int bit);
bool setall();
bool clrall();
void SETALL();
void CLRALL();

int fstEdgeSample=0;
int sndEdgeSample=0;
int sndEdgeSample_last=0;

/*For half period set signal LOW and for other half HIGH*/
void bitOne() {
  digitalWrite(SCIO_pin, LOW);
  delayMicroseconds(40); // Trimmed-Bit-Period
  digitalWrite(SCIO_pin, HIGH);
  delayMicroseconds(40); // Trimmed-Bit-Period
}

/*For half period set signal HIGH and for other half LOW*/
void bitZero() {
  digitalWrite(SCIO_pin, HIGH);
  delayMicroseconds(TE_half); // Trimmed-Bit-Period
  digitalWrite(SCIO_pin, LOW);
  delayMicroseconds(TE_half); // Trimmed-Bit-Period
}

void pulseTHDR() {
  pinMode(SCIO_pin, OUTPUT);
  digitalWrite(SCIO_pin, LOW); // High to Low transition for THDR
  delayMicroseconds(THDR); // THDR Time
}

void pulseStandBy() {
  digitalWrite(SCIO_pin, HIGH); // Write high digital signal
  delayMicroseconds(TSTBY); // let standby-pulse time pass
}

void pulseTss() {
  delayMicroseconds(TSS); // TSS Time
}

void pulseTWC() {
  digitalWrite(SCIO_pin, HIGH); // High to Low transition for THDR
  pinMode(SCIO_pin, OUTPUT);
  delayMicroseconds(TWC); // TWC Time
}

void pulseStartHeader() {
  pulseTHDR();
  //set_SCIO_interrupt();
  transmitByte(UNIO_STARTHEADER);
  //clr_SCIO_interrupt();
}

void pulseAddress() {
  transmitByte(UNIO_device_address);
}

bool MAK() {
//  pinMode(SCIO_pin, OUTPUT);      //5us
  bitOne();
  return true;
}

bool noMAK() {
//  pinMode(SCIO_pin, OUTPUT);      //5us
  bitZero();
  return false;
}

bool isSAK() {
  pinMode(SCIO_pin, INPUT);
  delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
  fstEdgeSample = digitalRead(SCIO_pin);
  delayMicroseconds(TE_half); // Trimmed-Bit-Period
  sndEdgeSample = digitalRead(SCIO_pin);
  delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
  pinMode(SCIO_pin, OUTPUT);
//  digitalWrite(SCIO_pin, sndEdgeSample); // Write output to actual Value before switching pinMode to OUTPUT


  if ((fstEdgeSample == LOW) && (sndEdgeSample == HIGH)) {
    return true;
  } else {
    return false;
  }
}


void transmitByte(byte value) {
  if (value & 0x80)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x40)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x20)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x10)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x08)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x04)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x02)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  if (value & 0x01)
  {
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
  else
  {
    digitalWrite(SCIO_pin, HIGH);
    delayMicroseconds(40); // Trimmed-Bit-Period
    digitalWrite(SCIO_pin, LOW);
    delayMicroseconds(40); // Trimmed-Bit-Period
  }
}


  
/*
void transmitByte(byte value) {
  if (value & 0x80)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x40)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x20)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x10)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x08)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x04)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x02)
    bitOne();
  else
    //bitOne();  
    bitZero();
  if (value & 0x01)
    bitOne();
  else
    //bitOne();  
    bitZero();
}
*/

/*
byte readByte() {
  byte fstEdgeSample_byte = 0b00000000;
  byte sndEdgeSample_byte = 0b00000000;
  byte output_read_byte = 0b11111111;
  byte cnt_read =0;
  long width_st = micros();
  long width_end = micros();
  byte width = 0;

  pinMode(SCIO_pin, INPUT);  
  if(HIGH==digitalRead(SCIO_pin))
  {
    digitalWrite(test_pin,HIGH);
  }
  else
  {
    digitalWrite(test_pin,LOW);
  }  

  for(cnt_read=0;cnt_read<16;cnt_read++)
  {
    while(digitalRead(SCIO_pin)==digitalRead(test_pin));
    digitalWrite(test_pin,digitalRead(_pin));     
  
  }
    
//  width_end = micros();
//  width = (byte)(width_end-width_st);
  
  
}
*/

byte readByte() {
  byte fstEdgeSample_byte = 0b00000000;
  byte sndEdgeSample_byte = 0b00000000;
  byte output_read_byte = 0b11111111;
  byte cnt_read =0;
  int level=0;

  pinMode(SCIO_pin, INPUT);
  for (cnt_read = 0; cnt_read<8; cnt_read++) {  
    delayMicroseconds(17); // Trimmed-Bit-Period
    //digitalWrite(test_pin,HIGH);     
    level=digitalRead(SCIO_pin);
                
    if(level)
    {
      fstEdgeSample_byte <<=1;
      fstEdgeSample_byte |= 1;//fstEdgeSample_byte|(1<<wb_t);
    } 
    else
    {        
        fstEdgeSample_byte <<=1;    
        fstEdgeSample_byte |= 0;        
    }      

//    asm("nop");
    delayMicroseconds(41); // Trimmed-Bit-Period
    //digitalWrite(test_pin,LOW);      
    level=digitalRead(SCIO_pin);  
    if(level)
    {   
        sndEdgeSample_byte <<=1;
        sndEdgeSample_byte |= 1;//sndEdgeSample_byte|(1<<wb_t);
    } 
    else
    {
        sndEdgeSample_byte <<=1;    
        sndEdgeSample_byte |= 0;
    }     
//    asm("nop");
    delayMicroseconds(23); // Trimmed-Bit-Period    
  }
//    digitalWrite(test_pin,HIGH); 
  pinMode(SCIO_pin, OUTPUT);
    //sndEdgeSample_last = digitalRead(SCIO_pin);
    //digitalWrite(SCIO_pin, sndEdgeSample_last); // Write output to actual Value before switching pinMode to OUTPUT
     
/*  if(fstEdgeSample_byte==(byte)(~sndEdgeSample_byte))
  {
     output_read_byte = sndEdgeSample_byte;
  }
  else
  {
     output_read_byte = sndEdgeSample_byte;    
  }*/  
//    digitalWrite(test_pin,LOW); 
    return sndEdgeSample_byte;        
 // return output_read_byte;     
}


bool connect1() {
  pinMode(SCIO_pin, OUTPUT);
  digitalWrite(SCIO_pin, LOW); //
  delayMicroseconds(200); 
  //delay(1);
  digitalWrite(SCIO_pin, HIGH);
  //delay(1); 
  delayMicroseconds(200); 
  pulseStandBy();    
  return nextCmd();
}

bool nextCmd() {
  pulseTss();
  pulseStartHeader();
  MAK();
  if (isSAK() == true) {
    Serial.println("nextCmd: failed 1!");          
    return false;
  }
  pulseAddress();
  MAK();
  if (isSAK() == false) {
    Serial.println("nextCmd: failed 2!");        
    return false;
  }
  return true;
}

bool transmitAddress(int addr) {
  transmitByte(addr >> 8);
  MAK();
  if (isSAK() == false) {
    return false;
  }
  transmitByte(addr & 0xFF);
  MAK();
  if (isSAK() == false) {
    return false;
  }
  return true;
}

bool readAddress(int addr, byte *array,int cnt) {
  transmitByte(UNIO_READ);
  MAK();
  if (isSAK() == false) {
    Serial.println("readAddress: failed 1!");      
    return 0;
  }
  if (!transmitAddress(addr))
   {
    Serial.println("readAddress: failed 2!");       
    return false;
   }
  int i = 0;
  for (i = 0; i < 256; i++)
  {
    array[i] = readByte();
    pinMode(SCIO_pin, OUTPUT);    //5us
    if (i == (cnt-1))
      noMAK();
    else
      MAK();
    if (isSAK() == false) {
    Serial.println("readAddress: failed 3!");    
    Serial.print(i,HEX);          
      return false;
    }
  }
  return true;
}

byte isWriteEnabled() {
  byte ret_val=1;
  transmitByte(UNIO_RDSR);
  MAK();
  if (isSAK() == false) {
    return -1;
  }
  byte rdsr_byte = readByte();
  noMAK();
  if (isSAK() == false) {
    return -2;
  }
 {
  if (getBit(rdsr_byte, 1) == 0) {
    //return 0;
   ret_val = 0;
  }
  if (getBit(rdsr_byte, 2) == 0) {
   ret_val = 2;
  }
  if (getBit(rdsr_byte, 2) == 1) {
   ret_val = 21;
   }  
  if (getBit(rdsr_byte, 3) == 0) {
//   ret_val = 3;
  }
  if (getBit(rdsr_byte, 3) == 1) {
//   ret_val = 31;
  }  
  return ret_val;
 }
  return 1;
}

bool enableWrite() {
  transmitByte(UNIO_WREN);
  noMAK();
  if (isSAK() == false) {
    return false;
  }
  return true;
}

bool disableWrite() {
  transmitByte(UNIO_WRDI);
  noMAK();
  if (isSAK() == false) {
    return false;
  }
  return true;
}

bool isReadyToWrite() {
  transmitByte(UNIO_RDSR);
    MAK();
    if (isSAK() == false) {
      return -1;
    }
    byte rdsr_byte = readByte();
    noMAK();
    if (isSAK() == false) {
      return -2;
    }
    if (getBit(rdsr_byte, 0) == 1) {
      return 0;
    }
    return 1;
}

int getBit(byte a, int bit) {
  return ((a >> bit) & 0x01);
}


byte arr[256];

void setup() {
  pinMode(test_pin,OUTPUT);
  Serial.begin(115200);
  Serial.println("Usage:");
  Serial.println("h - to connect to Arduino");
  Serial.println("Hello - to test connection");
  Serial.println("clrall - to clr the whole eeprom");
  Serial.println("setall - to set the whole eeprom");  
  Serial.println("write - to write to EEPROM at address 0x00");
  Serial.println("read - to read from EEPROM start from address 0x00");
  Serial.println("Waiting for client..(h)");
  while (!GUIConnected()) { //Loop until GUI client is connected
    delay(100);
  }
  //readEEPROM();
}

void loop() {
  if (Serial.available()) {
    String command;
        int addr;    
    command = Serial.readStringUntil('\n');
    //Serial.println(command);
   if (command.equals("h")) Serial.println("h");
 /*   if (command.equals("Hello")) Serial.println("Hello yourself!");
    if (command.equals("clrall")) {
      Serial.println("clr the whole eeprom");
      CLRALL();
    }
    if (command.equals("setall")) {
      Serial.println("set the whole eeprom");
      SETALL();
    }*/
    if (command.equals("read")) {
      Serial.println("read from 020");
      readEEPROM();
    }
    if (command.equals("write"))
    {
        Serial.println("write to 020");      
        for(byte i=0;i<16;i++)
        {
        writeEEPROM(16*i,(test_array+16*i));      
        }
    }
/*   if (command.equals("read")) {
      Serial.println("read from arduino eeprom");
      read_arduino();
    }
    if (command.equals("write"))
    {
        Serial.println("write to arduino eeprom");      
        for(byte i=0;i<16;i++)
        {
        write_arduino(16*i,(test_array+16*i));      
        }
    }*/
    
  /*  {
      Serial.println("input addr:  please send in HEX");
      while (!Serial.available());
      String line = Serial.readStringUntil('\n');
      if (line.length() <= 4) 
      {
        Serial.println("Got addr");
        addr = line.toInt();      
        Serial.print(addr,HEX);
       }
            
      Serial.println("input data: please send in HEX");
      while (!Serial.available());
      line = Serial.readStringUntil('\n');
      if (line.length() <= 16) 
      {
        Serial.println("Got it");
        char buf[17];//="1234567890abcde";
        line.toCharArray(buf, 16);      
        writeEEPROM(addr,buf);
      } 
      else
        Serial.println("Error: String is longer than 16 symbols");
    }*/
  }
}

void readEEPROM() {
    if (connect1()) {
      bool result = readAddress(0x0000, arr,0x100);
//    bool result = readAddress(0x0056, arr,1);
      Serial.println("Connected");
      if (result) {
        //for(int i=0;i<0x100;i++) 
        for(int i=0;i<0x40;i++)         
        {
          //int i=0;
          Serial.print("    addr is 0x");
          Serial.print((4*i),HEX);  
          Serial.print("   data is 0x");                 
          Serial.print(arr[4*i],HEX);    
          
          Serial.print("    addr is 0x");
          Serial.print((4*i+1),HEX);  
          Serial.print("   data is 0x");                 
          Serial.print(arr[4*i+1],HEX);   
          
          Serial.print("    addr is 0x");
          Serial.print((4*i+2),HEX);  
          Serial.print("   data is 0x");                 
          Serial.print(arr[4*i+2],HEX);   
          
          Serial.print("    addr is 0x");
          Serial.print((4*i+3),HEX);  
          Serial.print("   data is 0x");                 
          Serial.println(arr[4*i+3],HEX);   
        }    
        memcpy(test_array,arr,256);
        Serial.println("\n readEEPROM: success to get data!");        
      }
      else
      {
        Serial.println("readEEPROM: failed to get data!");       
      }
    } else {
      Serial.println("readEEPROM: Connection failed!");
    }  
}

bool writeAddress(int addr, byte *array) {
  transmitByte(UNIO_WRITE);
  MAK();
  if (isSAK() == false) {
    return false;
  }
  if (!transmitAddress(addr))
    return false;
  int i = 0;
  for (i = 0; i < 16; i++) {
    transmitByte(array[i]);
//    transmitByte(0x66);
    if (i == 15)
      noMAK();
    else
      MAK();
    if (isSAK() == false) {
      return false;
    }
  }
  pulseTWC();
  return true;
}

void writeEEPROM(int addr, byte *arr){
  //Serial.println(arr);      
  if (connect1()) {
    enableWrite();
  } else {
    Serial.println("writeEEPROM: Connection failed!");
  }
  
    if (connect1()) {     
    int result = writeAddress(addr, arr);
    if (result) {
      Serial.println("Write succeded!");
    } else {
      Serial.println("Write failed!");
    }
    }
  else {
    Serial.println("writeEEPROM: Connection failed!");
  }
  if (connect1()) {
    disableWrite();
  }else {
    Serial.println("writeEEPROM: Connection failed!");
  }
}

void writeEnabled() {
  if (connect1()) {
    int result = isWriteEnabled();
    switch (result) {
      case 1: Serial.println("Write enabled"); break;
      case 0: Serial.println("Write disabled"); break;
      case -1: Serial.println("RDSR: transmit problem"); break;
      case -2: Serial.println("RDSR: read problem"); break;
      //case 2: Serial.println("BP0==0"); break;
      //case 21: Serial.println("BP0==1"); break;
      //case 3: Serial.println("BP1==0"); break;    
      //case 31: Serial.println("BP1==1"); break;           
    }
  } else {
    Serial.println("writeEnabled: Connection failed!");
  }
}
/*
bool setall()
{
   transmitByte(UNIO_SETAL);
   noMAK();
    if (isSAK() == false) {
      return false;
    }
  pulseTWC();
  return true;
}

bool clrall()
{
   transmitByte(UNIO_ERAL);
   noMAK();
    if (isSAK() == false) {
      return false;
    }
  pulseTWC();
  return true;
}

void SETALL() {
  if (connect1()) 
  {   
    Serial.println("SETALL: Connection succeded!");    
    if(setall())
    {
      Serial.println("SETALL succeded!");
    }
    else
    {
      Serial.println("SETALL failed!");       
    }
  } 
  else
  {
    Serial.println("SETALL: Connection failed!");  
  }
}

void CLRALL() {
  if (connect1()) 
  {   
    Serial.println("CLRALL: Connection succeded!");    
    if(clrall())
    {
      Serial.println("CLRALL succeded!");
    }
    else
    {
      Serial.println("CLRALL failed!");       
    }
  } 
  else
  {
    Serial.println("CLRALL: Connection failed!");  
  }
}

void printArray(byte *arr,byte cnt) {
  for (int i = 0; i < cnt; i++) {
    Serial.print(arr[i],HEX);
//    Serial.print(arr[i]);
  }
  Serial.println("printarray");
}
*/

bool GUIConnected() {
  if (Serial.available()) {
    if (Serial.read() == 'h') {
      Serial.println("h");
      return true;
    }
  }
  return false;
}
