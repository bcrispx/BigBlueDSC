/*
 * Bluetooth Digital Setting Circles for the Big Blue Telescope at the Warren Rupp Observatory (wro.org)
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
 *   Alt-Azimuth mounts, with Bluetooth connectivity.
 *
 */

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String firmwareVersion = "2.0";

const long resolution_ra  =  20000;
const long resolution_dec = -24679;
const int  STATUS_LED     =  2;

#define enc_RA_A  27
#define enc_RA_B  26
#define enc_Dec_A 25
#define enc_Dec_B 33

volatile int lastEncodedRA = 0, lastEncodedDec = 0;
volatile long encoderValueRA = 0, encoderValueDec = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
  SerialBT.begin("BigBlueBluetoothDSC_V2");
  pinMode(enc_RA_A,  INPUT_PULLUP);
  pinMode(enc_RA_B,  INPUT_PULLUP);
  pinMode(enc_Dec_A, INPUT_PULLUP);
  pinMode(enc_Dec_B, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(enc_RA_A),  EncoderRA,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_RA_B),  EncoderRA,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_Dec_A), EncoderDec, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc_Dec_B), EncoderDec, CHANGE);
}

void loop() {
  bool connected = SerialBT.connected();
  digitalWrite(STATUS_LED, connected ? HIGH : LOW);

  if (connected && SerialBT.available() > 0) {
    int c = SerialBT.read();

    if (c == 'Q') {                        // encoder query
      long snapRA, snapDec;
      portENTER_CRITICAL(&encoderMux);
      snapRA  = encoderValueRA;
      snapDec = encoderValueDec;
      portEXIT_CRITICAL(&encoderMux);
      printEncoderValue(snapDec);
      SerialBT.print("\t");
      printEncoderValue(snapRA);
      SerialBT.print("\r");

    } else if (c == 'V') {                 // firmware version
      SerialBT.print("Magic DSC ");
      SerialBT.print(firmwareVersion);
      SerialBT.print(", ra resolution = ");
      SerialBT.print(resolution_ra);
      SerialBT.print(", dec resolution = ");
      SerialBT.print(resolution_dec);
      SerialBT.print("\r");

    } else if (c == 'H') {                 // resolution query
      SerialBT.print(resolution_ra);
      SerialBT.print("-");
      SerialBT.print(resolution_dec);
      SerialBT.print("\r");
    }
  }
}

void EncoderRA() {
  int EncodedRA = (digitalRead(enc_RA_A) << 1) | digitalRead(enc_RA_B);
  int sum = (lastEncodedRA << 2) | EncodedRA;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueRA++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueRA--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedRA = EncodedRA;
}

void EncoderDec() {
  int EncodedDec = (digitalRead(enc_Dec_A) << 1) | digitalRead(enc_Dec_B);
  int sum = (lastEncodedDec << 2) | EncodedDec;
  portENTER_CRITICAL_ISR(&encoderMux);
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValueDec++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValueDec--;
  portEXIT_CRITICAL_ISR(&encoderMux);
  lastEncodedDec = EncodedDec;
}

void printEncoderValue(long val)
{
  unsigned long aval;

  SerialBT.print(val < 0 ? "-" : "+");
  aval = abs(val);

  if      (aval < 10)    SerialBT.print("0000");
  else if (aval < 100)   SerialBT.print("000");
  else if (aval < 1000)  SerialBT.print("00");
  else if (aval < 10000) SerialBT.print("0");

  SerialBT.print(aval);
}
