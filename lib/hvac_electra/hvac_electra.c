#include "hvac_electra.h"

HvacElectraPacket hvac_electra_create_packet(HvacElectraPacketType packet_type) {
    HvacElectraPacket packet;

    switch(packet_type) {
    case HvacElectraPacketSettings:
        packet = (HvacElectraPacket)malloc(sizeof(uint8_t) * HVAC_ELECTRA_PACKET_SIZE);
        furi_assert(packet);

        packet[0] = HVAC_ELECTRA_PACKET_SETTINGS << 4 | HVAC_ELECTRA_PACKET_START_MAGIC;
        packet[1] = ~(HVAC_ELECTRA_PACKET_SETTINGS << 4 | HVAC_ELECTRA_PACKET_START_MAGIC);
        packet[2] = 0xFF;
        packet[3] = 0x0;
        packet[4] = 0x0;
        packet[5] = 0xFF;

        hvac_electra_set_mode(packet, HvacElectraModeCold);
        hvac_electra_set_temperature(packet, HVAC_ELECTRA_TEMPERATURE_DEFAULT);

        return packet;
    case HvacElectraPacketCommand:
        packet = (HvacElectraPacket)malloc(sizeof(uint8_t) * HVAC_ELECTRA_PACKET_SIZE);
        furi_assert(packet);

        packet[0] = HVAC_ELECTRA_PACKET_COMMAND << 4 | HVAC_ELECTRA_PACKET_START_MAGIC;
        packet[1] = (uint8_t) ~(HVAC_ELECTRA_PACKET_COMMAND << 4 | HVAC_ELECTRA_PACKET_START_MAGIC);
        packet[2] = HVAC_ELECTRA_COMMAND_MAGIC_1;
        packet[3] = (uint8_t)~HVAC_ELECTRA_COMMAND_MAGIC_1;
        packet[4] = HVAC_ELECTRA_COMMAND_MAGIC_2;
        packet[5] = ~HVAC_ELECTRA_COMMAND_MAGIC_2;

        return packet;
    default:
        furi_assert(false);
        break;
    }

    return NULL;
}

void hvac_electra_free_packet(HvacElectraPacket packet) {
    furi_assert(packet);
    free(packet);
}

void hvac_electra_set_fan_power_raw(HvacElectraPacket packet, uint8_t fan_power_raw) {
    furi_assert(packet);

    packet[2] &= ~(uint8_t)0x7;
    packet[2] |= fan_power_raw & 0x7;

    packet[3] |= (uint8_t)0x7;
    packet[3] &= ~(fan_power_raw & 0x7);
}

void hvac_electra_set_mode_raw(HvacElectraPacket packet, uint8_t mode_raw) {
    furi_assert(packet);

    packet[4] &= ~(uint8_t)0x30;
    packet[4] |= (mode_raw & 0x3) << 4;

    packet[5] |= (uint8_t)0x30;
    packet[5] &= ~((mode_raw & 0x3) << 4);
}

void hvac_electra_set_mode(HvacElectraPacket packet, HvacElectraMode mode) {
    furi_assert(packet);

    switch(mode) {
    case HvacElectraModeCold:
        hvac_electra_set_mode_raw(packet, HVAC_ELECTRA_MODE_COLD);
        break;
    case HvacElectraModeDry:
        hvac_electra_set_mode_raw(packet, HVAC_ELECTRA_MODE_DRY_FAN);
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_LOCKED);
        break;
    case HvacElectraModeFan:
        hvac_electra_set_mode_raw(packet, HVAC_ELECTRA_MODE_DRY_FAN);
        hvac_electra_set_temperature_raw(packet, HVAC_ELECTRA_TEMPERATURE_IGNORE);
        break;
    case HvacElectraModeHeat:
        hvac_electra_set_mode_raw(packet, HVAC_ELECTRA_MODE_HEAT);
        break;
    case HvacElectraModeAuto:
        hvac_electra_set_mode_raw(packet, HVAC_ELECTRA_MODE_AUTO);
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_LOCKED);
        break;
    default:
        furi_assert(false);
        break;
    }
}

void hvac_electra_set_fan_power(HvacElectraPacket packet, HvacElectraFanPower fan_power) {
    furi_assert(packet);

    switch(fan_power) {
    case HvacElectraFanPower1:
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_1);
        break;
    case HvacElectraFanPower2:
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_2);
        break;
    case HvacElectraFanPower3:
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_3);
        break;
    case HvacElectraFanPowerAuto:
        hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_AUTO);
        break;
    default:
        furi_assert(false);
        break;
    }
}

void hvac_electra_set_temperature_raw(HvacElectraPacket packet, uint8_t temperature_raw) {
    furi_assert(packet);

    packet[4] &= ~(uint8_t)0xF;
    packet[4] |= temperature_raw & 0xF;

    packet[5] |= (uint8_t)0xF;
    packet[5] &= ~(temperature_raw & 0xF);
}

