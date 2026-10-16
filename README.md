# ESPHome Water Heater Reader for the ESP32-C6

Read a Honeywell / Resideo WV8840_ or WV8860 gas water heater controller into
Home Assistant with an ESP32-C6 running ESPHome. Optional inlet and outlet pipe
thermistors show when hot water is being drawn.

"WV8840_" stands for any suffix: the A, B and C variants appear to differ only
in which water heater maker they were built for and their outlet pressure, not
in their electronics or serial port. Upstream confirmed the C; this project is
testing an A.

This repository is a fork of [russdill/resideo](https://github.com/russdill/resideo),
which reverse-engineered the controller's serial port and wrote the ESPHome
support. That project targets the ESP32-C3. This fork:

- fixes the `irda_uart` component so it compiles and works on the **ESP32-C6**
  (the C6 moved the UART config register and needs a register-update strobe);
- adds ready-made packages for the **Seeed Studio XIAO ESP32-C6** and the
  **Adafruit ESP32-C6 Feather**;
- adds an **inlet / outlet NTC thermistor** package;
- keeps everything from upstream, including the protocol description in
  [PROTOCOL.md](PROTOCOL.md) and the original ESP32-C3 example. Upstream's
  `wv8840c-modbus.yaml` is carried here as `wv8840_-modbus.yaml`.

## Status

| Item | State |
|---|---|
| ESP32-C3, WV8840C and WV8860 | Working upstream |
| ESP32-C6 `irda_uart` patch | Written against the ESP-IDF headers; **not yet verified on hardware** |
| XIAO ESP32-C6 | First test board |
| Adafruit ESP32-C6 Feather | Second test board, for the power-efficiency comparison |
| WV8840A1057 (Bradford White ICON) | Being tested now |
| NTC package | Written; not yet verified |
| Battery operation | Not yet; the first build is USB powered |

If you try it on a different controller or board, please open an issue with
the result either way. The controller reports its model name through the
**Model** text sensor, which is the quickest confirmation that a given valve
speaks the protocol.

## What you get in Home Assistant

From the controller: tank temperature, temperature setting and dial position,
heating / idle status, fault flags, heating cycle count, a `water_heater`
entity, and a setback (vacation) temperature you can write. Many diagnostic
registers are included but disabled by default. See
[packages/wv8840_-modbus.yaml](packages/wv8840_-modbus.yaml).

From the thermistor package: inlet temperature, outlet temperature and the
rise between them.

## Hardware

- An ESP32-C6 board. Packages are provided for the Seeed Studio XIAO ESP32-C6
  and the Adafruit ESP32-C6 Feather; any other C6 board works with its own pins.
- Three wires to the controller's COM port.
- Optional: two 10 kOhm NTC thermistor probes, two 10 kOhm resistors, two
  100 nF capacitors.

The COM port is on the underside of the controller board. Pins, left to right
on the PCB: **RX, TX, GND**. 3.3 V logic. Connectors people have used: Molex
093050-0001, TE 1-2391423-3, or the original Lumberg RAST 2.5 3512 03.

### Wiring

| Signal | XIAO ESP32-C6 | Adafruit ESP32-C6 Feather |
|---|---|---|
| Controller RX | D6 (GPIO16) | TX (GPIO16) |
| Controller TX | D7 (GPIO17) | RX (GPIO17) |
| Controller GND | GND | GND |
| NTC inlet divider | D1 (GPIO1) | A0 (GPIO1) |
| NTC outlet divider | D0 (GPIO0) | A5 (GPIO2) |
| NTC divider supply | D3 (GPIO21) | D9 (GPIO7) |
| Dummy UART pin (leave unconnected) | D10 (GPIO18) | D12 (GPIO14) |

Thermistor divider, one per probe: the supply pin to a fixed 10 kOhm resistor,
then to the ADC pin, then through the NTC to GND. Optional 100 nF from the ADC
pin to GND. The supply pin is switched on only while sampling.

### Antenna

The XIAO ESP32-C6 has an onboard ceramic antenna and a u.FL socket, selected by
an RF switch. **The onboard antenna is the default** and the board package
leaves it that way. To use the u.FL socket, fit an antenna first, then add
`packages/xiao-esp32c6-external-antenna.yaml` to the `files:` list. The
Feather has an onboard antenna only.

## Install with ESPHome Device Builder (from GitHub)

1. Device Builder, **New Device**, name it. When the wizard asks for the
   hardware, pick your actual board (Seeed Studio XIAO ESP32C6 or Adafruit
   Feather ESP32-C6) if it is listed; otherwise choose the ESP32-C6 family. The
   board package sets the exact board anyway, so this only affects the file the
   wizard generates. Skip the install step.
2. Open the device's **Edit** view and copy the generated API encryption key
   and OTA password somewhere; the next step moves them into `secrets.yaml`.
3. Open **Secrets** (top-right menu in Device Builder) and make sure these four
   entries exist. The wizard normally writes the first two; add the other two
   with the values from step 2, or any random values for a new device:

   ```yaml
   wifi_ssid: "your network"
   wifi_password: "your wifi password"
   api_key: "the 32-byte base64 key from the generated config"
   ota_password: "the password from the generated config"
   ```

4. Back in **Edit**, replace the whole file with the example for your board:
   - [examples/xiao-esp32c6-water-heater-reader.yaml](examples/xiao-esp32c6-water-heater-reader.yaml)
   - [examples/feather-esp32c6-water-heater-reader.yaml](examples/feather-esp32c6-water-heater-reader.yaml)

   The device is called `water-heater-reader` ("Water Heater Reader" in Home
   Assistant). Change `name` and `friendly_name` at the top if you want
   something else.
5. Remove the `ntc-inlet-outlet.yaml` line if you have no thermistors.
6. **Install**. The first flash must be over USB (**Plug into this computer**
   in Chrome or Edge); later updates go over the air.
7. Connect to the controller and watch the log. Logging is at DEBUG level, so
   the Modbus traffic is visible. The **Model** text sensor should fill in
   within a few seconds.

The example pulls everything it needs from this repository through ESPHome's
`packages:` mechanism, so there is nothing else to copy. Keep the board package
last in the `files:` list: ESPHome applies later packages over earlier ones, and
the board package carries the pin assignments.

## Install from a local copy

Use this when the Home Assistant machine cannot reach GitHub, or to work on the
files yourself.

1. Download this repository (**Code > Download ZIP** on GitHub) and unzip it.
2. Copy the whole folder into the ESPHome configuration directory on the Home
   Assistant machine, so you end up with
   `/config/esphome/ESPHomeWaterHeaterReader_ESP32C6/` holding `components/`,
   `packages/` and `examples/`. The Samba, Terminal & SSH or File editor
   add-ons can all do this; from a terminal it is
   `scp -r ESPHomeWaterHeaterReader_ESP32C6 root@HA_IP:/config/esphome/`.
3. In Device Builder create the device as above, then replace its `packages:`
   block with local includes. For the XIAO:

   ```yaml
   packages:
     modbus: !include ESPHomeWaterHeaterReader_ESP32C6/packages/modbus-esp32c6-local.yaml
     valve: !include ESPHomeWaterHeaterReader_ESP32C6/packages/wv8840_-modbus.yaml
     ntc: !include ESPHomeWaterHeaterReader_ESP32C6/packages/ntc-inlet-outlet.yaml
     board: !include ESPHomeWaterHeaterReader_ESP32C6/packages/board-xiao-esp32c6.yaml
   ```

   For the Feather, swap in `board-adafruit-feather-esp32c6.yaml` and copy its
   `substitutions:` into the device YAML.

`modbus-esp32c6-local.yaml` is the same as `modbus-esp32c6.yaml` except that it
loads the `irda_uart` component from the local `components/` folder. If you put
the folder somewhere other than `/config/esphome/ESPHomeWaterHeaterReader_ESP32C6`,
edit the `path:` in that file.

### Changing pins or calibration

Override any substitution in your device YAML:

```yaml
substitutions:
  uart_tx: GPIO16
  uart_rx: GPIO17
  uart_dummy_tx: GPIO18
  ntc_inlet_pin: GPIO1
  ntc_outlet_pin: GPIO0
  ntc_power_pin: GPIO21
  ntc_series_resistor: 9.87kOhm   # measure your resistors
  ntc_b: "3950"                   # from the probe datasheet
```

### Working on the component

The local install above is also the way to test changes to `irda_uart`: edit
the files under `components/irda_uart/` and recompile. Changes to the packages
or the component are picked up on the next build.

## Using a different ESP32

- **ESP32-C3:** use upstream's `packages/modbus-esp32c3.yaml` and
  `examples/water-heater.yaml`, both kept here unchanged.
- **Other ESP32-C6 boards:** use `modbus-esp32c6.yaml` and `ntc-inlet-outlet.yaml`,
  add your own `esp32:` block with `variant: esp32c6` and `framework: type: esp-idf`,
  and set the pin substitutions in your device YAML. Any two free GPIOs work
  for the UART; the dummy pin just has to be free; the NTC pins must be ADC1
  (GPIO0 to GPIO6 on the C6).
- **ESP32-H2 / C5:** should take the C6 code path (same register layout), but
  nobody has tried.

## Safety

The controller is the water heater's gas safety control. This project only
talks to its accessory port, the same port the manufacturer's own accessory
module uses, but:

- the protocol includes factory commands that erase calibration data; the
  packages here do not use them, and you should not either;
- the setback, hysteresis and leak-flag controls write to the controller.
  Leave them alone until the reads are solid;
- work on the wiring with the gas control knob off.

Use at your own risk.

## Power

The controller runs on its thermopile and supplies no power to the reader. The
first build is USB powered. For battery operation the plan is deep sleep with
a wake every few minutes; a Thread or sub-GHz radio may replace Wi-Fi later.
Both boards have a lithium charger on board, and the Feather also carries a
MAX17048 fuel gauge (not configured yet). Nothing about battery operation is in
the packages yet.

## Credits and licence

Protocol reverse engineering, the `irda_uart` component and the Modbus
package are by Russ Dill, [russdill/resideo](https://github.com/russdill/resideo),
MIT licence. ESP32-C6 support, the XIAO package, the thermistor package and
this README are additions by rousslabs, also MIT. See [LICENSE](LICENSE).
