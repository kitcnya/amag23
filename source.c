/*
 * Koolertron AMAG23 custimizations
 * Copyright and License Terms are pending, 2023, kitcnya@outlook.com
 */

#include QMK_KEYBOARD_H

enum custom_keycodes {
	CKC_NO = SAFE_RANGE,
	CKC_ALT_1,
	CKC_ALT_2,
	CKC_ALT_3,
	CKC_ALT_4,
	CKC_ALT_5,
	CKC_J_V,
	CKC_B_M,
	CKC_L_C,
	CKC_G_U,
	CKC_Y_O,
};

/*
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

#define TIMER_THRESHOLD	100

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
	THDEF(KC_F20, KC_J, KC_V),
	THDEF(KC_F21, KC_B, KC_M),
	THDEF(KC_F22, KC_L, KC_C),
	THDEF(KC_F23, KC_G, KC_U),
	THDEF(KC_F24, KC_Y, KC_O),
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

void
housekeeping_task_user(void)
{
	int i;
	struct tap_or_hold_def *th;

	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (!th->pending) continue;
		if (timer_elapsed(th->timer) < TIMER_THRESHOLD) continue;
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
	struct tap_or_hold_def *th;

	for (i = 0; i < NTHDEFS; i++) {
		th = &tap_or_hold[i];
		if (keycode != th->kc) continue;
		th_process_record(th, record);
		return false;
	}
	return true;
}
