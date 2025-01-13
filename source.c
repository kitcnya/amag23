/*
 * Koolertron AMAG23 customizations
 * Copyright (C) 2025 kitcnya@outlook.com
 * https://opensource.org/license/mit/
 * SPDX-License-Identifier: MIT
 */

#include QMK_KEYBOARD_H

/*
 * System Interfaces
 */

#define LAYER_RGB_SETTING	2
#define RGB_HUE_STEP	32

void
housekeeping_task_user(void)
{
	static uint8_t top_prev = LAYER_RGB_SETTING;
	static uint8_t hue, sat, val;
	uint8_t top = get_highest_layer(layer_state);

	if (top != top_prev) {
		if (top_prev == LAYER_RGB_SETTING) {
			hue = rgblight_get_hue();
			sat = rgblight_get_sat();
			val = rgblight_get_val();
		}
		if (top != LAYER_RGB_SETTING) {
			uint8_t h = hue + RGB_HUE_STEP * top;
			if (top > LAYER_RGB_SETTING) {
				h -= RGB_HUE_STEP;
			}
			rgblight_sethsv(h, sat, val);
		}
		top_prev = top;
	}
}

bool
process_record_user(uint16_t keycode, keyrecord_t *record)
{
	return true;
}
