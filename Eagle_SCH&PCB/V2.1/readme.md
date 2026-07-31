# CrowPanel ESP32 Display 2.4 V2.1 Hardware Driver Documentation

| Item | Content |
|---|---|
| Document Version | V1.0 |
| Corresponding Hardware | CrowPanel ESP32 Display-2.4 V2.1 (Schematic dated 2024-03-14) |
| Document Date | 2026-07-29 |
| Author | Codex (compiled by cross-referencing the project schematic and verified in-repo examples) |
| Intended Use | Hardware maintenance, Arduino driver porting, functional regression, and onboarding for new team members |

## 1. Scope and Validation Rules

The hardware basis for this documentation is `V2.1/CrowPanel ESP32 Display-2.4-V2.1-20240314.sch` (EAGLE 9.5.1) and the same-named PDF/BRD files; the software basis is the 8 peripheral examples under `Arduino/Course`, the LVGL composite example, and the project-bundled library configurations for TFT_eSPI/U8g2/XPT2046/SD/Crowbits_DHT20.

The evidence priority is as follows:

1. **Verified code (highest)**: The GPIO, bus mode, polarity, and parameters actually invoked in the examples.
2. **Project-specific library configuration**: Especially the active macros in `TFT_eSPI/User_Setup.h`.
3. **Schematic net connections**: Used to confirm electrical connections, pull-ups/pull-downs, power amplifiers, power supplies, and circuits not covered by code.
4. **General device characteristics**: Used only for explanation, not as a substitute for this board's measured behavior; content that cannot be verified from the repository is marked "to be measured."

Verification status definitions: **A** = confirmed by both schematic and running example; **B** = confirmed by schematic and project configuration but lacking an independent running example; **C** = confirmed by schematic only; **D** = externally connected expansion example, not an onboard device.

> Important: The repository contains no test logs, no pinned build-environment version files, and no measured waveforms. This documentation treats `Arduino/Course` as the "verified working code" referenced by the task, but it does not fabricate experimental results such as temperature, current, or waveforms.

## 2. Product and Software Architecture Overview

- Main controller: ESP32-WROOM-32-N4, 3.3 V logic, module integrates 4 MB SPI Flash and 2.4 GHz Wi-Fi/BLE.
- Software layer: Arduino-ESP32 (Arduino API + ESP-IDF driver headers), with no standalone RTOS tasks; the underlying FreeRTOS/Wi-Fi/BLE protocol stacks are managed by the Arduino core.
- Display: 2.4-inch 320×240 TFT module J1 (schematic library part `QD281801`), configured to run as **ILI9341, 4-wire SPI**.
- Input: 4-wire resistive touchscreen + XPT2046; two onboard buttons, BOOT and RESET.
- Storage: onboard microSD/TF card socket, on an independent VSPI bus.
- Audio: ESP32 DAC2 → SC8002B power amplifier → 2-pin speaker connector.
- Peripheral expansion: I2C, UART2, GPIO/ADC/DAC Crowtail/Twig interfaces.
- Power/Download: USB Type-C, CH340C, auto-download circuit, Li-ion charging/power path, RY3420 3.3 V regulator.

## 3. Peripheral Overview Table

