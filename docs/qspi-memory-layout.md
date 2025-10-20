# QSPI Memory Layout for Daisy Patch.init()

## Memory Map Overview

The Daisy Patch.init() contains 8MB of QSPI flash memory organized as follows:

```
STM32H7 QSPI Memory Map (8MB Total)
=====================================

0x90000000  ┌─────────────────────────────────┐
            │     BOOTLOADER RESERVED         │
            │        (256KB / 4 Sectors)      │ ← DO NOT USE
            │                                 │
0x90040000  ├─────────────────────────────────┤
            │     APPLICATION FIRMWARE        │
            │         (~46KB used)            │ ← Current firmware
            │      (7936KB available)         │
            │                                 │
            │  ┌─ Current Settings Storage     │
0x902B000   │  │    (172KB offset)           │ ← storage.Init(_, 0x2B000)
            │  └─ ACCEPTABLE but sub-optimal  │
            │                                 │
0x90080000  │  ┌─ RECOMMENDED Settings        │ ← Safer location
            │  │    (512KB offset)            │
            │  └─ Better safety margin        │
            │                                 │
            │     AVAILABLE FOR DATA          │
            │      (~7.5MB remaining)         │
            │                                 │
0x90800000  └─────────────────────────────────┘
```

## Detailed Address Ranges

### Bootloader Reserved Region
- **Start Address**: `0x90000000`
- **End Address**: `0x90040000` (exclusive)
- **Size**: 256KB (4 × 64KB sectors)
- **Purpose**: Bootloader metadata and future features
- **Access**: **FORBIDDEN** - Applications must not write here
- **Documentation Reference**: "The first four 64kB sectors are left untouched"

### Application Firmware Region
- **Start Address**: `0x90040000`
- **Current Size**: ~46KB (116KB compiled size)
- **Available Size**: 7936KB (from linker script)
- **Purpose**: Executable code and read-only data
- **Linker Section**: `.text`, `.rodata`, `.isr_vector`
- **Growth Room**: 7890KB available for firmware expansion

### Application Data Region
- **Start Address**: `0x90040000` + firmware_size
- **Current Usage**: Settings at offset 0x2B000 (172KB)
- **Absolute Address**: `0x90040000 + 0x2B000 = 0x906B000`
- **Available Space**: ~7.5MB for user data
- **Recommended Start**: `0x90080000` (512KB offset)

## Sector Organization

### QSPI Flash Sectors (64KB each)
```
Sector  Address Range               Purpose                Status
------  --------------------------  ---------------------  ----------
0       0x90000000 - 0x90010000     Bootloader Reserved    Protected
1       0x90010000 - 0x90020000     Bootloader Reserved    Protected
2       0x90020000 - 0x90030000     Bootloader Reserved    Protected
3       0x90030000 - 0x90040000     Bootloader Reserved    Protected
4       0x90040000 - 0x90050000     Firmware Code          In Use
5       0x90050000 - 0x90060000     Firmware/Data          Available
6       0x90060000 - 0x90070000     Current Settings       In Use
7       0x90070000 - 0x90080000     Data                   Available
8       0x90080000 - 0x90090000     Recommended Settings   Preferred
9-127   0x90090000 - 0x90800000     Available Data         Free
```

### Sector-Aligned Addressing
- **Erase Granularity**: 64KB sectors (4KB minimum)
- **Write Granularity**: 256-byte pages
- **PersistentStorage**: Automatically aligns to 256-byte boundaries
- **Current Address**: 0x2B000 → aligned to 0x2B000 (already aligned)

## Memory Usage Analysis

### Current Memory Utilization
```
Region                  Start        Size      Used    Available
-------------------    ----------   --------   -----   ----------
Bootloader Reserved    0x90000000   256KB      100%    0KB
Application Firmware   0x90040000   7936KB     0.57%   7890KB
Settings Storage       0x902B000    <1KB       100%    N/A
Available Data Space   0x902C000    ~7.5MB     0%      ~7.5MB
```

### Storage Growth Projections
- **Current Settings**: ~32 bytes (Settings struct)
- **Storage Overhead**: +4 bytes (PersistentStorage state)
- **Sector Usage**: <1KB in 64KB sector (efficient)
- **Growth Room**: Can expand settings struct significantly

## Address Recommendations

### Safe Address Ranges for PersistentStorage

1. **Current Address** (Acceptable but improvable):
   ```cpp
   storage.Init(defaults, 0x2B000);  // 172KB offset
   // Absolute: 0x906B000
   // Pro: Works, tested
   // Con: Close to firmware, less safety margin
   ```

