/*
 * Koolertron AMAG23 customizations
 * Copyright and License Terms are pending, 2023, kitcnya@outlook.com
 */

#include QMK_KEYBOARD_H

/*
 * Simulated Modified key
 * ======================
 *
 * kc is pressed then released after the term:
 *           TIMER
 * ------------+-----------------
 * == kc ======|=========
 * == kc1 =====|=========
 *             |== kc2 ==
 * ------------+-----------------
 *             |        `- kc2 and kc1 release by real kc release
 *             `- kc2 press by timer
 *
 * kc is pressed then released with in the term:
 *           TIMER
 * ------------+-----------------
 * == kc ===   |
 * == kc1 ==   |
 * ------------+-----------------
 *         `- kc1 release by real kc release
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
 *                       TIMER
 * ------------------------+-----------------
 * == kc ==                |
 *        == kc1 ==========|
 * ------------------------+-----------------
 *        |                `- kc1 release by timer
 *        `- kc1 press by real kc release
 *
 * kc is pressed then released after the term:
 *                       TIMER
 * ------------------------+-----------------
 * == kc ==================|=========
 *                         |== kc2 ==
 * ------------------------+-----------------
 *                         |        `- kc2 release by real kc release
 *                         `- kc2 press by timer
 *
 * quick retap within the term:
 *                                        TIMER (updated)
 * ----------------+-------:----------------+---------
 * == kc ==        |== kc =:...             |
 *        == kc1 ==|       :                |
 * ----------------+-------:----------------+---------
 *        |        `- kc1 release by real kc repress
 *        `- kc1 press by real kc release
 */

#define TH_TIMER	175

#define THDEF(pkc, pkc1, pkc2)						\
	{								\
		.kc = (pkc),						\
		.kc1 = (pkc1),						\
		.kc2 = (pkc2),						\
		.pending = false,					\
	}

static struct tap_or_hold_def {
	uint16_t kc;			/* keycode to sense */
	uint16_t kc1;			/* primary keycode to emit */
	uint16_t kc2;			/* secondary keycode to emit */
	uint16_t timer;			/* timer on start */
	bool pending;			/* pending action exists on timer */
	bool kc_press;			/* previous kc press state */
} tap_or_hold[] = {
	THDEF(KC_F13, KC_B, KC_G),
	THDEF(KC_F14, KC_J, KC_U),
	THDEF(KC_F15, KC_M, KC_Y),
	THDEF(KC_F16, KC_C, KC_O),
	THDEF(KC_F17, KC_ENT, KC_LALT),
	THDEF(KC_F18, KC_L, KC_V),
	THDEF(KC_F19, KC_BTN1, KC_P),
};

#define NTHDEFS	(sizeof(tap_or_hold) / sizeof(struct tap_or_hold_def))

static void
th_safe_register_kc1(struct tap_or_hold_def *th)
{
	if (th->kc1 != KC_NO) register_code(th->kc1);
}

static void
th_safe_unregister_kc1(struct tap_or_hold_def *th)
{
	if (th->kc1 != KC_NO) unregister_code(th->kc1);
}

static void
th_safe_register_kc2(struct tap_or_hold_def *th)
{
	if (th->kc2 != KC_NO) register_code(th->kc2);
}

static void
th_safe_unregister_kc2(struct tap_or_hold_def *th)
{
	if (th->kc2 != KC_NO) unregister_code(th->kc2);
}

static void
th_process_record(struct tap_or_hold_def *th, keyrecord_t *record)
{
	if (record->event.pressed) {
		if (th->pending) {
			/* kc repressed with in the term */
			th_safe_unregister_kc1(th);
		}
		th->timer = timer_read();
		th->pending = true;
		th->kc_press = true;
	} else {
		if (th->pending) {
			th_safe_register_kc1(th);
		} else {
			th_safe_unregister_kc2(th);
		}
		th->kc_press = false;
	}
}

/*
 * sysmtem interfaces
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

	for (i = 0; i < NSMDEFS; i++) {
		sm = &sim_mod_key[i];
		if (!sm->pending) continue;
		if (timer_elapsed(sm->timer) < SM_TIMER) continue;
		/* timer activated */
		register_code(sm->kc2);
		sm->pending = false;
	}
	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (!th->pending) continue;
		if (timer_elapsed(th->timer) < TH_TIMER) continue;
		/* timer activated */
		if (th->kc_press) {
			th_safe_register_kc2(th);
		} else {
			th_safe_unregister_kc1(th);
		}
		th->pending = false;
	}
}

bool
process_record_user(uint16_t keycode, keyrecord_t *record)
{
	int i;
	struct sim_mod_key_def *sm;
	struct tap_or_hold_def *th;

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
	return true;
}
