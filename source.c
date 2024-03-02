/*
 * Koolertron AMAG23 customizations
 * Copyright (C) 2023 kitcnya@outlook.com
 * https://opensource.org/license/mit/
 * SPDX-License-Identifier: MIT
 */

#include QMK_KEYBOARD_H

/*
 * Simulated Modified key
 * ======================
 *
 * kc is pressed then released after the term:
 *      TIMER -.
 * ------------+-----------------
 * == kc ======|=========|
 * == kc1 =====|=========|
 *             |== kc2 ==|
 * ------------+---------+-------
 *             |         `- kc2 and kc1 release by real kc release
 *             `- kc2 press by timer
 *
 * kc is pressed then released with in the term:
 *      TIMER -.
 * ------------+-----------------
 * == kc ===|  |
 * == kc1 ==|  |
 * ---------+--+-----------------
 *          `- kc1 release by real kc release
 */

#define SM_TIMER	50

#define SMDEF(pkc, pkc1, pkc2)						\
	{								\
		.kc = (pkc),						\
		.kc1 = (pkc1),						\
		.kc2 = (pkc2),						\
		.pending = false,					\
	}

static struct sim_mod_key_def {
	uint16_t kc;			/* keycode to sense */
	uint16_t kc1;			/* modifier keycode */
	uint16_t kc2;			/* target keycode */
	uint16_t timer;			/* timer on start */
	bool pending;			/* pending action exists on timer */
} sim_mod_key[] = {
	SMDEF(KC_F20, KC_LALT, KC_1),
	SMDEF(KC_F21, KC_LALT, KC_2),
	SMDEF(KC_F22, KC_LALT, KC_3),
	SMDEF(KC_F23, KC_LALT, KC_4),
	SMDEF(KC_F24, KC_LALT, KC_5),
};

#define NSMDEFS	(sizeof(sim_mod_key) / sizeof(struct sim_mod_key_def))

static void
sm_timer_action(struct sim_mod_key_def *sm)
{
	if (timer_elapsed(sm->timer) < SM_TIMER) return;
	register_code(sm->kc2);
	sm->pending = false;
}

static void
sm_process_record(struct sim_mod_key_def *sm, keyrecord_t *record)
{
	if (record->event.pressed) {
		register_code(sm->kc1);
		sm->timer = timer_read();
		sm->pending = true;
	} else {
		if (!sm->pending) unregister_code(sm->kc2);
		unregister_code(sm->kc1);
		sm->pending = false;
	}
}

/*
 * Tap or Hold stroking
 * ====================
 *
 * kc is pressed then released with in the term:
 *  WATCH TIMER -.   .- HOLD TIMER (start @ kc release)
 * --------------+---+---------------
 * == kc ==|     |   |  (quick pressed kc within hold time will be ignored)
 * == kc0 =|=========|
 *         |== kc1 ==|
 * --------+-----+---+---------------
 *         |         `- kc1 and kc0 release by hold timer
 *         `- kc1 press by kc release
 *
 * kc is pressed then released after the term:
 *  WATCH TIMER -.
 * --------------+-------------------
 * == kc ========|============|
 * == kc0 =======|============|
 *               |== kc2 =====|  (kc2 can be null keycode)
 * --------------+------------+------
 *               |            `- kc2 and kc0 release by kc release
 *               `- kc2 press by watch timer
 */

#define TH_TIMER	175
#define TH_HOLD		20

#define THDEF(pkc, pkc0, pkc1, pkc2)					\
	{								\
		.kc = (pkc),						\
		.kc0 = (pkc0),						\
		.kc1 = (pkc1),						\
		.kc2 = (pkc2),						\
		.state = TH_WAITING_PRESS,				\
	}

enum tap_or_hold_state {
	TH_WAITING_PRESS,
	TH_WAITING_RELEASE_OR_T1,
	TH_WAITING_PRESS_OR_T2,
	TH_WAITING_RELEASE_OR_T2,
	TH_WAITING_RELEASE_FOR_KC1,
	TH_WAITING_RELEASE_FOR_KC2,
};

static struct tap_or_hold_def {
	uint16_t kc;			/* keycode to sense */
	uint16_t kc0;			/* modifier keycode to emit */
	uint16_t kc1;			/* primary keycode to emit */
	uint16_t kc2;			/* secondary keycode to emit */
	uint16_t timer;			/* timer on start */
	bool pending;			/* pending action exists on timer */
	enum tap_or_hold_state state;
} tap_or_hold[] = {
	THDEF(KC_EXEC, 0, KC_F, KC_BTN1),
	THDEF(KC_F13, 0, KC_B, KC_P),
	THDEF(KC_F14, 0, KC_L, KC_O),
	THDEF(KC_F15, 0, KC_M, KC_Y),
	THDEF(KC_F16, 0, KC_C, KC_G),
	THDEF(KC_F17, 0, KC_J, KC_U),
	THDEF(KC_F18, 0, KC_V, KC_V),
	THDEF(KC_F19, KC_LALT, KC_ENT, 0),
};

#define NTHDEFS	(sizeof(tap_or_hold) / sizeof(struct tap_or_hold_def))

