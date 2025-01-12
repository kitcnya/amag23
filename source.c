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
			val = rgblight_get_sat();
		}
		switch (top) {
		case 1:
			rgblight_sethsv(HSV_BLUE);
			break;
		case 2:
			rgblight_sethsv(HSV_AZURE);
			break;
		case 3:
			rgblight_sethsv(HSV_CYAN);
			break;
		case 4:
			rgblight_sethsv(HSV_SPRINGGREEN);
			break;
		case 5:
			rgblight_sethsv(HSV_GREEN);
			break;
		case 6:
			rgblight_sethsv(HSV_CHARTREUSE);
			break;
		case 7:
			rgblight_sethsv(HSV_YELLOW);
			break;
		case 8:
			rgblight_sethsv(HSV_RED);
			break;
		case 9:
			rgblight_sethsv(HSV_PINK);
			break;
		case 10:
			rgblight_sethsv(HSV_MAGENTA);
			break;
		case 12:
			rgblight_sethsv(HSV_PURPLE);
			break;
		default:
			rgblight_sethsv(hue, sat, val);
			break;
		}
		top_prev = top;
	}
}

bool
process_record_user(uint16_t keycode, keyrecord_t *record)
{
	return true;
}