| Peripheral/Device | Reference/Position | Interface and Key Pins (final code values) | Software/Driver | Status |
|---|---|---|---|---|
| ESP32-WROOM-32-N4 | U24 | GPIO0~39; UART0 GPIO1/3 | Arduino-ESP32 / ESP-IDF | A |
| ILI9341 TFT 320×240 | J1 | HSPI: SCLK14, MOSI13, MISO12, CS15, DC2; RST=-1; BL27 | TFT_eSPI | A |
| XPT2046 resistive touch | U3/J1 | Shares SCLK14/MOSI13/MISO12 with TFT; CS33; IRQ36 (unused in code) | TFT_eSPI built-in touch path | A/B |
| microSD/TF card | SD1 | VSPI: SCK18, MISO19, MOSI23, CS5 | SPI + SD + FS | A |
| 3.3 V power indicator LED | LED1 | 3V3→R3 4.7 kΩ→LED1→GND, always on | Pure hardware | C |
| GPIO25 controllable LED (code-defined) | Not mapped to LED1 in schematic | GPIO25; active-high (per verified code) | Arduino GPIO | A/C (with discrepancy) |
| Audio DAC/amplifier | U2/J8 | GPIO26/DAC2 → SC8002B → J8 | ESP-IDF `driver/dac.h` | A |
| I2C expansion port | J6 | **SDA22, SCL21**, 3V3, GND | Wire/U8g2/DHT20 | A/D |
| UART2 expansion port | J10 | MCU RX=16, TX=17, 3V3, GND | HardwareSerial(2) | A/D |
| GPIO/analog expansion port | J7 | GPIO25, GPIO32, 3V3, GND | GPIO/ADC/DAC (by usage) | C |
| Reserved analog/GPIO | Test/reserved nets | GPIO4, 34, 35, 39 | GPIO/ADC (34/35/39 input-only) | C |
| BOOT button | K1 | GPIO0, pulls to GND when pressed, active-low | ROM download boot strap | B |
| RESET button | K4 | EN, pulls to GND when pressed, active-low | Hardware reset | B |
| USB-UART | J3/U6 | Type-C D+/D− → CH340C → UART0 GPIO3/1 | USB serial driver + Serial | B |
| Auto-download | Q9/Q10 | CH340C DTR#/RTS# → GPIO0/EN | esptool/Arduino uploader | B |
| Wi-Fi | U24 internal | 2.4 GHz RF; no external GPIO | WiFi.h | A |
| Bluetooth LE | U24 internal | 2.4 GHz RF; no external GPIO | ESP32 BLE Arduino | A |
| Li-ion charging/power path | U26/Q3/D2/J5 | USB VBUS, BAT+, VIN | 4054A-class linear charging + PMOS/Schottky | C |
| 3.3 V power supply | U1/L4 | VIN → RY3420/HM3420B → 3V3 | Pure hardware, no software driver | C |
| External SSD1306 OLED | J6 external | SDA22, SCL21; software I2C | U8g2 | D |
| External DHT20 | J6 external | SDA22, SCL21; I2C address managed by library | Crowbits_DHT20 + Wire | D |
| External GPS | J10 external | GPS TX→GPIO16(RX2), GPS RX←GPIO17(TX2) | HardwareSerial(2) | D |

## 4. GPIO and Bus Resource Summary

| GPIO | Final Usage | Direction/Alternate | Electrical and Sharing Relationships |
|---:|---|---|---|
| 0 | BOOT | Input/boot strap | Pulled to GND when K1 pressed; must stay high during boot to boot normally from Flash |
| 1 | UART0 TX | Output/U0TXD | Via 22 Ω to CH340C RXD; used for download log |
| 2 | TFT D/C (RS) | Push-pull output | ESP32 boot strap pin; peripherals must not forcibly drive it to a wrong level during power-up |
| 3 | UART0 RX | Input/U0RXD | Via 22 Ω to CH340C TXD |
| 4 | Reserved GPIO/ADC2_CH0 | Bidirectional/ADC | ADC2 use is limited when concurrent with Wi-Fi |
| 5 | TF CS | Push-pull output/VSPI CS | Boot-strap-related pin; card socket has 10 kΩ pull-up network |
| 12 | TFT/touch MISO | Input/HSPIQ | GPIO12 is boot strap MTDI; being pulled high by a peripheral at power-up may change the Flash voltage configuration |
| 13 | TFT/touch MOSI | Output/HSPID | Shared by two devices, arbitrated by independent CS |
| 14 | Upper: TFT/touch SCLK | Output/HSPICLK | Shared by two devices; LCD 15.999999 MHz, touch 600 kHz |
| 15 | TFT CS | Push-pull output/HSPICS0 | Boot strap MTDO; should remain unselected before initialization |
| 16 | UART2 RX | Input/U2RXD | J10 pin 1; connects to peripheral TX |
| 17 | UART2 TX | Output/U2TXD | J10 pin 2; connects to peripheral RX |
| 18 | TF SCK | Output/VSPICLK | SD dedicated bus |
| 19 | TF MISO | Input/VSPIQ | SD dedicated bus |
| 21 | I2C SCL | Open-drain bidirectional (code role) | J6 pin 1, via 1 kΩ series; schematic net name `IO21_SCL` |
| 22 | I2C SDA | Open-drain bidirectional (code role) | J6 pin 2, via 1 kΩ series; schematic net name `IO22_SDA` |
| 23 | TF MOSI | Output/VSPID | SD dedicated bus |
| 25 | Code-defined controllable LED/J7 pin 1 | Push-pull output; also DAC1/ADC2 | Code controls LED high/low; V2.1 schematic only clearly connects it to J7, not to LED1 |
| 26 | Speaker | DAC2 analog output | Fed to SC8002B via coupling/feedback network; must not be reused as a normal digital peripheral |
| 27 | LCD backlight | Push-pull/PWM optional | Drives 2N7002 gate; code HIGH turns it on |
| 32 | J7 pin 2 | GPIO/ADC1_CH4 | 3.3 V logic; usable as a reliable ADC input |
| 33 | Touch CS | Push-pull output | Independent chip-select for XPT2046 |
| 34 | Reserved ADC1_CH6 | **Input only** | No internal output drive capability |
| 35 | Reserved ADC1_CH7 | **Input only** | No internal output drive capability |
| 36 | Touch IRQ | **Input only**/ADC1_CH0 | XPT2046 PENIRQ; current example polls, no attachInterrupt |
| 39 | Reserved ADC1_CH3 | **Input only** | No internal output drive capability |

