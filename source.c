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

void
housekeeping_task_user(void)
{
}

bool
process_record_user(uint16_t keycode, keyrecord_t *record)
{
	return true;
}