2. **Recommended Address** (Better safety):
   ```cpp
   storage.Init(defaults, 0x80000);  // 512KB offset
   // Absolute: 0x90080000
   // Pro: Large safety margin, sector-aligned
   // Con: Requires migration of existing data
   ```

3. **Conservative Address** (Maximum safety):
   ```cpp
   storage.Init(defaults, 0x100000); // 1MB offset
   // Absolute: 0x90140000
   // Pro: Maximum safety margin for firmware growth
   // Con: "Wastes" space, overkill for current needs
   ```

### Address Selection Criteria
- **Minimum Offset**: Firmware size + 64KB safety margin
- **Alignment**: 256-byte aligned (automatic in PersistentStorage)
- **Sector Boundary**: Preferably start at sector boundary
- **Future Growth**: Allow for firmware expansion

## Boundary Calculations

### Safe Minimum Calculation
```
firmware_current = 46KB
firmware_growth_buffer = 256KB  (conservative estimate)
sector_alignment = 64KB
safety_margin = 64KB

minimum_safe_offset = ALIGN_UP(
    firmware_current + firmware_growth_buffer + safety_margin,
    64KB
) = ALIGN_UP(46 + 256 + 64, 64) = ALIGN_UP(366KB, 64KB) = 384KB = 0x60000

recommended_offset = 512KB = 0x80000  (exceeds minimum)
```

### Current vs Recommended Comparison
```
Current Implementation:
├─ Firmware:  0x90040000 - 0x9004B000  (46KB)
├─ Gap:       0x9004B000 - 0x9006B000  (128KB)
└─ Settings:  0x9006B000 - 0x9006C000  (1KB)

Recommended Implementation:
├─ Firmware:  0x90040000 - 0x9004B000  (46KB)
├─ Growth:    0x9004B000 - 0x900C0000  (468KB gap)
└─ Settings:  0x900C0000 - 0x900C1000  (1KB)
```

## Bootloader Memory Coordination

### Bootloader Access Patterns
- **Read Access**: Firmware verification, checksum calculation
- **Write Access**: Firmware updates from SD card/USB
- **Timing**: During 2.5-second grace period
- **Regions**: Writes only to firmware region (0x90040000+)

### Application Coordination Requirements
- **Timing**: Wait 3+ seconds before QSPI access
- **Regions**: Avoid bootloader reserved space (0x90000000-0x90040000)
- **Mode**: PersistentStorage uses memory-mapped mode safely
- **Conflict**: None if timing requirements met

### Safe Initialization Sequence
```cpp
int main() {
    hw.Init();                    // Hardware setup

    System::Delay(3000);          // Wait for bootloader completion

    storage.Init(defaults,        // Safe to access QSPI now
                 0x80000);        // Use recommended address

    // Continue normal initialization...
}
```

## Migration Considerations

### Moving from Current Address (0x2B000) to Recommended (0x80000)

1. **Read existing settings** from 0x2B000
2. **Write settings** to new address 0x80000
3. **Erase old location** (optional cleanup)
4. **Update firmware** to use new address
5. **Test** settings persistence across reboots

### Migration Code Pattern
```cpp
// Migration helper (run once)
void MigrateSettingsStorage() {
    PersistentStorage<Settings> old_storage(hw.qspi);
    PersistentStorage<Settings> new_storage(hw.qspi);

    // Read from old location
    old_storage.Init(defaults, 0x2B000);
    Settings existing = old_storage.GetSettings();

    // Write to new location
    new_storage.Init(defaults, 0x80000);
    new_storage.GetSettings() = existing;
    new_storage.Save();
}
```

## Verification and Testing

### Address Range Verification
```cpp
// Compile-time checks for safe addressing
static_assert(SETTINGS_ADDRESS >= 0x40000,
             "Settings address must be beyond bootloader reserved space");
static_assert(SETTINGS_ADDRESS % 256 == 0,
             "Settings address must be 256-byte aligned");
```

### Runtime Validation
```cpp
void ValidateMemoryLayout() {
    void* qspi_base = hw.qspi.GetData(0);
    void* settings_ptr = hw.qspi.GetData(SETTINGS_ADDRESS);

    assert((uint32_t)settings_ptr >= 0x90040000); // Beyond bootloader space
    assert(((uint32_t)settings_ptr & 0xFF) == 0); // 256-byte aligned
}
```

## Summary

The current QSPI memory layout is functional but has room for improvement:

- **Current setup works** but uses suboptimal address spacing
- **Bootloader coordination** is the primary issue (timing, not addressing)
- **Recommended address** (0x80000 offset) provides better safety margins
- **Migration path** exists to move to better addresses if desired
- **Critical fix** is adding bootloader completion delay, not address changes