Bus isolation: display/touch use the HSPI pin group 12/13/14, while the TF card uses the VSPI pin group 19/23/18, so they can operate in parallel; on the same HSPI bus, TFT CS15 and Touch CS33 must be mutually exclusive.

## 5. Per-Peripheral Driver Description

### 5.1 ESP32-WROOM-32-N4 Main Controller

**Hardware**: U24, 3.3 V supply, module model explicitly N4 (4 MB Flash). EN is pulled up by 10 kΩ and connected to the reset button/auto-download circuit; GPIO0 is controlled by the BOOT button and the auto-download circuit. UART0 connects to USB via CH340C.

**Software layer**: Arduino sketch → Arduino-ESP32 core → ESP-IDF HAL/drivers and FreeRTOS. The code contains no bare-metal register writes and no self-created RTOS tasks.

**Basic configuration example**:

```cpp
void setup() {
  Serial.begin(115200);       // Debug baud rate used by most examples
  pinMode(25, OUTPUT);
}
```

When porting, select the classic ESP32 target (not S2/S3/C3), and configure Flash to at least 4 MB. The repository does not record the Arduino core version, partition table, CPU frequency, or Flash mode, so a build manifest should be added during maintenance.

### 5.2 TFT LCD (J1, code configured as ILI9341)

**Connections**: GPIO14=SCLK, 13=MOSI/SDI, 12=MISO/SDO, 15=CS, 2=D/C; TFT RESET does not occupy a GPIO—the project configures `TFT_RST -1`, and the schematic's `TFT_RESET` connects to the main reset EN via R5/C28. GPIO27 controls the backlight cathode through a 2N7002, HIGH turns it on. The display supply and LED anode are 3.3 V.

**Protocol/parameters**: 4-wire hardware SPI; ILI9341; physically portrait 240×320, rotated to 320×240 after the composite example calls `setRotation(1)`; write frequency **15,999,999 Hz**, read frequency 20 MHz. TFT_eSPI's `lcd.begin()` sends the controller initialization register table.

**Software dependencies**: TFT_eSPI; the LVGL example additionally depends on LVGL 9-style APIs. The key project configuration is in `Arduino/libraries/TFT_eSPI/User_Setup.h`.

```cpp
#define ILI9341_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   27
#define SPI_FREQUENCY 15999999

TFT_eSPI lcd;
lcd.begin();
lcd.fillScreen(TFT_BLACK);
pinMode(27, OUTPUT);
digitalWrite(27, HIGH);
lcd.setRotation(1);
```

**Initialization order**: SPI/controller init → clear to black → delay 300 ms (composite example) → backlight HIGH → set rotation → register LVGL flush. This reduces power-on display artifacts.

**Discrepancy note**: The schematic only uses the module part number `QD281801` and does not directly state ILI9341; the code and the active TFT_eSPI macros explicitly select ILI9341, so maintenance and porting should be based on ILI9341. The `TFT_WIDTH/TFT_HEIGHT` comment text mentions ST7789, but the active driver is ILI9341; this is leftover commentary from a generic configuration file and does not mean this board uses ST7789.

