# Soil Moisture and pH Value Measurement Device

An Arduino-based soil monitoring device that measures soil moisture levels and pH values in real-time, displaying the results on an OLED screen. This project is perfect for gardeners, farmers, and researchers who need accurate soil condition monitoring.

## Features

- **Real-time Monitoring**: Continuously displays soil moisture percentage and pH levels
- **OLED Display**: Clear visualization using a 128x64 SSD1306 OLED screen with custom icons
- **Hold Function**: Freeze current readings for detailed observation
- **30-Second Averaging Mode**: Collect and average measurements over 30 seconds for more accurate readings
- **Progress Bar**: Visual feedback during measurement countdown
- **Contact Information Display**: Built-in contact screen for project inquiries
- **Serial Output**: Monitor readings via Serial Monitor at 9600 baud
- **Calibrated Measurements**: Configurable calibration values for both sensors

## Hardware Requirements

### Components
- Arduino board (Uno, Nano, or compatible)
- SSD1306 OLED Display (128x64, I2C)
- Soil Moisture Sensor (analog output)
- pH Sensor (analog output)
- 2x Push Buttons
- Jumper wires
- Breadboard (optional)

### Pin Connections

| Component | Arduino Pin | Description |
|-----------|-------------|-------------|
| Soil Moisture Sensor | A0 | Analog input |
| pH Sensor | A1 | Analog input |
| Button 1 | Digital Pin 2 | Hold/Resume + Stop measuring |
| Button 2 | Digital Pin 3 | Countdown + Average button |
| OLED Display | I2C (SDA/SCL) | Default I2C address: 0x3C |

## Installation

### Prerequisites
Install the following Arduino libraries through the Library Manager (Sketch → Include Library → Manage Libraries):
- `Adafruit GFX Library`
- `Adafruit SSD1306`

### Steps
1. Clone this repository or download the `.ino` file
2. Open `soil-moisture-and-pH-value-measure-device.ino` in Arduino IDE
3. Connect your Arduino board to your computer
4. Select the correct board and port from Tools menu
5. Upload the sketch to your Arduino

## Usage

### Basic Operation

1. **Power On**: The device displays a splash screen showing "Soil Moisture & pH Meter"
2. **Live Mode**: By default, the device continuously displays:
   - Soil moisture level (0-100%)
   - pH value (0-14)

### Button Functions

#### Button 1 (Pin 2)
- **Single Press (Live Mode)**: Toggle Hold/Resume
  - Hold freezes the current readings on screen
  - Press again to resume live measurements
- **Single Press (Countdown Mode)**: Stop measuring and return to live mode
- **Single Press (Average Display)**: Exit average screen and return to live mode

#### Button 2 (Pin 3)
- **Single Press (Live Mode)**: Start 30-second countdown measurement
  - Device samples continuously for 30 seconds
  - Progress bar shows measurement progress
  - After 30 seconds, displays averaged results
- **Single Press (Average Display)**: Hide averages and return to live mode

#### Both Buttons (Simultaneous Press)
- **Hold Both Buttons**: Display contact information screen
- **Press Any Single Button**: Exit contact screen

### Display Modes

1. **Live Mode**: Real-time sensor readings with water and flask icons
2. **Hold Mode**: Frozen readings with "HOLD" indicator in top-right
3. **Countdown Mode**: Live sampling with progress bar at bottom
4. **Average Mode**: 30-second averaged results
5. **Contact Screen**: Developer contact information

## Calibration

The device comes with default calibration values that can be adjusted in the code:

### Soil Moisture Sensor
```cpp
int soilDry = 690;   // Analog reading in dry soil (~0%)
int soilWet = 470;   // Analog reading in wet soil (~100%)
```

**To calibrate:**
1. Place sensor in completely dry soil and note the analog reading from Serial Monitor
2. Place sensor in completely wet soil and note the analog reading
3. Update `soilDry` and `soilWet` values accordingly

### pH Sensor
```cpp
float cal4 = 3.88;   // Voltage reading at pH 4.01
float cal7 = 3.08;   // Voltage reading at pH 7.00
```

**To calibrate:**
1. Place sensor in pH 4.01 buffer solution and note the voltage reading
2. Place sensor in pH 7.00 buffer solution and note the voltage reading
3. Update `cal4` and `cal7` values accordingly

## Serial Monitor Output

Connect to the Serial Monitor at **9600 baud** to view detailed readings:
- Raw analog values
- Soil moisture percentage
- pH raw value and voltage
- Calculated pH value
- Average results (after 30-second measurement)

## Troubleshooting

### Display Not Working
- Verify I2C address (default: 0x3C)
- Check wiring connections
- Ensure Adafruit libraries are properly installed

### Inaccurate Readings
- Calibrate sensors according to the Calibration section
- Ensure sensors are properly inserted into soil
- Clean sensor probes regularly

### Buttons Not Responding
- Check button connections
- Verify INPUT_PULLUP configuration
- Add delay for debouncing if needed

## Project Information

**Developer**: Janith Jayashan  

## License

This project is open-source. Feel free to use, modify, and distribute as needed.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests to improve this project.

## Acknowledgments

- Built with Arduino IDE
- Uses Adafruit GFX and SSD1306 libraries for OLED display
- Custom icons designed for water (moisture) and flask (pH) indicators
