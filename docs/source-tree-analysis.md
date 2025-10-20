# Source Tree Analysis

## Complete Directory Structure with Annotations

```
/Users/nealsanche/nosuch/no_such_verb/
│
├── README.md
│   └─ Project overview: "Reverb for Patch.init()"
│     Purpose: Quick start guide, controls documentation, usage instructions
│     Size: 510 bytes (brief)
│
├── BUILD.md
│   └─ Build instructions for firmware compilation
│     Contains: Makefile usage, SDK setup, bootloader programming
│
├── Makefile
│   └─ PRIMARY BUILD CONFIGURATION
│     - Target: no_such_verb (firmware binary)
│     - Source: main.cpp (single entry point)
│     - Libraries: libDaisy, DaisySP (submodules)
│     - Output: Binary suitable for Patch.init() bootloader
│
├── main.cpp ⭐ ENTRY POINT
│   └─ Main firmware implementation file
│     Size: ~1200 lines
│     Purpose: Complete firmware implementation for Daisy Patch.init()
│     Note: This is the PRIMARY firmware file, NOT a multi-file project
│     Contains:
│       - All DSP algorithm implementations
│       - Hardware configuration
│       - Control/UI logic
│       - Audio processing callback
│
├── setup.sh
│   └─ Build automation script
│     Purpose: Streamline build process, handle dependencies
│
├── .gitignore
│   └─ Git exclusion patterns
│     Excludes: build artifacts, binaries, temporary files
│
├── .gitmodules
│   └─ Git submodule configuration
│     References external libraries managed as submodules
│
├── .github/
│   │
│   └── workflows/
│       │
│       └── build.yml
│           └─ GitHub Actions CI/CD pipeline
│             Purpose: Automated firmware compilation on push/PR
│             Steps: Install CLI tools, compile, artifact storage
│
├── docs/
│   │
│   ├── BUILD.md ⭐ DOCUMENTATION ENTRY POINT
│   │   └─ Build instructions and toolchain setup
│   │     Audience: Developers building firmware
│   │     Contains: Dependency installation, build commands, troubleshooting
│   │
│   ├── DESIGN.md
│   │   └─ Design rationale and high-level architecture
│   │     Audience: Developers understanding project decisions
│   │
│   └── [Generated documentation files]
│       └─ Architecture, development guides, API docs (auto-generated)
│
├── lib/
│   │
│   ├── daisy_midi.h
│   │   └─ MIDI interface header (custom)
│   │     Purpose: Protocol definitions for MIDI communication
│   │     Status: Header only, lightweight
│   │
│   └── daisy_midi.cpp
│       └─ MIDI implementation (custom)
│         Purpose: MIDI message parsing and generation
│         Size: 23 bytes (likely placeholder/stub)
│
├── libDaisy/ 🔗 SUBMODULE
│   │
│   ├── src/
│   │   └─ Daisy hardware abstraction library
│   │     Purpose: Low-level HAL for Daisy Patch.init()
│   │     Contains: GPIO, ADC, DAC, codec drivers
│   │
│   ├── Drivers/
│   │   └─ Hardware device drivers
│   │     Components: I2S codec, ADC peripherals, memory interfaces
│   │
│   ├── core/
│   │   └─ Core system files and linker scripts
│   │
│   ├── Middlewares/
│   │   └─ Third-party middleware (CMSIS, FreeRTOS, etc.)
│   │
│   ├── examples/
│   │   └─ Reference implementations from Daisy ecosystem
│   │
│   ├── tests/
│   │   └─ Daisy library test suite
│   │
│   ├── cmake/
│   │   └─ CMake build configuration (alternative to Makefile)
│   │
│   ├── Makefile
│   │   └─ Build system for libDaisy library itself
│   │
│   ├── build/ (generated)
│   │   └─ Compiled library artifacts
│   │
│   ├── .git/ (local history)
│   │   └─ Git history for submodule
│   │
│   └── doc/
│       └─ Daisy platform documentation
│
├── DaisySP/ 🔗 SUBMODULE
│   │
│   ├── src/
│   │   └─ DaisySP audio DSP library
│   │     Purpose: Pre-built DSP algorithms (oscillators, filters, effects)
│   │     Contains: Audio processing primitives
│   │
│   ├── build/ (generated)
│   │   └─ Compiled DaisySP library
│   │
│   └── [other support files]
│
├── build/ (generated)
│   │
│   ├── firmware.elf
│   │   └─ Compiled firmware (ELF format)
│   │
│   ├── firmware.bin
│   │   └─ Firmware binary (raw binary format)
│   │
│   └── [object files, dependencies]
│
├── .claude/ (AI workspace, untracked)
│   └─ Claude Code configuration and state
│
├── .idea/ (IDE metadata, untracked)
│   └─ JetBrains IDE project settings
│
└── .git/ (version control)
    └─ Git repository metadata and history
```