### 5.3 XPT2046 Resistive Touch

**Connections**: XPT2046 U3 shares GPIO14/DCLK, 13/DIN, 12/DOUT with the TFT; GPIO33=CS; GPIO36=PENIRQ. The touchscreen's four wires XP/XN/YP/YN connect to J1 through 100 pF filter capacitors. Supply/IOVDD/VREF are all 3.3 V.

**Protocol/parameters**: SPI, project sets `SPI_TOUCH_FREQUENCY 600000` (600 kHz); CS active-low. The current code polls using `lcd.getTouch()`, with a pressure threshold of 600 and no GPIO36 interrupt. The calibration array is for this specific unit/orientation: `{557, 3263, 369, 3493, 3}`.

```cpp
#define TOUCH_CS 33
#define SPI_TOUCH_FREQUENCY 600000
uint16_t calData[5] = {557, 3263, 369, 3493, 3};
lcd.setRotation(1);
lcd.setTouch(calData);
bool pressed = lcd.getTouch(&x, &y, 600);
```

**Note**: After changing the screen, changing the touch lamination orientation, or changing rotation, `calibrateTouch()` must be re-run; do not treat this calibration set as valid across the entire production batch. If switching to interrupt mode, GPIO36 is input-only—confirm on the actual board using PENIRQ active-low/falling-edge before enabling.

### 5.4 microSD/TF Card

**Connections**: GPIO18=SCK, 23=MOSI/CMD, 19=MISO/DATA0, 5=CS/DATA3, 3.3 V supply. CMD, CS, and DATA0/1/2/3 all have 10 kΩ pull-ups, meeting SD/SPI idle requirements.

**Protocol/parameters**: SPI mode of the Arduino SD library. The code explicitly maps VSPI pins and calls `SD.begin(5)` after a 100 ms wait; no frequency is passed, so the actual initial/operating frequency is determined by the Arduino-ESP32 SD library version in use—the repository cannot give a fixed value.

```cpp
SPI.begin(18, 19, 23);  // SCK, MISO, MOSI
delay(100);
if (!SD.begin(5)) { /* mount failed */ }
```

**Dependencies**: `SPI.h`, `SD.h`, `FS.h`. The example verifies mounting, card type, capacity, and two-level directory traversal.

**Note**: The card accepts 3.3 V signals only; the filesystem must finish writing before hot-swapping. The board has no card-detect GPIO confirmed from code/net names, so applications should judge insertion/removal via `SD.begin()`/I/O errors.

### 5.5 Power Indicator LED, GPIO25 Controllable LED, and GPIO Expansion

The LED1 in the schematic is a red **power indicator LED**: 3V3 connects through R3=4.7 kΩ to the LED1 anode, with the cathode to GND, so it stays on during power-up and has no electrical connection to GPIO25. GPIO25 connects to J7 pin 1 through R18=1 kΩ.

The verified example explicitly defines GPIO25 as the `on-board LED`, turned on with HIGH and off with LOW. Following the "make code run first" principle, software porting still uses GPIO25 and this polarity as the effective driving baseline; however, the V2.1 schematic cannot explain this controllable LED's on-board connection—it may be a board revision not synchronized with the schematic, an example targeting a different assembly version, or an LED that must be externally connected via J7. Maintenance personnel should confirm via PCB trace inspection/continuity testing with a multimeter, and must not treat the schematic's LED1 as this GPIO25 controllable LED.

```cpp
pinMode(25, OUTPUT);
digitalWrite(25, HIGH); // Verified example: LED on
```

J7: pin1=GPIO25, pin2=GPIO32, pin3=3V3, pin4=GND. GPIO25 also has DAC1/ADC2, and GPIO32 has ADC1_CH4; expansion peripherals will share GPIO25 with the code-defined LED control. Peripheral current must not directly exceed the ESP32 GPIO drive capability; high loads should add a MOSFET/transistor.

### 5.6 Speaker and SC8002B Amplifier

**Connections**: GPIO26/DAC2 enters U2 SC8002B through R12=4.7 kΩ and an AC/feedback network; U2 is 3.3 V supplied, with differential outputs VO1/VO2 connecting to the J8 speaker connector—neither end may be grounded. The schematic connects SHUTDOWN to AGND and assigns no software enable pin.

