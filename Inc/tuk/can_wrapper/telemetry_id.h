/**
 * @file telemetry_id.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date August 3, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_TELEMETRY_ID_H_
#define TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_TELEMETRY_ID_H_

// NOTE: do not use this type directly. You cannot assume it will be one byte
// wide. Instead, use the TelemetryID type defined farther down below.
enum TelemetryID_ {
	TEL_PCB_TEMP,
	TEL_MCU_TEMP,
	TEL_RSSI,
	TEL_CONVERTER_STATUS,
	TEL_BATTERY_TEMP,
	TEL_BATTERY_VOLTAGE,
	TEL_BATTERY_CURRENT,
	TEL_COULOMB_COUNT,
	TEL_SOLAR_PANEL_TEMP,
	TEL_SOLAR_PANEL_CURRENT,
	TEL_MAGNETIC_FIELD,
	TEL_ANGULAR_VELOCITY,
	TEL_WELL_TEMP,
	TEL_WELL_LUMINOSITY
};

typedef uint8_t TelemetryID; // use this!

// Usage: uint8_t key = CREATE_TELEMETRY_KEY(TEL_PCB_TEMP, 0)
#define CREATE_TELEMETRY_KEY(tel_id, variant_id) (uint8_t)((tel_id << 4) | (0xF & variant_id))

// Usage: uint8_t tel_id = GET_TELEMETRY_ID(key)
// Usage: uint8_t var_id = GET_VARIANT_ID(key)
#define GET_TELEMETRY_ID(tel_key) (TelemetryID)((tel_key & 0xF0) >> 4)
#define GET_VARIANT_ID(tel_key) (uint8_t)((tel_key & 0x0F))

#endif /* TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_TELEMETRY_ID_H_ */
