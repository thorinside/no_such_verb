# Development Guide

## Prerequisites

### Software Requirements

- **Arduino CLI** (v1.1.1 or later) - Command-line build tool
- **Teensyduino** - Teensy board support package for Arduino
- **GNU Make** - Build automation
- **bash** or compatible shell
- **Git** - Version control

### Hardware Requirements

- **Teensy 4.0** - Main microcontroller
- **USB Micro-B Cable** - For programming and power
- **SGTL5000 Audio Adapter Board** - Daisy audio codec
- **Breadboard & Jumper Wires** - Prototyping (optional)
- **5× 10kΩ Linear Potentiometers** - Control inputs

### Optional Tools

- **Serial Monitor** (screen, minicom, or arduino-cli monitor)
- **Oscilloscope** - For audio signal verification
- **Logic Analyzer** - For I2S/I2C debugging

## Installation

### macOS

#### 1. Install Arduino CLI

```bash
# Using Homebrew
brew install arduino-cli

# Verify installation
arduino-cli version
```

#### 2. Install Teensy Board Support

```bash
# Add Teensy core
arduino-cli core install teensy:avr

# Verify installation
arduino-cli core list | grep teensy
```

#### 3. Install Teensy Audio Library

```bash
# Install required library
arduino-cli lib install "Teensy Audio Library"
```

#### 4. Verify Teensy Connection

Connect Teensy 4.0 via USB, then:

```bash
# List connected boards
arduino-cli board list
```

Output should show: `/dev/cu.usbmodem* - Teensy 4.0`

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install arduino-cli make git

# Add Teensy support
arduino-cli core install teensy:avr

# Install Teensy Audio Library
arduino-cli lib install "Teensy Audio Library"

# Add user to dialout group (for USB access)
sudo usermod -a -G dialout $USER
newgrp dialout
```

### Windows

```cmd
# Using Chocolatey
choco install arduino-cli

# Or download from https://arduino.cc/pro/software

# Install Teensy support
arduino-cli core install teensy:avr

# Install Teensy Audio Library
arduino-cli lib install "Teensy Audio Library"
```

## Building the Firmware

### Quick Start

```bash
# Navigate to project root
cd /path/to/no_such_verb

# Compile firmware
./build.sh

# Output: build/no_such_verb.hex
```

### Manual Compilation

```bash
# Verbose compilation with full output
arduino-cli compile \
    --fqbn teensy:avr:teensy40 \
    --build-path ./build \
    --warnings all \
    no_such_verb/no_such_verb.ino

# Successful build outputs:
# - Compilation finished successfully
# - Binary size: ~150KB
# - Output file: build/no_such_verb.hex
```

### Compilation Options

```bash
# Verbose output (see all compiler output)
arduino-cli compile --verbose \
    --fqbn teensy:avr:teensy40 \
    no_such_verb/no_such_verb.ino

# Specify output directory
arduino-cli compile --build-path ./my_build_dir \
    --fqbn teensy:avr:teensy40 \
    no_such_verb/no_such_verb.ino

# Clean build (remove old artifacts)
rm -rf build/
arduino-cli compile --fqbn teensy:avr:teensy40 \
    --build-path ./build \
    no_such_verb/no_such_verb.ino
```

## Uploading Firmware

### Prerequisites

1. Connect Teensy via USB
2. Teensy bootloader must be active
3. Press reset button on Teensy (or hold boot button)

### Upload Methods

#### Method 1: arduino-cli upload (Recommended)

```bash
# Automatic upload after compilation
arduino-cli upload \
    -p /dev/cu.usbmodem* \
    --fqbn teensy:avr:teensy40 \
    --input-dir ./build

# Output: "No device found on port" if Teensy not in bootloader
# Hold boot button and press reset to enter bootloader mode
```

**Troubleshooting:**
- If "No device found", press reset while holding boot button
- Wait 2-3 seconds for bootloader to be ready
- Retry upload command

#### Method 2: Teensy Loader (GUI)

```bash
# Download Teensy Loader from https://www.pjrc.com/teensy/loader.html
# 1. Open Teensy Loader application
# 2. File → Open → select "build/no_such_verb.hex"
# 3. Press reset button on Teensy (enters bootloader)
# 4. Click "Program" button
# 5. Firmware uploads and Teensy boots automatically
```

#### Method 3: Manual DFU Mode (Advanced)

```bash
# For bootloader flashing (not typical firmware updates)
# Hold boot button, press reset, release boot button
# Teensy enters DFU mode (not HID bootloader)

arduino-cli upload \
    --fqbn teensy:avr:teensy40:option_cpu_speed=600 \
    -p /dev/cu.usbmodem* \
    --input-dir ./build