**Drive method**: ESP32 built-in 8-bit DAC channel 2. The example outputs via busy-waiting using a 256-point sine table; at 1 kHz the nominal sample rate is 256 ksample/s, playing for 1 s, muted for 2 s, then returning to the mid code 128 at the end to reduce DC step.

```cpp
#include <driver/dac.h>
dac_output_enable(DAC_CHANNEL_2);       // GPIO26
dac_output_voltage(DAC_CHANNEL_2, sample8bit);
```

**Risk**: The current `delayMicroseconds()` software timing occupies the CPU, and the actual frequency is affected by function overhead, making it unsuitable for high-fidelity output or strong concurrency with UI/Wi-Fi. For production, switch to I2S/DAC DMA or timer-driven output, and measure the amplifier gain, clipping, speaker impedance, and temperature rise. Do not perform single-ended oscilloscope ground measurements on the bridged amplifier output.

### 5.7 I2C Expansion Port, SSD1306, and DHT20

J6 pin1=GPIO21, pin2=GPIO22, pin3=3V3, pin4=GND; each signal has a 1 kΩ series resistor. **The final software definition is GPIO22=SDA, GPIO21=SCL**.

```cpp
Wire.begin(22, 21); // SDA, SCL
Crowbits_DHT20 dht20;
dht20.begin();
```

The external OLED example uses U8g2 software I2C:

```cpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
  U8G2_R0, /* clock */ 21, /* data */ 22, U8X8_PIN_NONE);
u8g2.begin();
```

**Parameters**: The SSD1306 example is 128×64 with no independent reset; the DHT20 example does not set address/frequency at the application layer and uses library defaults. The repository code does not lock the I2C clock and slave address, so they should be verified via the library implementation/bus scan before porting; common values cannot substitute for this project's evidence.

**Schematic discrepancy**: The net names are `IO21_SCL`, `IO22_SDA`, which are consistent with the code and the Arduino `Wire.begin(SDA,SCL)` parameters forming the "21=SCL, 22=SDA" connection; this itself is consistent, but old comments in the example files have easily caused the two to be misread in order. All new code must explicitly write `Wire.begin(22,21)`, and must not guess the parameter order based solely on interface names.

### 5.8 UART2 / External GPS

J10 pin1=GPIO16/RX2, pin2=GPIO17/TX2, pin3=3V3, pin4=GND. The example bridges GPS and the USB serial using UART2, 9600 baud, 8N1 full-duplex:

```cpp
HardwareSerial gpsSerial(2);
gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
```

Wiring must be crossed: GPS TX→J10 RX/GPIO16, GPS RX←J10 TX/GPIO17, and share ground. J10 logic level is 3.3 V and must not be directly connected to RS-232 levels. The composite LVGL example only writes `Serial2.begin(9600)`, relying on the ESP32 default UART2 mapping; when porting, explicitly specifying 16/17 is recommended to avoid core/board-type differences.

### 5.9 Wi-Fi

The ESP32 module has built-in 2.4 GHz Wi-Fi with no additional driver GPIO. The example uses Arduino `WiFi.h`, STA mode, auto-reconnect, and blocks waiting for connection:

```cpp
WiFi.begin(ssid, password);
WiFi.setAutoReconnect(true);
while (WiFi.status() != WL_CONNECTED) delay(100);
```

The example contains plaintext demonstration SSID/password and should not enter production firmware. The blocking wait has no timeout; production code should add a timeout, network provisioning, and secure credential storage. When using Wi-Fi, analog sampling on ADC2 (GPIO2/4/12/13/14/15/25/26, etc.) is limited by ESP32 hardware constraints; prefer ADC1's GPIO32/34/35/36/39.

### 5.
10 Bluetooth Low Energy

Using the classic ESP32 BLE Arduino stack, device name `ESP32SPI-BLE`; establish a custom service and a READ/WRITE/NOTIFY characteristic:

```cpp
BLEDevice::init("ESP32SPI-BLE");
auto server = BLEDevice::createServer();
auto service = server->createService(SERVICE_UUID);
auto characteristic = service->createCharacteristic(
  CHARACTERISTIC_UUID,
  BLECharacteristic::PROPERTY_READ |
  BLECharacteristic::PROPERTY_WRITE |
  BLECharacteristic::PROPERTY_NOTIFY);
```

