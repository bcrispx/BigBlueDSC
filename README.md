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
| SSID | `BigBlueWiFiDSC_V2` |
| Password | `bigbluebigblue` |
| IP address | `192.168.4.1` (fixed) |
| Port | `80` |
| WiFi channel | 6 |

### BigBlueBluetoothDSC_V2

The telescope advertises itself as a Bluetooth serial device.

| Setting | Value |
|---|---|
| Device name | `BigBlueBluetoothDSC_V2` |

## Encoder Resolution

| Axis | Steps/rev |
|---|---|
| RA | 20000 |
| Dec | -24679 |

The negative Dec value reverses the counting direction.

## Connecting with SkySafari

SkySafari Plus or Pro is required (the free version does not support telescope control).

### WiFi

**1. Join the network**
On your device, go to WiFi settings and connect to `BigBlueWiFiDSC_V2` (password: `bigbluebigblue`). Your device may show "no internet" — this is expected; stay connected.

**2. Add a scope preset in SkySafari**
- Open SkySafari → **Settings** → **Telescope** → **Add Preset**
- Choose **Other** as the connection type
- Set **Scope Type** to `Basic Encoder System`
- Set **Mount Type** to `Equatorial Push-To`
- Set **Connection** to `WiFi`
- Enter **IP Address:** `192.168.4.1` and **Port:** `80`
- Tap **Check IP and Port** to confirm the connection
- Enter encoder resolution manually:
  - **RA:** `20000`
  - **Dec:** `-24679`
- Enter the preset name: `Big Blue WiFi DSC V2`
- Save the preset

**3. Connect**
Tap **Scope** on the main screen, then **Connect**. The telescope position indicator will appear on the sky chart.

---

### Bluetooth

**1. Pair the device**
On your device, go to Bluetooth settings and pair with `BigBlueBluetoothDSC_V2`.

**2. Add a scope preset in SkySafari**
- Open SkySafari → **Settings** → **Telescope** → **Add Preset**
- Choose **Other** as the connection type
- Set **Scope Type** to `Basic Encoder System`
- Set **Mount Type** to `Equatorial Push-To`
- Set **Connection** to `Bluetooth`
- Select `BigBlueBluetoothDSC_V2` from the device list
- Enter encoder resolution manually:
  - **RA:** `20000`
  - **Dec:** `-24679`
- Enter the preset name: `Big Blue Bluetooth DSC V2`
- Save the preset

> **Note:** Bluetooth scope connection is only available on Android. iOS users must use the WiFi variant.

**3. Connect**
Tap **Scope** on the main screen, then **Connect**.

---

### Alignment

After connecting, point the telescope at one known star and center it in a medium- or high-power eyepiece. Tap the star in SkySafari and select **Align**. Because Big Blue is polar aligned, a single alignment star is sufficient for accurate pointing across the sky.

---

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