```

## Running Locally

### Connect Hardware

```
Teensy 4.0 Pins:
├── USB Power (5V) + GND
├── Pin 7 (TX/RX) → SGTL5000 I2S
├── Pin 8 (TX/RX) → SGTL5000 I2S
├── Pin 19 (SCL) → SGTL5000 I2C (100kΩ pull-up)
├── Pin 18 (SDA) → SGTL5000 I2C (100kΩ pull-up)
├── Pin 20 (TX) → SGTL5000 I2S
├── Pin 21 (RX) → SGTL5000 I2S
├── Pin 23 → SGTL5000 I2S (MCLK)
├── A0-A3, A6 → Potentiometer inputs (0-3.3V)
└── GND → Common ground with all peripherals
```

### Power On

1. Connect USB Micro-B cable from Teensy to computer
2. Connect 3.3V and GND to SGTL5000 codec board
3. Teensy LED should light (power indicator)
4. Firmware runs immediately

### Test Audio

```bash
# Connect 1/4" audio cable from input jack to audio source (guitar/line input)
# Connect 1/4" audio cable from output jack to speakers/headphones/interface
# Turn potentiometers - you should hear effects applied in real-time
# Latency should be imperceptible (< 10ms)
```

## Monitoring and Debugging

### Serial Monitor

```bash
# Open serial monitor at 115200 baud
arduino-cli monitor -p /dev/cu.usbmodem* -c baudrate=115200

# You should see:
# (firmware startup messages if Serial.print() calls exist)
# (or blank if no serial output in firmware)

# Exit: Ctrl+C
```

### CPU Performance Monitoring

Add to main loop in firmware:

```cpp
void loop() {
    controls.update();

    // Print CPU usage every 1 second
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
        float usage = AudioProcessorUsage();
        float max_usage = AudioProcessorUsageMax();

        Serial.print("CPU: ");
        Serial.print(usage);
        Serial.print("% (max: ");
        Serial.print(max_usage);
        Serial.println("%)");

        lastPrint = millis();
    }
}
```

Then view with serial monitor.

### Memory Monitoring

```cpp
// In setup() or loop():
int16_t *block = AudioStream::allocate();
if (block == NULL) {
    Serial.println("ERROR: Audio memory pool exhausted!");
} else {
    AudioStream::release(block);
}
```

### Audio Debugging

**Record output for analysis:**

```bash
# Via USB audio or audio interface
# Record 30 seconds at 44.1kHz
ffmpeg -f alsa -i hw:0 -d 30 output.wav

# Analyze in Audacity or other audio tool
# Check for:
# - Clipping (waveform hitting ceiling)
# - Aliasing (high-frequency fry at high overdrive)
# - Pops/clicks (discontinuities)
```

## Common Development Tasks

### Task 1: Modify Effect Parameter

**Goal:** Change reverb room size range from [50ms, 5000ms] to [100ms, 3000ms]

**Steps:**

1. Open `main.cpp`
2. Find `#define` or constant for reverb size:
   ```cpp
   #define MIN_REVERB_TIME 50    // Change to 100
   #define MAX_REVERB_TIME 5000  // Change to 3000
   ```
3. Recompile:
   ```bash
   ./build.sh
   ```
4. Upload and test

### Task 2: Add New Potentiometer Control

**Goal:** Add a 6th potentiometer for "Reverb Diffusion"

**Steps:**

1. Edit `main.cpp`
2. Add ADC pin definition:
   ```cpp
   #define DIFFUSION_POT A7
   ```
3. In `setup()`, initialize ADC:
   ```cpp
   pinMode(DIFFUSION_POT, INPUT);
   ```
4. In `loop()`, read potentiometer:
   ```cpp
   float diffusion = analogRead(DIFFUSION_POT) / 1023.0f;
   reverbProcessor.setDiffusion(diffusion);
   ```
5. In `ReverbProcessor`, add setter:
   ```cpp
   void ReverbProcessor::setDiffusion(float diff) {
       // Update reverb algorithm
   }
   ```
6. Recompile and upload

### Task 3: Change Audio Input Level

**Goal:** Increase input gain from 1.33Vpp to 2.0Vpp

**Steps:**

1. Open `main.cpp`
2. Find audio codec setup in `setup()`:
   ```cpp
   audioShield.lineInLevel(5);  // Current: 1.33Vpp, 5 = 0-5 steps
   ```
3. Increase gain (0=highest, 15=lowest):
   ```cpp
   audioShield.lineInLevel(3);  // Higher gain
   ```
4. Recompile and upload
5. Test with lower-level instrument input

### Task 4: Adjust Control Response Curve

**Goal:** Make overdrive control more sensitive at low values (exponential scaling)

**Steps:**

1. Find control scaling code:
   ```cpp
   float overdrive = (raw_value / 1023.0f);  // Linear scaling
   ```
2. Apply exponential curve:
   ```cpp
   float overdrive = pow(raw_value / 1023.0f, 2.0f);  // Square curve
   ```
3. Recompile and test
4. Adjust exponent (2.0 = quadratic, 3.0 = cubic) as needed