This example has no pairing, encryption, authentication, or write callback and cannot be used directly as a secure product protocol.
If you need to notify the client, you must also set a new value after connection and call `notify()`; the example only demonstrates the advertising/GATT setup path.

### 5.11 USB Type-C, CH340C, and Auto-Download

J3 is the USB Type-C USB 2.0 data/5 V input; CC1/CC2 each have a 5.1 kΩ pulldown, declaring the device side (sink/UFP). D+/D− run through 22 Ω series resistors to CH340C U6. The CH340C UART connects to ESP32 UART0 via 22 Ω: CH340 TXD → GPIO3/RX0, CH340 RXD ← GPIO1/TX0.

The CH340C DTR#/RTS# automatically control GPIO0 and EN via Q9/Q10 (S9013), implementing the download/reset sequence; K1/K4 allow manual control of BOOT/RESET. On the software side, `Serial` uses UART0; the example baud rates are inconsistent between 9600 and 115200, so the serial monitor must match the current sketch.

Maintenance note: UART0 handles both downloading and logging; external devices must not drive GPIO1/3 strongly during reset/download. Type-C only depicts USB 2.0 and does not support USB-PD negotiation; it must not be powered via PD high voltage.

### 5.12 Battery Charging, Power Path, and 3.3 V Supply

**Charging/Path**: J5 is the battery connector, pin1=BAT+, the remaining visible pins are grounded; U26 is marked `4054A`, powered by USB VBUS, with PROG setting the charge current via R9=2 kΩ; Q3 (PMOS-3401) and D2 (1N5817) form the USB/battery-to-VIN path and reverse-current protection. The specific charge-current formula, termination voltage, and battery polarity must be verified against the actual mounted U26 datasheet and silkscreen; the repository contains no BOM/datasheet, and this document makes no assumptions.

**3.3 V**: U1 schematic deviceset `HM3420B`, value `RY3420`, L4=5.6 µH; feedback R20=45.3 kΩ, R21=10 kΩ, output net name 3V3. It is a switching power supply topology supplying the MCU, LCD, touch, SD, and interfaces. The code has no power enable control.

**Risks**:

- Only allow a single-cell Li-ion system to be connected after confirming the specific U26 model; J5 polarity must be verified on the actual board.
- The charge current corresponding to R9=2 kΩ must not be taken into mass production directly from a generic 4054 formula; the actual part number must be checked and temperature rise verified.
- USB/battery switching, undervoltage behavior, maximum 3.3 V load, and ripple are not covered by the software example and require dedicated power testing.
- All external interfaces are rated at 3.3 V nominal; 5 V GPIO levels must not be applied.

### 5.13 BOOT, RESET, and Reserved Analog Pins

- K1: GPIO0→GND, active low; hold BOOT then reset to enter ROM download mode.
- K4: EN→GND, active low hard reset; EN has a pull-up and capacitor forming the power-on reset.
- GPIO34/35/39: schematic marked as a reserved ADC/analog network, input only, with no internal pull-up/pull-down.
- GPIO4: reserved GPIO/ADC2; its ADC2 function should not be relied upon when Wi-Fi is in use.
- GPIO36: already connected to touch PENIRQ and cannot be used unconditionally as a general-purpose ADC.

## 6. LVGL Comprehensive Driver Path

The comprehensive example simultaneously validates TFT, touch, GPIO25, I2C DHT20, and retains UART2 initialization. LVGL uses a 320×240 landscape display with a 1/8-screen single buffer: `320*240/8` `lv_color_t` units. The flush callback writes via a TFT_eSPI transaction; the input callback polls the touch.

```cpp
static lv_color_t buf1[320 * 240 / 8];
lv_init();
lv_tick_set_cb(millis);

lv_display_t *disp = lv_display_create(320, 240);
lv_display_set_flush_cb(disp, my_disp_flush);
lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1),
                       LV_DISPLAY_RENDER_MODE_PARTIAL);

lv_indev_t *indev = lv_indev_create();
lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
lv_indev_set_read_cb(indev, my_touchpad_read);
```

The main loop calls `lv_timer_handler()` every 10 ms but also synchronously reads DHT20 and prints heavily to the serial port. For production, the sensor sampling frequency should be reduced (e.g., via a separate timer/timestamp period) to avoid I2C or serial blocking that degrades UI frame rate.

