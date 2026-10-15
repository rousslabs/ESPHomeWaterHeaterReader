# ESPHome Water Heater Reader for the ESP32-C6

Read a Honeywell / Resideo WV8840 or WV8860 gas water heater controller into
Home Assistant with an ESP32-C6 running ESPHome. Optional inlet and outlet pipe
thermistors show when hot water is being drawn.

This repository is a fork of [russdill/resideo](https://github.com/russdill/resideo),
which reverse-engineered the controller's serial port and wrote the ESPHome
support. That project targets the ESP32-C3. This fork:

- fixes the `irda_uart` component so it compiles and works on the **ESP32-C6**
  (the C6 moved the UART config register and needs a register-update strobe);
- adds a ready-made package for the **Seeed Studio XIAO ESP32-C6**;
- adds an **inlet / outlet NTC thermistor** package;
- keeps everything from upstream, including the protocol description in
  [PROTOCOL.md](PROTOCOL.md) and the original ESP32-C3 example.

## Status

| Item | State |
|---|---|
| ESP32-C3, WV8840C and WV8860 | Working upstream |
| ESP32-C6 `irda_uart` patch | Written against the ESP-IDF headers; **not yet verified on hardware** |
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
[packages/wv8840c-modbus.yaml](packages/wv8840c-modbus.yaml).

From the thermistor package: inlet temperature, outlet temperature and the
rise between them.

## Hardware

- Seeed Studio XIAO ESP32-C6 (other ESP32-C6 boards work with different pins).
- Three wires to the controller's COM port.
- Optional: two 10 kOhm NTC thermistor probes, two 10 kOhm resistors, two
  100 nF capacitors.

The COM port is on the underside of the controller board. Pins, left to right
on the PCB: **RX, TX, GND**. 3.3 V logic. Connectors people have used: Molex
093050-0001, TE 1-2391423-3, or the original Lumberg RAST 2.5 3512 03.

### Wiring, XIAO ESP32-C6

| Signal | XIAO pin | GPIO |
|---|---|---|
| Controller RX | D6 | GPIO16 |
| Controller TX | D7 | GPIO17 |
| Controller GND | GND | |
| NTC inlet divider | D1 | GPIO1 |
| NTC outlet divider | D0 | GPIO0 |
| NTC divider supply | D3 | GPIO21 |
| Dummy UART pin (leave unconnected) | D10 | GPIO18 |

Thermistor divider, one per probe: GPIO21 to a fixed 10 kOhm resistor, then to
the ADC pin, then through the NTC to GND. Optional 100 nF from the ADC pin to
GND. The supply pin is switched on only while sampling.

## Install with ESPHome Device Builder

1. Device Builder, **New Device**, name it, choose **ESP32-C6**, skip the
   install step.
2. Open the device's **Edit** view. Copy the generated `api:` and `ota:` blocks
   aside; they hold the device's key and password.
3. Replace the whole file with
   [examples/xiao-esp32c6-water-heater.yaml](examples/xiao-esp32c6-water-heater.yaml)
   and paste your two blocks back over the placeholders.
4. Remove the `ntc-inlet-outlet.yaml` line if you have no thermistors.
5. **Install**. The first flash must be over USB (**Plug into this computer**
   in Chrome or Edge); later updates go over the air.
6. Connect to the controller and watch the log. The **Model** text sensor should
   fill in within a few seconds.

The example pulls everything it needs from this repository through ESPHome's
`packages:` mechanism, so there is nothing else to copy.

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

### Working on the component locally

To test changes to `irda_uart` without pushing to GitHub, copy `components/`
next to your device YAML and replace the `external_components:` block in
`packages/modbus-esp32c6.yaml` with:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [ irda_uart ]
```

## Using a different ESP32

- **ESP32-C3:** use upstream's `packages/modbus-esp32c3.yaml` and
  `examples/water-heater.yaml`, both kept here unchanged.
- **Other ESP32-C6 boards:** use `modbus-esp32c6.yaml` and override the pins.
  Any two free GPIOs work for the UART; the dummy pin just has to be free.
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
Nothing about battery operation is in the packages yet.

## Credits and licence

Protocol reverse engineering, the `irda_uart` component and the Modbus
package are by Russ Dill, [russdill/resideo](https://github.com/russdill/resideo),
MIT licence. ESP32-C6 support, the XIAO package, the thermistor package and
this README are additions by rousslabs, also MIT. See [LICENSE](LICENSE).