static void
th_timer_action(struct tap_or_hold_def *th)
{
	/* assert(th->pending); */
	switch (th->state) {
	case TH_WAITING_RELEASE_OR_T1:
		if (timer_elapsed(th->timer) < TH_TIMER) return;
		if (th->kc2) register_code(th->kc2);
		th->state = TH_WAITING_RELEASE_FOR_KC2;
		th->pending = false;
		break;
	case TH_WAITING_PRESS_OR_T2:
		if (timer_elapsed(th->timer) < TH_HOLD) return;
		if (th->kc1) unregister_code(th->kc1);
		if (th->kc0) unregister_code(th->kc0);
		th->state = TH_WAITING_PRESS;
		th->pending = false;
		break;
	case TH_WAITING_RELEASE_OR_T2:
		if (timer_elapsed(th->timer) < TH_HOLD) return;
		th->state = TH_WAITING_RELEASE_FOR_KC1;
		th->pending = false;
		break;
	default:
		break;
	}
}

static void
th_process_record(struct tap_or_hold_def *th, keyrecord_t *record)
{
	switch (th->state) {
	case TH_WAITING_PRESS:
		if (!record->event.pressed) break;
		if (th->kc0) register_code(th->kc0);
		th->state = TH_WAITING_RELEASE_OR_T1;
		th->timer = timer_read();
		th->pending = true;
		break;
	case TH_WAITING_RELEASE_OR_T1:
		if (record->event.pressed) break;
		if (th->kc1) register_code(th->kc1);
		th->state = TH_WAITING_PRESS_OR_T2;
		th->timer = timer_read();
		th->pending = true;
		break;
	case TH_WAITING_PRESS_OR_T2:
		if (!record->event.pressed) break;
		th->state = TH_WAITING_RELEASE_OR_T2;
		break;
	case TH_WAITING_RELEASE_OR_T2:
		if (record->event.pressed) break;
		th->state = TH_WAITING_PRESS_OR_T2;
		break;
	case TH_WAITING_RELEASE_FOR_KC1:
		if (record->event.pressed) break;
		if (th->kc1) unregister_code(th->kc1);
		if (th->kc0) unregister_code(th->kc0);
		th->state = TH_WAITING_PRESS;
		break;
	case TH_WAITING_RELEASE_FOR_KC2:
		if (record->event.pressed) break;
		if (th->kc2) unregister_code(th->kc2);
		if (th->kc0) unregister_code(th->kc0);
		th->state = TH_WAITING_PRESS;
		break;
	default:
		break;
	}
}

/*
 * Keycode Macro
 */

#define KM_STEP		10

enum keymac_element_type {
	KM_TYPE_KEY_PRESS,
	KM_TYPE_KEY_RELEASE,
	KM_TYPE_KEY_TAP,
	KM_TYPE_INTERVAL,
	KM_TYPE_REPEAT,			/* start point to repeat */
};

struct keymac_element {
	enum keymac_element_type type;
	union {
		uint16_t keycode;
		uint16_t interval;
	};
};

#define KM_KEY_PRESS(kc)	{ .type = KM_TYPE_KEY_PRESS, .keycode = (kc), }
#define KM_KEY_RELEASE(kc)	{ .type = KM_TYPE_KEY_RELEASE, .keycode = (kc), }
#define KM_KEY_TAP(kc)		{ .type = KM_TYPE_KEY_TAP, .keycode = (kc), }
#define KM_INTERVAL(time)	{ .type = KM_TYPE_INTERVAL, .interval = (time), }
#define KM_REPEAT(time)		{ .type = KM_TYPE_REPEAT, .interval = (time), }

#define KMSEQ(name)	static struct keymac_element name[]

KMSEQ(machine_gun) = {
	KM_REPEAT(150),
	KM_KEY_TAP(KC_BTN1),
	KM_INTERVAL(60),
	KM_KEY_TAP(KC_R),
	KM_INTERVAL(90),
	KM_KEY_TAP(KC_R),
};

KMSEQ(emacs_arg_arg_org_c_star) = {
	KM_KEY_PRESS(KC_LCTL),
	KM_KEY_TAP(KC_U),
	KM_KEY_TAP(KC_U),
	KM_KEY_TAP(KC_C),
	KM_KEY_RELEASE(KC_LCTL),
	KM_KEY_PRESS(KC_LSFT),
	KM_KEY_TAP(KC_8),		/* = '*' asterisk */
	KM_KEY_RELEASE(KC_LSFT),
};

#define KMDEF(pkc, pmac)						\
	{								\
		.kc = (pkc),						\
		.mac = (pmac),						\
		.lmac = (sizeof(pmac) / sizeof(struct keymac_element)),	\
		.pending = false,					\
		.state = KM_WAITING_PRESS,				\
	}

enum keymac_state {
	KM_WAITING_PRESS,
	KM_WAITING_RELEASE_OR_INTERVAL,
	KM_WAITING_PRESS_OR_INTERVAL,
	KM_WAITING_RELEASE,
};