void hvac_electra_set_temperature(HvacElectraPacket packet, HvacElectraTemperature temperature) {
    furi_assert(packet);
    furi_assert(
        (temperature >= HVAC_ELECTRA_TEMPERATURE_MIN) &&
        (temperature <= HVAC_ELECTRA_TEMPERATURE_MAX));

    hvac_electra_set_temperature_raw(
        packet, hvac_electra_temperature_table[temperature - HVAC_ELECTRA_TEMPERATURE_MIN]);
}

void hvac_electra_set_command_raw(HvacElectraPacket packet, uint8_t command_raw) {
    furi_assert(packet);

    packet[4] |= (command_raw & 0x1F) << 3;
    packet[5] &= ~((command_raw & 0x1F) << 3);
}

void hvac_electra_set_command(HvacElectraPacket packet, HvacElectraCommand command) {
    furi_assert(packet);

    switch(command) {
    case HvacElectraCommandSilent:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_SILENT);
        break;
    case HvacElectraCommandVerticalSwing:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_VERTICAL_SWING);
        break;
    case HvacElectraCommandTurbo:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_TURBO);
        break;
    case HvacElectraCommandLED:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_LED);
        break;
    case HvacElectraCommandLEDChangeInfo:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_LED_CHANGE_INFO);
        break;
    case HvacElectraCommandClean:
        hvac_electra_set_command_raw(packet, HVAC_ELECTRA_COMMAND_CLEAN);
        break;
    default:
        furi_assert(false);
    }
}

void hvac_electra_set_toggle_swing(HvacElectraPacket packet) {
    furi_assert(packet);

    hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_IGNORE);
    hvac_electra_set_temperature_raw(packet, HVAC_ELECTRA_TEMPERATURE_IGNORE);

    // power off / swing flag
    packet[2] &= ~(uint8_t)0x20;
    packet[3] |= (uint8_t)0x20;

    // select swing
    packet[2] &= ~(uint8_t)0x8;
    packet[3] |= (uint8_t)0x8;
}

void hvac_electra_set_power_off(HvacElectraPacket packet) {
    furi_assert(packet);

    hvac_electra_set_fan_power_raw(packet, HVAC_ELECTRA_FAN_POWER_IGNORE);
    hvac_electra_set_temperature_raw(packet, HVAC_ELECTRA_TEMPERATURE_IGNORE);

    // power off / swing flag
    packet[2] &= ~(uint8_t)0x20;
    packet[3] |= (uint8_t)0x20;

    // select power off
    packet[2] |= (uint8_t)0x8;
    packet[3] &= ~(uint8_t)0x8;
}

void hvac_electra_send_ext(const HvacElectraSendVec* sendvec, size_t sendvec_len) {
    furi_assert(sendvec);
    furi_assert(sendvec_len);

    size_t packets_amount = 0;
    for(size_t i = 0; i < sendvec_len; i++) {
        furi_assert(sendvec[i].packet);
        furi_assert(sendvec[i].repeats);
        packets_amount += (size_t)sendvec[i].repeats;
    }

    size_t timings_len = packets_amount * HVAC_ELECTRA_TRANSMIT_TIMINGS_PER_PACKET;
    furi_assert(timings_len <= MAX_TIMINGS_AMOUNT);

    uint32_t* timings = malloc(sizeof(uint32_t) * timings_len);
    furi_assert(timings);

    size_t timings_idx = 0;
    for(size_t i = 0; i < sendvec_len; i++) {
        for(uint8_t j = 0; j < sendvec[i].repeats; j++) {
            // header
            timings[timings_idx++] = HVAC_ELECTRA_HDR_MARK;
            timings[timings_idx++] = HVAC_ELECTRA_HDR_SPACE;

            // packet bytes
            for(uint8_t packet_byte_idx = 0; packet_byte_idx < HVAC_ELECTRA_PACKET_SIZE;
                packet_byte_idx++) {
                uint8_t packet_byte = sendvec[i].packet[packet_byte_idx];

                for(uint8_t mask = 1; mask > 0; mask <<= 1) {
                    timings[timings_idx++] = HVAC_ELECTRA_BIT_MARK;
                    timings[timings_idx++] = (packet_byte & mask) ? HVAC_ELECTRA_ONE_SPACE :
                                                                    HVAC_ELECTRA_ZERO_SPACE;
                }
            }

            // ending
            timings[timings_idx++] = HVAC_ELECTRA_END_MARK;
            timings[timings_idx++] = HVAC_ELECTRA_END_SPACE;
        }
    }

    infrared_send_raw_ext(
        timings, timings_len, true, HVAC_ELECTRA_TRANSMIT_FREQUENCY, HVAC_ELECTRA_TRANSMIT_DUTY_CYCLE);
    free(timings);
}

void hvac_electra_send(const HvacElectraPacket packet) {
    HvacElectraSendVec sendvec[] = {
        {.packet = packet, .repeats = HVAC_ELECTRA_TRANSMIT_REPEATS_DEFAULT},
    };

    hvac_electra_send_ext(sendvec, sizeof(sendvec) / sizeof(HvacElectraSendVec));
}