## Critical Directories and Their Purpose

### 🎯 Primary Application Code

**Location:** Project Root (`/Users/nealsanche/nosuch/no_such_verb/`)

| File | Purpose | Size | Dependency |
|------|---------|------|-----------|
| `main.cpp` | **PRIMARY ENTRY POINT** - All firmware logic | ~1200 LOC | libDaisy, DaisySP |
| `Makefile` | Build configuration targeting Daisy Patch.init() | Config | GNU Make |

**Note:** This is a **single-file firmware project**. All business logic, DSP algorithms, and hardware configuration are contained in `main.cpp`. The build system compiles this single file against libDaisy and DaisySP libraries.

### 📚 Documentation Tree

**Location:** `docs/`

| File | Purpose | Audience |
|------|---------|----------|
| `BUILD.md` | Build instructions, SDK setup, bootloader process | Developers |
| `DESIGN.md` | Design decisions, architecture rationale | Technical leads |
| `[Generated]` | API docs, architecture diagrams, deployment guides | Development team |

**Entry Point:** Start with `docs/BUILD.md` to set up development environment.

### 🔧 External Dependencies (Submodules)

**Location:** `libDaisy/` and `DaisySP/`

| Submodule | Purpose | Type |
|-----------|---------|------|
| `libDaisy` | Daisy Patch.init() hardware abstraction layer | Platform HAL |
| `DaisySP` | Audio DSP algorithm library | DSP Library |

**Init:** Required submodules - initialize with `git submodule update --init --recursive`

### 🏗️ Build Artifacts

**Location:** `build/`

**Contents** (auto-generated during compilation):
- `firmware.elf` - Executable format (debug symbols, larger)
- `firmware.bin` - Binary format (raw firmware, bootloader-ready)
- Object files (`.o`) and dependency files (`.d`)

**Note:** Should be gitignored - regenerated on each build.

## Entry Points for Development

### 1. Starting Development

```
1. Clone repository:
   git clone <repo-url> no_such_verb
   cd no_such_verb

2. Initialize submodules:
   git submodule update --init --recursive

3. Read build documentation:
   cat docs/BUILD.md

4. Build firmware:
   make -j$(sysctl -n hw.ncpu)

5. Program device:
   make program-boot  (or make program-dfu)
```

### 2. Understanding the Architecture

```
docs/DESIGN.md          ← Design decisions and architecture overview
docs/architecture.md    ← Generated detailed architecture documentation
main.cpp               ← Read from the top (includes, globals, setup(), loop())
```

### 3. Modifying Firmware

```
Edit main.cpp:
├─ DSP algorithms (audio processing functions)
├─ Hardware configuration (defines, pins)
├─ Control logic (parameter mapping)
└─ Effect parameters (constants, ranges)

Recompile:
make clean && make -j4

Upload:
make program-boot
```

## File Relationships and Dependencies