static struct keymac_def {
	uint16_t kc;			/* keycode to sense */
	struct keymac_element *mac;	/* macro sequence */
	uint8_t lmac;			/* number of elements */
	uint16_t timeout;		/* timer timeout value */
	uint16_t timer;			/* timer on start */
	uint8_t ix;			/* macro element index to process */
	uint8_t iy;			/* sub index to process */
	bool pending;			/* pending action exists on timer */
	enum keymac_state state;
} keymac[] = {
	KMDEF(KC_SLCT, machine_gun),
	KMDEF(KC_F9, emacs_arg_arg_org_c_star),
};

#define NKMDEFS	(sizeof(keymac) / sizeof(struct keymac_def))

static uint16_t
km_step(struct keymac_def *km, bool repeat)
{
	struct keymac_element *e;
	uint16_t interval;
	uint8_t i;
	bool adv;

	if (km->ix >= km->lmac) {
		if (!repeat) return 0;
		for (i = 0; i < km->lmac; i++) {
			e = &km->mac[i];
			if (e->type != KM_TYPE_REPEAT) continue;
			km->ix = i + 1;
			km->iy = 0;
			return e->interval;
		}
		return 0;		/* unrepeatable */
	}
	interval = KM_STEP;
	adv = true;
	e = &km->mac[km->ix];
	switch (e->type) {
	case KM_TYPE_KEY_PRESS:
		register_code(e->keycode);
		break;
	case KM_TYPE_KEY_RELEASE:
		unregister_code(e->keycode);
		break;
	case KM_TYPE_KEY_TAP:
		switch (km->iy) {
		case 0:
			register_code(e->keycode);
			adv = false;
			km->iy++;
			break;
		case 1:
			unregister_code(e->keycode);
			break;
		default:
			break;
		}
		break;
	case KM_TYPE_INTERVAL:
		interval = e->interval;
		break;
	default:
		break;
	}
	if (adv) {
		km->ix++;
		km->iy = 0;
	}
	return interval;
}

static void
km_step_action(struct keymac_def *km, bool pressed)
{
	uint16_t interval = km_step(km, pressed);

	if (interval > 0) {
		if (pressed) {
			km->state = KM_WAITING_RELEASE_OR_INTERVAL;
		} else {
			km->state = KM_WAITING_PRESS_OR_INTERVAL;
		}
		km->timeout = interval;
		km->timer = timer_read();
		km->pending = true;
	} else {
		/* end of sequence */
		if (pressed) {
			km->state = KM_WAITING_RELEASE;
		} else {
			km->state = KM_WAITING_PRESS;
		}
		km->pending = false;
	}
}

static void
km_timer_action(struct keymac_def *km)
{
	/* assert(km->pending); */

	if (timer_elapsed(km->timer) < km->timeout) return;
	switch (km->state) {
	case KM_WAITING_RELEASE_OR_INTERVAL:
		km_step_action(km, true);
		break;
	case KM_WAITING_PRESS_OR_INTERVAL:
		km_step_action(km, false);
		break;
	default:
		break;
	}
}

static void
km_process_record(struct keymac_def *km, keyrecord_t *record)
{
	switch (km->state) {
	case KM_WAITING_PRESS:
		if (!record->event.pressed) break;
		km->ix = 0;
		km->iy = 0;
		km_step_action(km, true);
		break;
	case KM_WAITING_RELEASE_OR_INTERVAL:
		if (record->event.pressed) break;
		km->state = KM_WAITING_PRESS_OR_INTERVAL;
		break;
	case KM_WAITING_PRESS_OR_INTERVAL:
		if (!record->event.pressed) break;
		km->state = KM_WAITING_RELEASE_OR_INTERVAL;
		break;
	case KM_WAITING_RELEASE:
		if (record->event.pressed) break;
		km->state = KM_WAITING_PRESS;
		break;
	default:
		break;
	}
}

/*
 * System Interfaces
 *
 * see:
 * - https://docs.qmk.fm/#/custom_quantum_functions
 * - https://docs.qmk.fm/#/feature_macros
 */

void
housekeeping_task_user(void)
{
	int i;
	struct sim_mod_key_def *sm;
	struct tap_or_hold_def *th;
	struct keymac_def *km;

	for (i = 0; i < NSMDEFS; i++) {
		sm = &sim_mod_key[i];
		if (!sm->pending) continue;
		sm_timer_action(sm);
	}
	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (!th->pending) continue;
		th_timer_action(th);
	}
	for (i = 0; i < NKMDEFS; i++) {
		km = &keymac[i];
		if (!km->pending) continue;
		km_timer_action(km);
	}
}

bool
process_record_user(uint16_t keycode, keyrecord_t *record)
{
	int i;
	struct sim_mod_key_def *sm;
	struct tap_or_hold_def *th;
	struct keymac_def *km;

	for (i = 0; i < NSMDEFS; i++) {
		sm = &sim_mod_key[i];
		if (keycode != sm->kc) continue;
		sm_process_record(sm, record);
		return false;
	}
	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (keycode != th->kc) continue;
		th_process_record(th, record);
		return false;
	}
	for (i = 0; i < NKMDEFS; i++) {
		km = &keymac[i];
		if (keycode != km->kc) continue;
		km_process_record(km, record);
		return false;
	}
	return true;
}
