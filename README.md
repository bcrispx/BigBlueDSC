# Big Blue DSC — Digital Setting Circles

Arduino firmware for the **Big Blue** telescope at the [Warren Rupp Observatory](https://wro.org/), providing digital setting circle (DSC) functionality via WiFi and Bluetooth.

## Hardware

- **Microcontroller:** ESP32
- **Encoders:** Two quadrature optical encoders
  - Azimuth / RA: pins 27 (A), 26 (B)
  - Altitude / Dec: pins 25 (A), 33 (B)
- **Status LED:** pin 2

## Variants

### BigBlueWiFiDSC_V2

The telescope creates its own WiFi access point. Connect your laptop or tablet directly to it.

| Setting | Value |
|---|---|
| SSID | `BigBlueWiFiDSC` |
| Password | `bigbluebigblue` |
| IP address | `192.168.4.1` (fixed) |
| Port | `80` |
| WiFi channel | 6 |

### BigBlueBluetoothDSC_V2

The telescope advertises itself as a Bluetooth serial device.

| Setting | Value |
|---|---|
| Device name | `BigBlueBluetoothDSC` |

## Encoder Resolution

| Axis | WiFi sketch | Bluetooth sketch |
|---|---|---|
| Azimuth / RA | -24679 steps/rev | 24679 steps/rev |
| Altitude / Dec | 20000 steps/rev | 20000 steps/rev |

The azimuth resolution is negative in the WiFi sketch to reverse the counting direction.

## Protocol

Both variants implement the standard Basic Encoder protocol:

| Command (sent by client) | Response |
|---|---|
| `Q` | `+XXXXX\t+XXXXX\r` — azimuth/RA then altitude/Dec encoder counts |
| `H` | `XXXXX-XXXXX` — az/RA resolution then alt/Dec resolution |
| `V` | Firmware version string |

## Status LED

| LED state | Meaning |
|---|---|
| Off | Waiting for connection |
| On | Client connected |

## Building & Flashing

1. Install [Arduino IDE](https://www.arduino.cc/en/software) and the ESP32 board package.
2. Open the `.ino` file for the variant you want (`BigBlueWiFiDSC_V2` or `BigBlueBluetoothDSC_V2`).
3. Select your ESP32 board under **Tools → Board**.
4. Select the correct COM port under **Tools → Port**.
5. Click **Upload**.

If the upload fails with "No serial data received," hold the **BOOT** button on the ESP32 while the IDE is connecting, briefly press **EN/RST**, then release BOOT.

## Notes

- The WiFi variant handles OS captive-portal connectivity probes (Windows, Android, iOS) so the "no internet" warning clears automatically after joining the network.
- Both variants use interrupt-safe spinlocks when reading encoder values to prevent corrupted counts.
- The WiFi variant drops stale client connections after 5 seconds of inactivity.