```
main.cpp (Firmware)
├─ Includes libDaisy headers
│  └─ daisy_patch_sm.h (Patch.init() hardware API)
│  └─ Drivers/* (GPIO, ADC, I2S drivers)
│  └─ core/* (System core and startup code)
│
├─ Includes DaisySP headers
│  └─ daisysp.h (DSP algorithm library)
│  └─ Contains: Oscillators, Filters, Effects
│
└─ Includes custom lib/
   └─ lib/daisy_midi.h (Custom MIDI support)

Makefile
├─ Includes libDaisy/core/Makefile
│  └─ Defines: Compiler toolchain, optimization flags, linker script
│
└─ Compiles main.cpp against:
   ├─ libDaisy (compiled library)
   └─ DaisySP (compiled library)

GitHub Actions CI
└─ Runs: make clean && make
   └─ Artifacts: firmware.bin
```

## Code Statistics

| Metric | Value | Notes |
|--------|-------|-------|
| **Total Lines (all files)** | ~9,000 LOC | Includes libraries |
| **Main firmware (main.cpp)** | ~1,200 LOC | Core application logic |
| **libDaisy (submodule)** | ~15,000 LOC | Hardware abstraction |
| **DaisySP (submodule)** | ~8,000 LOC | DSP algorithms |
| **Custom library (lib/)** | 23 LOC | Minimal MIDI stub |
| **Build files** | ~50 LOC | Makefile only |
| **Documentation** | ~200 LOC | BUILD.md, DESIGN.md |

## Technology Stack By Directory

```
/
├── main.cpp
│   └─ Language: C++
│      Framework: Daisy Platform
│      Compiler: arm-gcc (STM32H7 ARM Cortex-M7)
│
├── Makefile
│   └─ Build System: GNU Make
│      Language: Shell scripting
│
├── libDaisy/ (submodule)
│   └─ Language: C++/C
│      Hardware: STM32H7 HAL (STMicroelectronics)
│      Features: GPIO, ADC, DAC, I2S, USB, Memory management
│
├── DaisySP/ (submodule)
│   └─ Language: C++
│      Domain: Audio DSP primitives
│      Algorithms: Oscillators, Filters, Delays, Effects
│
├── lib/daisy_midi.*
│   └─ Language: C++
│      Protocol: MIDI (Musical Instrument Digital Interface)
│
└── .github/workflows/build.yml
    └─ Platform: GitHub Actions
       Language: YAML
       Purpose: CI/CD automation
```

## Key Files for Specific Tasks

### To Modify Audio Processing:
1. `main.cpp` - Audio callback function
2. Look for `AudioCallback()` or `update()` equivalent
3. Modify DSP algorithm implementation

### To Add New Hardware Control:
1. `main.cpp` - Hardware setup section
2. Add ADC input initialization
3. Add parameter scaling logic

### To Build/Compile:
1. `Makefile` - Build configuration
2. `libDaisy/core/Makefile` - Compiler settings
3. `docs/BUILD.md` - Step-by-step instructions

### To Debug:
1. `main.cpp` - Add serial output (printf debugging)
2. `build/` - Inspect generated binary and map file
3. Daisy debugger via USB Serial

### To Deploy:
1. `build/firmware.bin` - Binary file to program
2. `docs/BUILD.md` - Bootloader programming steps
3. Teensy Loader or bootloader utility

---

## Summary

This is a **single-file firmware project** with a straightforward structure:

- **Entry Point:** `main.cpp` (all application logic)
- **Build:** `Makefile` targets Daisy Patch.init() platform
- **Dependencies:** Two submodules (libDaisy HAL, DaisySP DSP library)
- **Output:** `build/firmware.bin` (ready for bootloader)
- **Documentation:** `docs/` contains build instructions and design notes

The project prioritizes simplicity and real-time performance over architectural complexity, making it ideal for embedded audio DSP development with tight resource constraints.
