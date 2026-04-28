/*
 * WiFi Digital Setting Circles for the Big Blue Telescope at the Warren Rupp Observatory (wro.org)
 *
 * See: https://github.com/bcrispx/BigBlueDSC
 *
 * This software is a modified version of Magic Digital Setting Circles
 *
 * See: https://github.com/MagicDigitalSettingCircles/DSC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Description:
 *   Digital setting circles microcontroller for Equatorial and
 *   Alt-Azimuth mounts, with WiFi connectivity.
 *
 */       

String firmwareVersion = "2.2";
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

const char *ssid = "BigBlueWiFiDSC_V2";
const char *password = "bigbluebigblue";
const long resolution_az = -24679;
const long resolution_alt = 20000;
const int STATUS_LED = 2;

WiFiServer server(80);
WiFiClient client;
 
#define enc_az_A 27                        
#define enc_az_B 26                        
#define enc_al_A 25                        
#define enc_al_B 33                        

volatile int lastEncodedAl = 0, lastEncodedAz = 0;
volatile long encoderValueAl = 0, encoderValueAz = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
  delay(1000);
  pinMode(enc_al_A, INPUT_PULLUP);
  pinMode(enc_al_B, INPUT_PULLUP);
  pinMode(enc_az_A, INPUT_PULLUP);
  pinMode(enc_az_B, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);  

  attachInterrupt(digitalPinToInterrupt(enc_al_A), EncoderAl, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_al_B), EncoderAl, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_az_A), EncoderAz, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_az_B), EncoderAz, CHANGE);
  
  //Serial.begin(115200);
  //Serial.println();

  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password, 6);   // channel 6 for RF stability
  server.begin();
  
}

void loop() {
  client = server.available();

  if (client) {
    digitalWrite(STATUS_LED, HIGH);
    unsigned long lastActivity = millis();

    while (client.connected()) {
      if (client.available()) {
        lastActivity = millis();
        int c = client.read();

        if (c == 'Q') {                      // encoder query
          long snapAz, snapAl;
          portENTER_CRITICAL(&encoderMux);
          snapAz = encoderValueAz;
          snapAl = encoderValueAl;
          portEXIT_CRITICAL(&encoderMux);
          printEncoderValue(snapAz);
          client.print("\t");
          printEncoderValue(snapAl);
          client.print("\r");

        } else if (c == 'V') {               // firmware version
          printFirmware();

        } else if (c == 'H') {               // resolution
          printResolution();
          client.print("\r");

        } else if (c == 'G') {               // HTTP GET — captive-portal probe
          // drain remaining headers
          while (client.connected() && client.available()) client.read();
          client.print("HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
          break;
        }
      }

      // drop stale connections
      if (millis() - lastActivity > 5000) break;
    }

    client.stop();
    digitalWrite(STATUS_LED, LOW);
  }
}

void EncoderAl() {
  int encodedAl = (digitalRead(enc_al_A) << 1) | digitalRead(enc_al_B);
  int sum  = (lastEncodedAl << 2) | encodedAl;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueAl++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueAl--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedAl = encodedAl;
}

void EncoderAz() {
  int encodedAz = (digitalRead(enc_az_A) << 1) | digitalRead(enc_az_B);
  int sum  = (lastEncodedAz << 2) | encodedAz;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueAz++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueAz--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedAz = encodedAz;
}

void printEncoderValue(long val)
{  
  unsigned long aval; 

  if (val < 0)
    client.print("-");
  else
    client.print("+");

  aval = abs(val);

  if (aval < 10)
    client.print("0000");
  else if (aval < 100)
    client.print("000");
  else if (aval < 1000)
    client.print("00");
  else if (aval < 10000) 
    client.print("0");

  client.print(aval);  
}

void printResolution()
{  

   //char response[20];
   //snprintf(response, 20, "%u-%u", STEPS_AZ, STEPS_ALT);
   //remoteClient.println(response);

    client.print(resolution_az);
    client.print("-");
    client.print(resolution_alt);
}

void printFirmware()
{
  client.print("Magig DSC ");
  client.print(firmwareVersion);
  client.print(", az rezolution = ");
  client.print(resolution_az);
  client.print(", alt rezolution = ");
  client.print(resolution_alt);         
  client.print("\r"); 
}