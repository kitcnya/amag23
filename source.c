/*
 * Koolertron AMAG23 custimizations
 * Copyright and License Terms are pending, 2023, kitcnya@outlook.com
 */

#include QMK_KEYBOARD_H

/*
 * Special Modifiered Key
 * ======================
 */

#define SMK_TIMER	50

#define SMKDEF(pkc, pkc1, pkc2)					\
	{								\
		.kc = (pkc),						\
		.kc1 = (pkc1),						\
		.kc2 = (pkc2),						\
		.pending = false,					\
	}

static struct spc_mod_key_def {
	uint16_t kc;			/* keycode to sense */
	uint16_t kc1;			/* modifier keycode */
	uint16_t kc2;			/* target keycode */
	uint16_t timer;			/* timer on start */
	bool pending;			/* pending action exists on timer */
} spc_mod_key[] = {
	SMKDEF(KC_F20, KC_LALT, KC_1),
	SMKDEF(KC_F21, KC_LALT, KC_2),
	SMKDEF(KC_F22, KC_LALT, KC_3),
	SMKDEF(KC_F23, KC_LALT, KC_4),
	SMKDEF(KC_F24, KC_LALT, KC_5),
};

#define NSMKDEFS	(sizeof(spc_mod_key) / sizeof(struct spc_mod_key_def))

static void
smk_process_record(struct spc_mod_key_def *smk, keyrecord_t *record)
{
	if (record->event.pressed) {
		register_code(smk->kc1);
		smk->timer = timer_read();
		smk->pending = true;
	} else {
		if (!smk->pending) unregister_code(smk->kc2);
		unregister_code(smk->kc1);
		smk->pending = false;
	}
}

/*
 * Tap or Hold stroking
 * ====================
 *
 * kc is pressed then released with in the term:
 *                 TIMER_THRESHOLD
 * ------------------------+-----------------
 * == kc ==                |
 *        == kc1 ==========|
 * ------------------------+-----------------
 *        |                `- kc1 release by timer
 *        `- kc1 press by real kc release
 *
 * kc is pressed then released after the term:
 *                 TIMER_THRESHOLD
 * ------------------------+-----------------
 * == kc ==================|=========
 *                         |== kc2 ==
 * ------------------------+-----------------
 *                         |        `- kc2 release by real kc release
 *                         `- kc2 press by timer
 *
 * quick retap within the term:
 *                                  TIMER_THRESHOLD (timer updated)
 * ----------------+-------:----------------+---------
 * == kc ==        |== kc ==...             |
 *        == kc1 ==|                        |
 * ----------------+-------:----------------+---------
 *        |        `- kc1 release by real kc repress
 *        `- kc1 press by real kc release
 *
 * see:
 * - https://docs.qmk.fm/#/custom_quantum_functions
 * - https://docs.qmk.fm/#/feature_macros
 */

#define TH_TIMER_THRESHOLD	175

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
	THDEF(KC_F13, KC_B, KC_M),
	THDEF(KC_F14, KC_J, KC_V),
	THDEF(KC_F15, KC_G, KC_U),
	THDEF(KC_F16, KC_Y, KC_O),
	THDEF(KC_F17, KC_L, KC_C),
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
 */

void
housekeeping_task_user(void)
{
	int i;
	struct spc_mod_key_def *smk;
	struct tap_or_hold_def *th;

	for (i = 0; i < NSMKDEFS; i++) {
		smk = &spc_mod_key[i];
		if (!smk->pending) continue;
		if (timer_elapsed(smk->timer) < SMK_TIMER) continue;
		/* timer activated */
		register_code(smk->kc2);
		smk->pending = false;
	}
	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (!th->pending) continue;
		if (timer_elapsed(th->timer) < TH_TIMER_THRESHOLD) continue;
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
	struct spc_mod_key_def *smk;
	struct tap_or_hold_def *th;

	for (i = 0; i < NSMKDEFS; i++) {
		smk = &spc_mod_key[i];
		if (keycode != smk->kc) continue;
		smk_process_record(smk, record);
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