### Task 5: Add Serial Logging

**Goal:** Log parameter changes to serial monitor for debugging

**Steps:**

1. In `setup()`, initialize serial:
   ```cpp
   Serial.begin(115200);
   Serial.println("Firmware started");
   ```
2. Add logging where parameters change:
   ```cpp
   if (overdrive != last_overdrive) {
       Serial.print("Overdrive: ");
       Serial.println(overdrive);
       last_overdrive = overdrive;
   }
   ```
3. View with:
   ```bash
   arduino-cli monitor -p /dev/cu.usbmodem* -c baudrate=115200
   ```

## Testing Checklist

### Pre-Deployment Testing

- [ ] Code compiles without errors or warnings
- [ ] Firmware uploads successfully
- [ ] Audio input is detected and processed
- [ ] Audio output produces sound (no silence)
- [ ] All 5 potentiometers respond to input
- [ ] CPU usage stays below 85% (check with monitor)
- [ ] No clicking, popping, or dropouts in audio
- [ ] Latency feels real-time (< 20ms is imperceptible)
- [ ] LEDs (power, clip, bypass) function correctly
- [ ] Device survives 30-minute continuous operation

### Audio Quality Testing

- [ ] Test with guitar, bass, synthesizer inputs
- [ ] Test at low, medium, and high input levels
- [ ] Verify no clipping at nominal levels
- [ ] Listen for aliasing at high overdrive settings
- [ ] Verify reverb tail decays smoothly
- [ ] Check dry/wet mix is linear and balanced
- [ ] Record output and analyze in DAW for distortion

### Parameter Response Testing

- [ ] Turn each potentiometer full range; smooth sweep
- [ ] No clicking or pops when turning pots
- [ ] Parameter changes feel responsive (< 50ms latency)
- [ ] Extreme settings don't cause crashes or audio glitches

## Performance Optimization

### CPU Usage Reduction

If CPU usage exceeds 85%, reduce load:

```cpp
// Option 1: Reduce reverb buffer sizes
#define MAX_REVERB_TIME 2000  // Reduce from 5000ms

// Option 2: Use faster filter (cascade fewer stages)
// Replace complex tone control with simple 1-pole filter

// Option 3: Reduce audio block size (more latency trade-off)
#define AUDIO_BLOCK_SIZE 64   // Reduce from 128
```

### Memory Optimization

If running out of audio memory:

```cpp
// Reduce allocated blocks
AudioMemory(100);  // Reduce from 200

// Reduce reverb delay lines
#define MAX_REVERB_TIME 1000  // Limit room size
```

### Latency Optimization

Current latency: ~8ms (acceptable for real-time)

To reduce further:

```cpp
// Reduce audio block size (increases CPU usage)
#define AUDIO_BLOCK_SIZE 64   // From 128

// Avoid blocking operations in loop()
// Use DMA for ADC instead of polling
// Use interrupt-driven I2C for codec control
```

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| Compilation fails | Missing libraries | Run `arduino-cli lib install "Teensy Audio Library"` |
| Upload fails ("No device") | Teensy not in bootloader | Press reset while holding boot button |
| No sound output | Codec not initialized | Check I2C/I2S connections; verify codec enable code |
| Audio crackling | Buffer underrun (CPU overload) | Reduce reverb size, simplify effects |
| High noise floor | ADC picking up noise | Add 0.1µF capacitor on potentiometer inputs |
| Latency noticeable | Large buffer size | Reduce audio block size (increases CPU) |
| Parameters not responding | ADC not configured | Verify `pinMode(A0-A6, INPUT)` in setup() |
| USB connection issues | Driver not installed | Install Teensyduino package for USB support |

## Continuous Integration

### GitHub Actions Build

The project includes `.github/workflows/build.yml` which automatically:

1. Compiles firmware on every push to main
2. Runs on every pull request
3. Uploads built `.hex` artifact for download

To manually trigger:

```bash
# Push to main branch
git add .
git commit -m "Update firmware"
git push origin main

# Or manually trigger via GitHub Actions UI
# (if workflow_dispatch is enabled)
```

### Local CI Simulation

Run same checks locally before push:

```bash
# Clean build
rm -rf build/

# Compile
arduino-cli compile \
    --fqbn teensy:avr:teensy40 \
    --build-path ./build \
    --warnings all \
    no_such_verb/no_such_verb.ino

# Check output
ls -lh build/no_such_verb.hex

# Run tests (if test suite exists)
# (Currently no automated tests - consider adding)
```

## Next Steps

1. **Build and upload** the firmware to your Teensy
2. **Connect audio** and test parameter response
3. **Monitor performance** with CPU/memory tracking
4. **Extend functionality** using the common tasks section
5. **Contribute improvements** back via pull requests

---

For detailed architecture information, see `docs/architecture.md`
For build process details, see `docs/BUILD.md`
For hardware setup, see `docs/hardware/schematic.txt`
