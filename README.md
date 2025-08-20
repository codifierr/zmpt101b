# ZMPT101B Voltage Sensor for ESPHome

This is an ESPHome external component for the ZMPT101B voltage sensor module, designed to measure AC voltage with high precision.

## Features

- AC voltage measurement using ZMPT101B sensor
- Configurable sampling frequency (40-70 Hz)
- Adjustable sensitivity calibration
- Compatible with ESP8266, ESP32, and RP2040 platforms
- Proper voltage classification and state reporting

## Hardware Setup

### For NodeMCU ESP8266

1. **Wiring:**
   - VCC → 3.3V or 5V
   - GND → GND
   - OUT → A0 (GPIO17)

2. **Important Notes:**
   - ESP8266 only supports ADC on pin A0
   - Input voltage range: 0-1V (use voltage divider if needed)
   - Ensure proper isolation when measuring mains voltage

## ESPHome Configuration

### Basic Configuration for NodeMCU ESP8266

```yaml
esphome:
  name: voltage-monitor
  platform: ESP8266
  board: nodemcuv2

wifi:
  ssid: "Your_WiFi_SSID"
  password: "Your_WiFi_Password"

# Enable logging
logger:

# Enable Home Assistant API
api:

# Enable OTA updates
ota:

# External component
external_components:
  - source: github://codifierr/zmpt101b@main
    components: [zmpt101b]

# ZMPT101B voltage sensor
sensor:
  - platform: zmpt101b
    name: "AC Voltage"
    id: ac_voltage
    pin: A0
    frequency: 50  # Use 60 for North America, 50 for Europe/Asia
    sensitivity: 941.25  # Calibration value - adjust based on your module
    update_interval: 5s  # How often to read the sensor
    accuracy_decimals: 2
    filters:
      - calibrate_linear:
          # Map raw ADC values to actual voltage
          # You may need to calibrate these values
          - 0.0 -> 0.0
          - 1.0 -> 250.0  # Adjust based on your voltage range (max 250V)
```

### Advanced Configuration with Multiple Sensors

```yaml
esphome:
  name: power-monitor
  platform: ESP8266
  board: nodemcuv2

wifi:
  ssid: "Your_WiFi_SSID"
  password: "Your_WiFi_Password"

logger:
api:
ota:

external_components:
  - source: github://codifierr/zmpt101b@main
    components: [zmpt101b]

sensor:
  # Voltage measurement
  - platform: zmpt101b
    name: "Mains Voltage"
    id: mains_voltage
    pin: A0
    frequency: 50
    sensitivity: 941.25
    update_interval: 2s
    accuracy_decimals: 2
    filters:
      - calibrate_linear:
          - 0.0 -> 0.0
          - 1.0 -> 250.0  # Maximum measurable voltage: 250V
      - sliding_window_moving_average:
          window_size: 5
          send_every: 5

  # WiFi signal strength
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 60s

  # Uptime sensor
  - platform: uptime
    name: "Uptime"

# Binary sensor for voltage status
binary_sensor:
  - platform: template
    name: "Voltage Normal"
    lambda: |-
      if (id(mains_voltage).state > 200 && id(mains_voltage).state < 245) {
        return true;
      } else {
        return false;
      }

# Text sensor for status
text_sensor:
  - platform: template
    name: "Power Status"
    lambda: |-
      if (id(mains_voltage).state > 245) {
        return {"High Voltage"};
      } else if (id(mains_voltage).state < 200) {
        return {"Low Voltage"};
      } else {
        return {"Normal"};
      }
    update_interval: 5s
```

## Configuration Options

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `pin` | Pin | Required | ADC pin (A0 for ESP8266) |
| `frequency` | int | 50 | Line frequency in Hz (40-70) |
| `sensitivity` | float | 941.25 | Sensor sensitivity for calibration |
| `update_interval` | Time | 5s | Sensor reading interval |

## Calibration

1. **Initial Setup:** Use the default sensitivity value (941.25)
2. **Measurement:** Compare readings with a known accurate voltmeter
3. **Adjustment:** Modify the sensitivity value:
   - If reading is too high: increase sensitivity value
   - If reading is too low: decrease sensitivity value
4. **Fine-tuning:** Use the `calibrate_linear` filter for precise mapping

## Safety Warnings

⚠️ **DANGER - HIGH VOLTAGE**

- Never work on live circuits without proper safety equipment
- Use appropriate isolation and safety measures when measuring mains voltage
- Ensure proper grounding and use isolation transformers when testing
- This sensor measures AC voltage - use appropriate safety precautions

## Troubleshooting

- **No readings:** Check wiring and ensure A0 pin is used on ESP8266
- **Incorrect values:** Adjust sensitivity parameter and calibration filters
- **Unstable readings:** Increase averaging window or reduce update frequency
- **Compile errors:** Ensure you're using the correct component source
