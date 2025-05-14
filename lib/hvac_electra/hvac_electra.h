#pragma once

#include <infrared_transmit.h>
#include <infrared_worker.h>

#include "furi_hal.h"

#define HVAC_ELECTRA_PACKET_START_MAGIC (uint8_t)0xD
#define HVAC_ELECTRA_PACKET_SIZE        6
typedef uint8_t* HvacElectraPacket;

#define HVAC_ELECTRA_PACKET_SETTINGS (uint8_t)0x4
#define HVAC_ELECTRA_PACKET_COMMAND  (uint8_t)0xA
#define HVAC_ELECTRA_COMMAND_MAGIC_1 (uint8_t)0xAF
#define HVAC_ELECTRA_COMMAND_MAGIC_2 (uint8_t)0x5
typedef enum {
    HvacElectraPacketSettings,
    HvacElectraPacketCommand,
} HvacElectraPacketType;
HvacElectraPacket hvac_electra_create_packet(HvacElectraPacketType packet_type);
void hvac_electra_free_packet(HvacElectraPacket packet);

#define HVAC_ELECTRA_FAN_POWER_LOCKED 0x0 // necessary to differentiate "dry" and "fan" modes
#define HVAC_ELECTRA_FAN_POWER_1      0x1
#define HVAC_ELECTRA_FAN_POWER_2      0x2
#define HVAC_ELECTRA_FAN_POWER_3      0x4
#define HVAC_ELECTRA_FAN_POWER_AUTO   0x5
#define HVAC_ELECTRA_FAN_POWER_IGNORE 0x6 // necessary to power off, toggle swing
void hvac_electra_set_fan_power_raw(HvacElectraPacket packet, uint8_t fan_power_raw);
typedef enum {
    HvacElectraFanPower1,
    HvacElectraFanPower2,
    HvacElectraFanPower3,
    HvacElectraFanPowerAuto,
} HvacElectraFanPower;
void hvac_electra_set_fan_power(HvacElectraPacket packet, HvacElectraFanPower fan_power);

#define HVAC_ELECTRA_MODE_COLD    0x0
#define HVAC_ELECTRA_MODE_DRY_FAN 0x2 // differentiated by fan power
#define HVAC_ELECTRA_MODE_HEAT    0x3
#define HVAC_ELECTRA_MODE_AUTO    0x1
void hvac_electra_set_mode_raw(HvacElectraPacket packet, uint8_t mode_raw);
typedef enum {
    HvacElectraModeCold,
    HvacElectraModeDry,
    HvacElectraModeFan,
    HvacElectraModeHeat,
    HvacElectraModeAuto,
} HvacElectraMode;
void hvac_electra_set_mode(HvacElectraPacket packet, HvacElectraMode mode);

#define HVAC_ELECTRA_TEMPERATURE_IGNORE 0x7 // necessary to power off, toggle swing
void hvac_electra_set_temperature_raw(HvacElectraPacket packet, uint8_t temperature_raw);
typedef uint8_t HvacElectraTemperature;
#define HVAC_ELECTRA_TEMPERATURE_MIN (HvacElectraTemperature)17
#define HVAC_ELECTRA_TEMPERATURE_DEFAULT \
    (HvacElectraTemperature)22 // just reference value used in packet constructor
static const uint8_t hvac_electra_temperature_table[] = {
    0x0,
    0x8,
    0xC,
    0x4,
    0x6,
    0xE,
    0xA,
    0x2,
    0x3,
    0xB,
    0x9,
    0x1,
    0x5,
    0xD,
}; // index - addition to HVAC_ELECTRA_TEMPERATURE_MIN
#define HVAC_ELECTRA_TEMPERATURE_MAX \
    (HVAC_ELECTRA_TEMPERATURE_MIN +  \
     (HvacElectraTemperature)(sizeof(hvac_electra_temperature_table) / sizeof(uint8_t)) - 1)
void hvac_electra_set_temperature(HvacElectraPacket packet, HvacElectraTemperature temperature);

#define HVAC_ELECTRA_COMMAND_SILENT         0xD // puts into silent mode
#define HVAC_ELECTRA_COMMAND_VERTICAL_SWING 0x6 // toggles vertical swing
#define HVAC_ELECTRA_COMMAND_TURBO          0x8 // activates turbo mode
#define HVAC_ELECTRA_COMMAND_LED            0x14 // toggles led temperature indicator
#define HVAC_ELECTRA_COMMAND_LED_CHANGE_INFO \
    0x4 // toggle between ambient and desired temperature displaying
#define HVAC_ELECTRA_COMMAND_CLEAN 0xA // toggle cleaning mode
void hvac_electra_set_command_raw(HvacElectraPacket packet, uint8_t command_raw);
typedef enum {
    HvacElectraCommandSilent,
    HvacElectraCommandVerticalSwing,
    HvacElectraCommandTurbo,
    HvacElectraCommandLED,
    HvacElectraCommandLEDChangeInfo,
    HvacElectraCommandClean,
} HvacElectraCommand;
void hvac_electra_set_command(HvacElectraPacket packet, HvacElectraCommand command);

void hvac_electra_set_toggle_swing(HvacElectraPacket packet);

void hvac_electra_set_power_off(HvacElectraPacket packet);

#define HVAC_ELECTRA_TRANSMIT_FREQUENCY  38000
#define HVAC_ELECTRA_TRANSMIT_DUTY_CYCLE 0.33
#define HVAC_ELECTRA_TRANSMIT_TIMINGS_PER_PACKET \
    (2 + 2 * HVAC_ELECTRA_PACKET_SIZE * 8 +      \
     2) // start mark-space, packet mark-space per bit, end mark-space
#define HVAC_ELECTRA_HDR_MARK   8000
#define HVAC_ELECTRA_HDR_SPACE  4200
#define HVAC_ELECTRA_BIT_MARK   560
#define HVAC_ELECTRA_ONE_SPACE  1600
#define HVAC_ELECTRA_ZERO_SPACE 560
#define HVAC_ELECTRA_END_MARK   560
#define HVAC_ELECTRA_END_SPACE  5000
typedef struct {
    HvacElectraPacket packet;
    uint8_t repeats;
} HvacElectraSendVec; // in some cases we should send multiple packets simultaneously, i.e. while turning night mode on
void hvac_electra_send_ext(const HvacElectraSendVec* sendvec, size_t sendvec_len);
#define HVAC_ELECTRA_TRANSMIT_REPEATS_DEFAULT 2
void hvac_electra_send(const HvacElectraPacket packet);
