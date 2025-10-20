#pragma once

#include "daisy_patch_sm.h"
#include "sys/fatfs.h"
#include <cstring>
#include <cstdlib>

using namespace daisy;

/**
 * SD Card-based settings storage for BOOT_QSPI mode
 *
 * This class provides persistent settings storage using the SD card
 * as an alternative to QSPI storage, which cannot be written to when
 * the program executes from QSPI (BOOT_QSPI mode).
 */
template <typename SettingStruct>
class SDSettings
{
public:
    enum class Result
    {
        OK,
        ERR_NO_SD_CARD,
        ERR_MOUNT_FAILED,
        ERR_FILE_ERROR,
        ERR_VERSION_MISMATCH,
    };

    SDSettings(DaisyPatchSM& hardware) : hw_(hardware), initialized_(false)
    {
        // Ensure clean state
        memset(&file_handle_, 0, sizeof(file_handle_));
        memset(filename_, 0, sizeof(filename_));
    }

    /** Initialize SD card and load settings
     *  @param defaults Default settings to use if file doesn't exist
     *  @param filename Settings filename (default: "settings.bin")
     */
    Result Init(const SettingStruct& defaults, const char* filename = "0:/settings.bin")
    {
        // Clean up any previous initialization
        if(initialized_)
        {
            f_mount(nullptr, fsi_.GetSDPath(), 0);  // Unmount
            fsi_.DeInit();
            initialized_ = false;
        }

        settings_ = defaults;
        default_settings_ = defaults;
        strncpy(filename_, filename, sizeof(filename_) - 1);
        filename_[sizeof(filename_) - 1] = '\0';

        // Clear file handle
        memset(&file_handle_, 0, sizeof(file_handle_));

        // Initialize SD card peripheral
        SdmmcHandler::Config sd_cfg;
        sd_cfg.Defaults();
        sd_cfg.speed = SdmmcHandler::Speed::STANDARD;
        sd_cfg.width = SdmmcHandler::BusWidth::BITS_4;

        auto sd_result = sdcard_.Init(sd_cfg);
        if(sd_result != SdmmcHandler::Result::OK)
        {
            return Result::ERR_NO_SD_CARD;
        }

        // Small delay after SD init
        System::Delay(10);

        // Initialize FatFS
        FatFSInterface::Config fs_cfg;
        fs_cfg.media = FatFSInterface::Config::MEDIA_SD;
        auto fs_result = fsi_.Init(fs_cfg);

        if(fs_result != FatFSInterface::OK)
        {
            return Result::ERR_MOUNT_FAILED;
        }

        // Mount the filesystem with immediate mount (parameter = 1)
        FRESULT mount_result = f_mount(&fsi_.GetSDFileSystem(), fsi_.GetSDPath(), 1);
        if(mount_result != FR_OK)
        {
            return Result::ERR_MOUNT_FAILED;
        }

        initialized_ = true;

        // Try to load existing settings - but don't fail init if load fails
        Result load_result = Load();

        // If load failed, we're still initialized - just using defaults
        if(load_result != Result::OK)
        {
            settings_ = defaults;
        }

        return Result::OK;  // Init succeeded even if load failed
    }

    /** Save current settings to SD card */
    Result Save()
    {
        if(!initialized_)
            return Result::ERR_MOUNT_FAILED;

        UINT bytes_written = 0;

        // Ensure filesystem is mounted for write operations - remount if needed
        // This fixes cases where mount may be invalidated between initialization and save
        FRESULT mount_result = f_mount(&fsi_.GetSDFileSystem(), fsi_.GetSDPath(), 1);
        if(mount_result != FR_OK)
        {
            return Result::ERR_MOUNT_FAILED;
        }

        // Open file for writing (create if doesn't exist)
        FRESULT result = f_open(&file_handle_, filename_, FA_WRITE | FA_CREATE_ALWAYS);
        if(result != FR_OK)
        {
            // File open failed - likely permissions or path issue
            return bytes_written == 0 ? Result::ERR_NO_SD_CARD : Result::ERR_FILE_ERROR;
        }

        // Write settings struct
        result = f_write(&file_handle_, &settings_, sizeof(SettingStruct), &bytes_written);
        if(result != FR_OK)
        {
            f_close(&file_handle_);
            // Write operation failed - disk full or hardware error
            return Result::ERR_FILE_ERROR;
        }

        // Check bytes written
        if(bytes_written != sizeof(SettingStruct))
        {
            f_close(&file_handle_);
            // Partial write - disk space issue
            return Result::ERR_FILE_ERROR;
        }

        // Sync before closing to ensure data is written
        result = f_sync(&file_handle_);
        if(result != FR_OK)
        {
            f_close(&file_handle_);
            return Result::ERR_FILE_ERROR;
        }

        // Close file
        result = f_close(&file_handle_);
        if(result != FR_OK)
        {
            return Result::ERR_FILE_ERROR;
        }

        return Result::OK;
    }

    /** Load settings from SD card */
    Result Load()
    {
        if(!initialized_)
            return Result::ERR_MOUNT_FAILED;

        UINT bytes_read = 0;

        // Try to open existing settings file
        FRESULT result = f_open(&file_handle_, filename_, FA_READ);
        if(result != FR_OK)
        {
            // File doesn't exist - use defaults, but DON'T create file yet
            // Let the first Save() call create it
            settings_ = default_settings_;
            return Result::ERR_FILE_ERROR;  // Not OK, but caller can handle it
        }

        // Read settings
        result = f_read(&file_handle_, &settings_, sizeof(SettingStruct), &bytes_read);
        if(result != FR_OK)
        {
            f_close(&file_handle_);
            settings_ = default_settings_;
            return Result::ERR_FILE_ERROR;
        }

        // Check bytes read
        if(bytes_read != sizeof(SettingStruct))
        {
            f_close(&file_handle_);
            settings_ = default_settings_;
            return Result::ERR_FILE_ERROR;
        }

        // Close file
        result = f_close(&file_handle_);
        if(result != FR_OK)
        {
            settings_ = default_settings_;
            return Result::ERR_FILE_ERROR;
        }

        return Result::OK;
    }

    /** Get reference to settings struct */
    SettingStruct& GetSettings() { return settings_; }

    /** Restore factory defaults */
    void RestoreDefaults()
    {
        settings_ = default_settings_;
    }

    bool IsInitialized() const { return initialized_; }

private:
    DaisyPatchSM& hw_;
    SdmmcHandler sdcard_;
    FatFSInterface fsi_;
    FIL file_handle_;  // File handle for FatFS operations
    SettingStruct settings_;
    SettingStruct default_settings_;
    char filename_[64];
    bool initialized_;
};
