#include <EtherCard.h>

static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 }; 
//이더넷 카드마다 다르게 설정하는 것(임의적(랜덤)으로 수정하면 됩니다. 16진수(0x는) 유지하고 뒤에 숫자만 변경해 주시면 됩니다.
static byte myip[] = { 165, 132, 46, 120 }; // ip address
const char ad_id[] PROGMEM = "/data/0/"; //아두이노 ID

static byte gwip[] = { 165, 132, 46, 1 }; //gateway
static byte hisip[] = { 52, 68, 126, 129 }; //접속할 IP
static byte dns_ip[] = { 165, 132, 10, 21 }; // dns
static byte mask[] = { 255, 255, 252, 0 }; // subnet mask

byte Ethernet::buffer[500];
const char website[] PROGMEM = "dust.toycode.org";

int dustPin = 0;
char s[32];
float voltage = 0;  // average voltage
float dustdensity = 0;  // average dust density
unsigned long lastSensingTime = 0;
const unsigned long SENSING_INTERVAL = 3000;

int delayTime=280;
int delayTime2=40;
float offTime=9680;

unsigned long lastConnectionTime = 0;
const unsigned long POSTING_INTERVAL = 10*1000;

char url_buf[113];

void setup() {
  Serial.begin(57600);
  initialize_ethernet();
}
 
void loop(){
  ether.packetLoop(ether.packetReceive());
  
  if(millis() - lastSensingTime > SENSING_INTERVAL) {
    delayMicroseconds(delayTime);  
    voltage=analogRead(dustPin);
    dustdensity = 0.17*voltage-0.1;
    delayMicroseconds(delayTime2);
    delayMicroseconds(offTime);
    lastSensingTime = millis();
  }

  if(millis() - lastConnectionTime > POSTING_INTERVAL) {
    sendData();
    lastConnectionTime = millis();
  }

}

void sendData() {
  for(int i=0; i<113; i++)
    url_buf[i] = 0x00;
  String strUrl = "";
  strUrl += dtostrf(dustdensity, 5, 4, s);
  strUrl.toCharArray(url_buf, 112);
  url_buf[112] = 0x00;

  Serial.println();
  Serial.print("<<< REQ ");
  ether.browseUrl(PSTR(ad_id), url_buf, website, my_callback);  
}

static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("");
}

void initialize_ethernet () {
  Serial.println("\n[getStaticIP]");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println( "Failed to access Ethernet controller");

  ether.staticSetup(myip, gwip, dns_ip, mask);

  ether.copyIp(ether.hisip, hisip);
  ether.printIp("Server: ", ether.hisip);

  while (ether.clientWaitingGw())
    ether.packetLoop(ether.packetReceive());
  Serial.println("Gateway found");
}
