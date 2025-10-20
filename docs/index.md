# No Such Verb - Master Documentation Index

**Project Type:** Embedded Audio DSP Firmware
**Platform:** Teensy 4.0 (ARM Cortex-M7 @ 600MHz)
**Language:** C++
**Status:** Active Development
**Last Updated:** October 19, 2025

---

## 🚀 Quick Start

**New to this project?** Start here:

1. **[README.md](../README.md)** - Project overview and quick controls reference
2. **[Project Overview](./project-overview.md)** - Executive summary and tech stack
3. **[BUILD.md](./BUILD.md)** - Build instructions and environment setup
4. **[Development Guide](./development-guide.md)** - Common development tasks

**Want to understand the architecture?**

→ Read **[Architecture Documentation](./architecture.md)** (comprehensive technical deep-dive)

---

## 📚 Documentation Structure

### Getting Started

| Document | Purpose | Audience |
|----------|---------|----------|
| **[Project Overview](./project-overview.md)** | Executive summary, quick facts, tech stack | Everyone |
| **[README.md](../README.md)** | Usage guide, controls, quick start | Users & developers |
| **[BUILD.md](./BUILD.md)** | Environment setup, compilation steps | Developers |

### Architecture & Design

| Document | Purpose | Audience |
|----------|---------|----------|
| **[Architecture Documentation](./architecture.md)** | Complete technical architecture, patterns, API | Developers & architects |
| **[Source Tree Analysis](./source-tree-analysis.md)** | Directory structure, file purposes, entry points | Developers & maintainers |
| **[DESIGN.md](./DESIGN.md)** | Design decisions and rationale | Technical leads |

### Development & Operations

| Document | Purpose | Audience |
|----------|---------|----------|
| **[Development Guide](./development-guide.md)** | Setup, common tasks, troubleshooting | Developers |
| **[Hardware Reference](../hardware/)** | PCB design, schematics, BOM | Hardware engineers |

---

## 🎯 Project Summary

**No Such Verb** is a professional-grade digital reverb effects pedal implemented as embedded firmware for the Teensy 4.0 microcontroller.

### Key Characteristics

- **Real-time audio processing** at 44.1kHz, 16-bit stereo
- **Low latency** (~8ms end-to-end)
- **5 analog controls** (potentiometers) for immediate parameter adjustment
- **Schroeder-Moorer reverb** algorithm with configurable room size and decay
- **Overdrive distortion** with harmonic saturation
- **Tone shaping** (tone control filter)

### Quick Reference

| Aspect | Detail |
|--------|--------|
| **CPU Usage** | 43% (full reverb + overdrive) |
| **Audio Latency** | ~8ms end-to-end |
| **Memory** | 1MB RAM + 8MB PSRAM (reverb delays) |
| **Build Time** | ~15 seconds |
| **Firmware Size** | ~150KB |
| **Compile Command** | `./build.sh` or `arduino-cli compile ...` |
| **Upload Command** | `arduino-cli upload -p /dev/cu.usbmodem* ...` |

---

## 🔧 Development Workflow

### Setup (One-Time)

```bash
# 1. Install tools
brew install arduino-cli make

# 2. Add Teensy support
arduino-cli core install teensy:avr

# 3. Install audio library
arduino-cli lib install "Teensy Audio Library"

# 4. Clone and initialize
git clone <repo>
cd no_such_verb
git submodule update --init --recursive
```

### Build & Deploy (Iterative)

```bash
# Build firmware
./build.sh

# Upload to Teensy (in bootloader mode)
arduino-cli upload -p /dev/cu.usbmodem* \
    --fqbn teensy:avr:teensy40 \
    --input-dir ./build

# Monitor serial output (optional)
arduino-cli monitor -p /dev/cu.usbmodem* -c baudrate=115200
```

### Common Tasks