## 7. Schematic–Code Cross-Verification Conclusions

| Item | Schematic | Verified Code/Config | Conclusion and Handling |
|---|---|---|---|
| LCD controller | J1 only marked `QD281801` | `ILI9341_DRIVER` | Use ILI9341 as reference; module part number does not equal controller name |
| LCD SPI | 14/13/12, CS15, DC2 | exactly the same | Consistent |
| LCD reset | `TFT_RESET` to EN (via R5/C28) | `TFT_RST=-1` | Consistent: no independent GPIO |
| Backlight | GPIO27→2N7002 | GPIO27 HIGH then lights up | Consistent; PWM can be added |
| Touch | XPT2046, shared SPI, CS33, IRQ36 | Shared SPI, CS33; polling does not use IRQ | Connection consistent; IRQ function not verified in code |
| TF card | SCK18/MOSI23/MISO19/CS5 | exactly the same | Consistent |
| I2C | Net name 21=SCL, 22=SDA | `Wire.begin(22,21)` | Consistent; note the API parameter order is SDA, SCL |
| UART2 | RX16/TX17 | Explicit example identical | Consistent; comprehensive example suggests adding explicit pins |
| Speaker | GPIO26/DAC2→SC8002B | DAC_CHANNEL_2 | Consistent |
| LED | LED1 is a 3V3 always-on power light; GPIO25 only clearly connects to J7 | Code refers to GPIO25 as the onboard LED, HIGH=on | Clearly inconsistent; drive follows code, physical connection reason pending continuity confirmation |
| TFT SPI frequency | Undefined in schematic | 15,999,999 Hz | Follow project configuration |
| Touch SPI frequency | Undefined in schematic | 600 kHz | Follow project configuration |
| DHT20/OLED/GPS | Only expansion ports provided, no onboard devices | External module examples | Marked as external capability, not listed as onboard devices |

Apart from the GPIO25 controllable LED, no clear "schematic pin conflicting with running code" was found. The GPIO25 discrepancy can only list possible causes (board revision/assembly version/external LED), and without physical continuity evidence no single cause can be asserted.

## 8. Risks and Notes Checklist

### 8.1 High Priority

1. **Strap-pin conflict**: GPIO0, 2, 5, 12, and 15 all involve boot configuration or onboard SPI; peripherals strongly pulling at power-on may prevent booting. In particular, a high level on GPIO12 may select an incompatible Flash power-supply configuration.
2. **3.3 V level**: I2C/UART/GPIO/SPI are not 5 V-tolerant interfaces; 5 V modules must add level shifting.
3. **Amplifier is differential output**: Neither terminal of J8 is ground; do not ground the negative terminal or directly short it with a grounded oscilloscope probe clip.
4. **Battery safety not closed**: With no BOM and no specific U26 datasheet, charge current, temperature rise, battery polarity, and protection capability must be confirmed by measurement.
5. **Credential leakage**: The Wi-Fi example contains a plaintext demo password; for mass production it must be removed and the real credentials rotated.

### 8.2 Medium Priority

1. GPIO25 simultaneously carries the code-defined LED function and an expansion port; GPIO36 simultaneously connects to the touch IRQ; do not reassign them. GPIO25's LED physical connection needs re-checking per the discrepancy noted above.
2. TFT and touch share SPI; any self-written driver must correctly pull up the non-target CS and avoid holding transactions across devices.
3. GPIO34/35/36/39 are input only; they cannot drive an LED, CS, or bus clock.
4. ADC2 conflicts with Wi-Fi concurrency; analog sampling should prefer GPIO32/34/35/39 (of which 36 is already occupied).
5. The software DAC sine implementation keeps the CPU busy and jitters under UI, BLE/Wi-Fi concurrency.
6. Touch calibration is related to screen orientation/individual unit; recalibrate after changing the screen.
7. The SD card has no card-detect pin visible; removing the card during writing will corrupt the filesystem.

### 8.3 Maintainability

1. The repository does not pin versions of Arduino-ESP32, TFT_eSPI, LVGL, etc.; a reproducible build manifest should be added.
2. Example serial baud rates mix 9600/115200; unify the product logging configuration.
3. `TFT_eSPI/User_Setup.h` is a global library configuration that may be overwritten on library upgrade/reinstall; migrate it to a standalone setup file or build macro.
4. The comprehensive example has no explicit touch `setTouch(calData)`; if the library does not save/load calibration, coordinates may be wrong and should be set explicitly during product initialization.
5. The BLE example has no security mechanism and the Wi-Fi connection has no timeout; both are teaching code and should not be carried into production as-is.

