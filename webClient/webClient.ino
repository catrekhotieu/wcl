// Demo using DHCP and DNS to perform a web client request.
// 2011-06-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"

SMSGSM sms;

boolean started = false; //trạng thái modul sim
char soupBuffer[1000];
char smstext[160];// nội dung tin nhắn
char number[15]; // số điện thoại format theo định dạng quốc tế
char mrLong[13] = "+84968686717";
char lastProc[20] = "longdeptrai"; //init for first requests
char smsReceiver[20];
char smsContent[160];
char soupResult[160];
char lastProcBuffer[20];
byte posOfReadSMS, i;     
String soupTmp;
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
static uint32_t timer;

const char website[] PROGMEM = "www.emyeuptit.com";

void ethInit();
void ethGet();
void smsInit();
void formatNumber();
void smsSnd();

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+700] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  soupTmp = (const char*) Ethernet::buffer + off;
  Serial.println("...");
  smsSnd();
}


void setup () {
  Serial.begin(9600);
  smsInit();
  smsPcs();
  ethInit();
}

void loop () {
  ethGet();
}

void ethInit(){
  Serial.println(F("\n[webClient]"));

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

#if 1
  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
#elif 2
  // if website is a string containing an IP address instead of a domain name,
  // then use it directly. Note: the string can not be in PROGMEM.
  char websiteIP[] = "192.168.1.1";
  ether.parseIp(ether.hisip, websiteIP);
#else
  // or provide a numeric IP address instead of a string
  byte hisip[] = { 192,168,1,1 };
  ether.copyIp(ether.hisip, hisip);
#endif
    
  ether.printIp("SRV: ", ether.hisip);
}


void ethGet(){
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {
    timer = millis() + 2711;
    Serial.println();
    Serial.print("<<< REQ ");
    ether.browseUrl(PSTR("/listofnumber.php?mssv="), lastProc, website, my_callback);
  }
}

void formatNumber(char input[]){
  String ftmp = input;
  if(ftmp.charAt(0) == '0'){
    ftmp.remove(0, 1);
    ftmp = "+84" + ftmp;
    ftmp.toCharArray(input, ftmp.length() + 1);
  }
}


void smsInit(){
  Serial.println(F("Sim900 Initializing..."));
  delay(2711);
  if (gsm.begin(4800)) {
    Serial.println(F("\nGSM status=READY"));
    started = true;
    delay(2711);
  } else {
    Serial.println(F("\n GSM status=IDLE"));
  }
  delay(500);
}

void smsSnd(){
      soupTmp.replace("<br>", "\n");
      int posOfStart = soupTmp.indexOf("<body>");
      soupTmp.remove(0, posOfStart + 6);
      posOfStart = soupTmp.indexOf("0");
      soupTmp.remove(0, posOfStart);
      int posOfEnd = soupTmp.indexOf("</body>") - 1;
      soupTmp.remove(posOfEnd); // raw remaining
      soupTmp.toCharArray(soupResult, 160);
      int posOfDeli = soupTmp.indexOf('|');
      if(posOfDeli != -1){
        String rcvTmp = soupTmp.substring(0, posOfDeli);
        rcvTmp.toCharArray(smsReceiver, 20);
        String cntTmp = soupTmp.substring(posOfDeli + 1);
        cntTmp.toCharArray(smsContent, 160);
        Serial.println(F("-------------------------------------------posOfDeli------------------------------------"));
        Serial.println(posOfDeli);
        Serial.println(smsReceiver);
        strcpy(lastProc, smsReceiver);
        formatNumber(smsReceiver);
        if(sms.SendSMS(smsReceiver, smsContent)){
          delay(5422);
        } else {
          char badSMSBuffer [100] = "Viettel fuck: ";
          strcat(badSMSBuffer, smsReceiver);
          sms.SendSMS(mrLong, badSMSBuffer);
          delay(5422);
        }
        
      } else {
        //sms.SendSMS(mrLong, "n0thingElse :boss:");
        delay(2711);
        strcpy(lastProc, "a");
      }
}

void smsPcs(){
  char pos;
  pos = sms.IsSMSPresent(SMS_UNREAD);
    if ((int)pos) {
      if (sms.GetSMS(pos, number, smstext, 160)) {
        char smsContent[160];
        if (strcmp(smstext, "kttk") == 0) {
          gsm.SimpleWriteln(F("AT+CUSD=1,\"*101#\""));
          delay(15000);
          char resp[160];
          gsm.read(resp, 160); 
          Serial.println(resp);
          delay(2711);
          sms.SendSMS(mrLong, resp);
          delay(2711);
          strcpy(resp, "");
          // *102#
          gsm.SimpleWriteln(F("AT+CUSD=1,\"*102#\""));
          delay(5000);
          gsm.read(resp, 200); //lay no
          Serial.println(resp);//in ra lan 2
          delay(2711);
          sms.SendSMS(mrLong, resp);
          strcpy(resp, "");
          delay(2711);
        } else if (strcmp(smstext, "kttn") == 0) {
              sms.SendSMS("109", "kt100");
              delay(2711);
        } else {
              strcpy(smsContent, "");
              strcat(smsContent, "rvcd 1 sms from :: ");
              strcat(smsContent, number);
              strcat(smsContent, "\n");
              strcat(smsContent, smstext);
              sms.SendSMS(mrLong, smsContent);
              strcpy(smsContent, "");
              delay(2711);
        }
      }
        sms.DeleteSMS(byte(pos));
    }
    delay(2711);
}