- **Modify effect parameters** → See [Development Guide](./development-guide.md#task-1-modify-effect-parameter)
- **Add new control input** → See [Development Guide](./development-guide.md#task-2-add-new-potentiometer-control)
- **Debug audio issues** → See [Development Guide](./development-guide.md#monitoring-and-debugging)
- **Optimize performance** → See [Development Guide](./development-guide.md#performance-optimization)

---

## 📋 Documentation Map

### By Use Case

**I want to...**

| Goal | Document |
|------|----------|
| Understand the project at a high level | [Project Overview](./project-overview.md) |
| Build the firmware | [BUILD.md](./BUILD.md) |
| Set up my development environment | [Development Guide](./development-guide.md) |
| Understand the technical architecture | [Architecture Documentation](./architecture.md) |
| Navigate the source code | [Source Tree Analysis](./source-tree-analysis.md) |
| Modify a specific component | [Architecture Documentation](./architecture.md) + [Development Guide](./development-guide.md) |
| Create hardware for the project | [Hardware Reference](../hardware/) |
| Troubleshoot a problem | [Development Guide](./development-guide.md#troubleshooting) |

### By Role

**I am a...**

| Role | Start Here |
|------|-----------|
| **User** | [README.md](../README.md) + [Project Overview](./project-overview.md) |
| **Developer** | [BUILD.md](./BUILD.md) + [Development Guide](./development-guide.md) + [Architecture Documentation](./architecture.md) |
| **Architect** | [Architecture Documentation](./architecture.md) + [Source Tree Analysis](./source-tree-analysis.md) |
| **Maintainer** | All documentation, especially [Architecture Documentation](./architecture.md) |
| **Hardware Engineer** | [Hardware Reference](../hardware/) + [Architecture Documentation](./architecture.md) (hardware integration section) |
| **Contributor** | [Development Guide](./development-guide.md) + [Architecture Documentation](./architecture.md) |

---

## 🎵 Audio Signal Flow

```
Instrument/Line Input (0-3.16Vpp analog)
    ↓
[SGTL5000 ADC] (16-bit, 44.1kHz)
    ↓ (I2S digital stream)
[Audio Input Block] (128-sample blocks, 2.9ms)
    ↓
[AudioProcessor::update()] (core DSP pipeline)
    ├─ Overdrive Stage (soft clipping)
    ├─ Tone Filtering (1-pole low-pass)
    └─ Reverb Processing (Schroeder-Moorer)
        ├─ 8 comb filters (parallel)
        └─ 4 allpass filters (series)
    ↓ (with dry/wet mixing)
[Audio Output Block] (128-sample blocks)
    ↓
[SGTL5000 DAC] (16-bit, 44.1kHz)
    ↓ (I2S digital stream)
Line/Headphone Output (0-3.16Vpp analog)
```

**Latency:** ~8ms from input to output (imperceptible to human hearing)

---

## 🛠️ Key Technologies

### Hardware
- **Teensy 4.0** (ARM Cortex-M7 @ 600MHz)
- **SGTL5000** audio codec (16-bit, I2S interface)
- **5× potentiometers** (analog controls)

### Software
- **C++** (Arduino framework)
- **Teensy Audio Library** (block-based DSP)
- **libDaisy** (hardware abstraction)
- **DaisySP** (DSP algorithm library)
- **Arduino CLI** (build system)

### Development
- **GitHub Actions** (CI/CD)
- **Git** (version control)
- **GNU Make** (build automation)

---

## 📊 Repository Statistics

| Metric | Value |
|--------|-------|
| **Main firmware** | ~1200 LOC (main.cpp) |
| **Architecture patterns** | 4 (Strategy, Observer, Template Method, Singleton) |
| **CPU usage** | 43% (full reverb) |
| **Memory usage** | 7.4MB (with max reverb delays) |
| **Build time** | ~15 seconds |
| **Firmware size** | ~150KB (hex format) |
| **Documentation** | This guide + 5 detailed docs |

---

## 🔍 Architecture at a Glance

### System Architecture

```
┌─────────────────────────────────────────────────────┐
│          Teensy 4.0 @ 600MHz (1MB RAM + 8MB PSRAM) │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Audio Processing (interrupt-driven, 344Hz)        │
│  ├─ Audio Input (I2S DMA)                          │
│  ├─ AudioProcessor (core DSP pipeline)             │
│  │  ├─ Overdrive (8% CPU)                          │
│  │  ├─ Tone Filter (5% CPU)                        │
│  │  └─ Reverb (25% CPU)                            │
│  └─ Audio Output (I2S DMA)                         │
│                                                     │
│  Control Interface (loop-level, 60Hz)              │
│  ├─ ADC polling (5 pots)                           │
│  ├─ Parameter smoothing                            │
│  └─ Effect parameter updates                       │
│                                                     │
└─────────────────────────────────────────────────────┘
        ↑ I2C Control      ↓ I2S Audio
    ┌────────────┐    ┌──────────────┐
    │ SGTL5000   │    │ 1/4" Jacks   │
    │ Audio Codec│    │ (in/out)     │
    └────────────┘    └──────────────┘
```

### Data Flow (Simplified)

```
Controls: Potentiometer → ADC → Smoothing → Parameter Update ↓
Audio: Input → Overdrive → Filter → Reverb → Mixer → Output ↓
Timing: Control updates (~20ms latency)
        Audio processing (~8ms latency)
```

---

## 📚 Additional Resources

### External References
- **Teensy Resources:** https://www.pjrc.com/teensy/
- **Audio DSP:** https://www.daisysp.audio/
- **Reverb Algorithm:** Schroeder, "Natural Sounding Artificial Reverberation"
- **Real-Time Audio:** https://www.dsprelated.com/

### In This Repository
- **[Hardware Schematics](../hardware/schematic.txt)** - PCB design reference
- **[Bill of Materials](../hardware/BOM.txt)** - Component list
- **[GitHub Actions Workflow](./.github/workflows/build.yml)** - CI/CD pipeline

---

## ⚠️ Important Notes

### Performance Constraints
- Real-time deadline: 2.9ms per audio block (hard constraint)
- CPU headroom: Keep usage below 85% to maintain stability
- Memory: Reverb delay buffers are pre-allocated in PSRAM

### Hardware Requirements
- **Teensy 4.0** (not compatible with older Teensy versions)
- **SGTL5000 codec board** (required for audio I/O)
- **8MB PSRAM** (for full reverb room sizes)

### Known Limitations
- Single sample rate (44.1kHz only)
- No preset storage between power cycles
- Manual control (no MIDI yet)
- Single effect chain (no multi-effect)

### Future Enhancements
- Preset system (EEPROM storage)
- MIDI over USB
- Footswitch bypass control
- Parametric EQ
- Stereo width control

---

## 🚦 Quick Navigation

### Start Here (5 minutes)
→ [Project Overview](./project-overview.md)

### Setup (15 minutes)
→ [BUILD.md](./BUILD.md) + [Development Guide](./development-guide.md)

### Deep Dive (1 hour)
→ [Architecture Documentation](./architecture.md) + [Source Tree Analysis](./source-tree-analysis.md)

### Daily Development
→ [Development Guide](./development-guide.md)

### Troubleshooting
→ [Development Guide - Troubleshooting](./development-guide.md#troubleshooting)

---

## 📞 Support

**Need help?**

1. Check [Development Guide](./development-guide.md#troubleshooting) troubleshooting section
2. Review [Architecture Documentation](./architecture.md) for technical details
3. Check [GitHub Issues](../../issues) for existing problems/solutions
4. Open a new issue describing your problem

---

## 📝 Document History

| Version | Date | Notes |
|---------|------|-------|
| 1.0 | Oct 19, 2025 | Initial comprehensive documentation |

---

**This is the master index for No Such Verb project documentation.**

Use the links above to navigate to specific documentation based on your needs.

For AI assistants: This index provides the entry point for understanding the complete No Such Verb project. All critical documentation is linked from this page with clear descriptions of purpose and audience.
