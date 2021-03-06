#include "modeldata.h"
#include "ModeDataFiles/defaultM1SeriesBootCode.h"
#include "ModeDataFiles/defaultT1SeriesBootCode.h"
#include "ModeDataFiles/defaultS51evflBootCode.h"
#include "ModeDataFiles/defaultA13tevflBootCode.h"
#include "ModeDataFiles/defaultEraseEepromCode.h"
#include "ModeDataFiles/defaultFlashDriverCode.h"

const char* ModelData::BOOT_CODE_M1AFL2 = DEFAULT_M1_SERIES_BOOT_CODE;
const char* ModelData::BOOT_CODE_T19 = DEFAULT_T1_SERIES_BOOT_CODE;
const char* ModelData::BOOT_CODE_S51EVFL = DEFAULT_S51EVFL_BOOT_CODE;
const char* ModelData::BOOT_CODE_A13TEV = DEFAULT_A13TEV_BOOT_CODE;
const char* ModelData::ERASE_EEPROM_FIRMWARE = DEFAULT_ERASE_EEPROM_CODE;
const char* ModelData::FLASH_DRIVER_NO_PART_NUMBER = DEFAULT_CHERY_COMMON_FLASHDRIVER_CODE;

ModelData::ModelData()
{
}

