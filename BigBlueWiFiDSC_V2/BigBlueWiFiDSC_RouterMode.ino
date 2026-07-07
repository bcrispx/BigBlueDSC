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
 *   Digital setting circles microcontroller for the Big Blue equatorial
 *   telescope, with WiFi connectivity in ROUTER MODE (STA).
 *   Connects to existing WiFi router instead of creating its own AP.
 *
 */

String firmwareVersion = "2.0-Router";
#include <WiFi.h>
#include <WiFiClient.h>

const char *router_ssid = "YourRouterSSID";
const char *router_password = "YourRouterPassword";

IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const long resolution_ra  =  20000;
const long resolution_dec = -24679;
const int  STATUS_LED     =  2;

WiFiServer server(80);
WiFiClient client;

#define enc_ra_A  27
#define enc_ra_B  26
#define enc_dec_A 25
#define enc_dec_B 33

volatile int lastEncodedDec = 0, lastEncodedRa = 0;
volatile long encoderValueDec = 0, encoderValueRa = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
  delay(1000);
  pinMode(enc_dec_A, INPUT_PULLUP);
  pinMode(enc_dec_B, INPUT_PULLUP);
  pinMode(enc_ra_A,  INPUT_PULLUP);
  pinMode(enc_ra_B,  INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(enc_dec_A), EncoderDec, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_dec_B), EncoderDec, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_ra_A),  EncoderRa,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_ra_B),  EncoderRa,  CHANGE);

  WiFi.mode(WIFI_STA);
  
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    blinkError();
  }
  
  WiFi.begin(router_ssid, router_password);
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    blinkError();
  }
  
  digitalWrite(STATUS_LED, LOW);
  WiFi.setSleep(false);
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
    return;
  }
  
  client = server.available();

  if (client) {
    client.setNoDelay(true);
    digitalWrite(STATUS_LED, HIGH);
    unsigned long lastActivity = millis();

    while (client.connected()) {
      if (client.available()) {
        lastActivity = millis();
        int c = client.read();

        if (c == 'Q') {
          long snapRa, snapDec;
          portENTER_CRITICAL(&encoderMux);
          snapRa  = encoderValueRa;
          snapDec = encoderValueDec;
          portEXIT_CRITICAL(&encoderMux);
          printEncoderValue(snapRa);
          client.print("\t");
          printEncoderValue(snapDec);
          client.print("\r");

        } else if (c == 'V') {
          printFirmware();

        } else if (c == 'H') {
          printResolution();
          client.print("\r");

        } else if (c == 'G') {
          while (client.connected() && client.available()) client.read();
          client.print("HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
          break;
        }
      }

      if (millis() - lastActivity > 30000) break;
    }

    client.stop();
    digitalWrite(STATUS_LED, LOW);
  }
}

void EncoderDec() {
  int encodedDec = (digitalRead(enc_dec_A) << 1) | digitalRead(enc_dec_B);
  int sum = (lastEncodedDec << 2) | encodedDec;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueDec++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueDec--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedDec = encodedDec;
}

void EncoderRa() {
  int encodedRa = (digitalRead(enc_ra_A) << 1) | digitalRead(enc_ra_B);
  int sum = (lastEncodedRa << 2) | encodedRa;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueRa++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueRa--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedRa = encodedRa;
}

void printEncoderValue(long val)
{
  unsigned long aval;

  client.print(val < 0 ? "-" : "+");
  aval = abs(val);

  if      (aval < 10)    client.print("0000");
  else if (aval < 100)   client.print("000");
  else if (aval < 1000)  client.print("00");
  else if (aval < 10000) client.print("0");

  client.print(aval);
}

void printResolution()
{
  client.print(resolution_ra);
  client.print("-");
  client.print(resolution_dec);
}

void printFirmware()
{
  client.print("Big Blue DSC ");
  client.print(firmwareVersion);
  client.print(", RA resolution = ");
  client.print(resolution_ra);
  client.print(", Dec resolution = ");
  client.print(resolution_dec);
  client.print("\r");
}

void blinkError()
{
  while (true) {
    digitalWrite(STATUS_LED, HIGH);
    delay(200);
    digitalWrite(STATUS_LED, LOW);
    delay(200);
  }
}
