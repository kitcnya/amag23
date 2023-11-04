#include QMK_KEYBOARD_H

/* tap dance declarations */
enum {
    TD_L_C,
    TD_G_U,
    TD_Y_O,
    TD_B_M,
    TD_J_V,
};

/* tap dance definitions */
tap_dance_action_t tap_dance_actions[] = {
    [TD_L_C] = ACTION_TAP_DANCE_DOUBLE(KC_L, KC_C),
    [TD_G_U] = ACTION_TAP_DANCE_DOUBLE(KC_G, KC_U),
    [TD_Y_O] = ACTION_TAP_DANCE_DOUBLE(KC_Y, KC_O),
    [TD_B_M] = ACTION_TAP_DANCE_DOUBLE(KC_B, KC_M),
    [TD_J_V] = ACTION_TAP_DANCE_DOUBLE(KC_J, KC_V),
};