## 9. Driver Porting Checklist

1. Select the classic ESP32 target, 4 MB Flash, and confirm Flash mode and the GPIO12 boot state.
2. Pin the versions of Arduino-ESP32, TFT_eSPI, LVGL, U8g2, SD, and Crowbits_DHT20.
3. Import this project's valid TFT_eSPI macros; confirm ILI9341 and HSPI 12/13/14.
4. Keep TFT CS15, Touch CS33, and SD CS5 deselected at power-on.
5. Start the display following the sequence "LCD init and clear screen → backlight GPIO27 HIGH".
6. After setting rotation=1, recalibrate the touch and save calibration data for each screen assembly orientation.
7. For I2C explicitly call `Wire.begin(22,21)`; for UART2 explicitly call `begin(...,16,17)`.
8. For SD explicitly call `SPI.begin(18,19,23)` then `SD.begin(5)`, and perform plug/unplug and power-loss tests.
9. Check that GPIO25, 26, 27, 36, and the strap pins are not re-assigned by the application.
10. Complete USB/battery switching, maximum load, backlight PWM, audio clipping/temperature-rise, and EMC measurements.

## 10. Suggested Hardware Regression Tests

| Test | Method | Pass Criteria |
|---|---|---|
| Boot/Download | USB cold start, auto-download, manual BOOT+RESET, 20 times each | No boot failures, stable download |
| 3.3 V Supply | USB/battery, no-load/full-load ripple and drop measurement | Meets MCU/LCD/SD device specifications |
| LCD | RGB/solid color/full-screen refresh/read ID (if supported) | No glitch, color error, or tearing anomaly |
| Backlight | GPIO27 switch and PWM sweep | No flicker; MOS/LED temperature rise qualified |
| Touch | Five-point calibration, corners, long press, swipe | Coordinate error meets product spec, no false touches |
| SD | Multiple capacity/speed-class cards, repeated mount and large-file read/write | No CRC/I/O errors; power-loss strategy effective |
| I2C | Scan, DHT20/OLED on same bus, long-line stress | No address conflict/NACK; waveform rise time qualified |
| UART2 | 9600 8N1 loopback and GPS long-duration reception | No packet loss/frame error |
| Audio | Multiple frequency/amplitude, rated speaker, temperature rise | No obvious clipping/DC bias; temperature rise qualified |
| RF | Wi-Fi/BLE concurrency, throughput during LCD/SD refresh | No unacceptable dropout or interference |
| Battery | Charge curve, termination, hot-plug, reverse current, undervoltage | Complies with actual U26/battery specifications and safety requirements |

## 11. Evidence File Index

- Schematic: `V2.1/CrowPanel ESP32 Display-2.4-V2.1-20240314.sch`
- Schematic PDF: `V2.1/CrowPanel ESP32 Display-2.4-V2.1-20240314.pdf`
- PCB: `V2.1/CrowPanel ESP32 Display-2.4-V2.1-20240314.brd`
- LCD/touch config: `Arduino/libraries/TFT_eSPI/User_Setup.h`
- Comprehensive example: `Arduino/Course/LVGL_Arduino2.4/LVGL_Arduino2.4.ino`
- Single examples: `Arduino/Course/Example1_LED_blinking` through `Example8_GPS_Module`
- Related libraries: TFT_eSPI, XPT2046_Touchscreen, SD, U8g2, Crowbits_DHT20, LVGL

---

**Handoff Conclusion**: The core driver baseline of this board consists of two non-overlapping SPI groups (LCD/touch HSPI and TF VSPI), I2C SDA22/SCL21, UART2 RX16/TX17, DAC2/GPIO26 audio, GPIO27 high-active backlight, and the code-defined GPIO25 controllable LED (whose on-board connection requires physical verification). The most error-prone aspects when porting are the TFT_eSPI global configuration, touch calibration, I2C API parameter order, external levels on the strap pins, and carrying the teaching examples' blocking/plaintext/no-security logic directly into product firmware.
