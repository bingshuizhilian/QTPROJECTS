#ifndef MODELDATA_H
#define MODELDATA_H

#if defined(MODELDATA_LIBRARY)
#  define MODELDATASHARED_EXPORT __declspec(dllexport)
#else
#  define MODELDATASHARED_EXPORT __declspec(dllimport)
#endif

class MODELDATASHARED_EXPORT ModelData
{

public:
    ModelData();

public:
    //boot related
    static const char* BOOT_CODE_M1AFL2;
    static const char* BOOT_CODE_T19;
    static const char* BOOT_CODE_S51EVFL;
    static const char* BOOT_CODE_A13TEV;

    //erase eeprom firmware
    static const char* ERASE_EEPROM_FIRMWARE;

    //flash driver
    static const char* FLASH_DRIVER_M1AFL2;
    static const char* FLASH_DRIVER_T19;
    static const char* FLASH_DRIVER_S51EVFL;
    static const char* FLASH_DRIVER_A13TEV;
    static const char* FLASH_DRIVER_NO_PART_NUMBER;
};

#endif // MODELDATA_H
