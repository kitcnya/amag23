<!-- -*- mode: markdown; coding: utf-8 -*- -->

# Koolertron AMAG23 keyboard

![Koolertron AMAG23 w/DSA profile keycaps](amag23.png)

  - [Motivations](#motivations)
  - [Keymap compression approach](#keymap-compression-approach)
  - [Simulated Modified key](#simulated-modified-key)
  - [Tap or Hold stroking](#tap-or-hold-stroking)
  - [Latest keymap](#latest-keymap)

## Motivations

We can use this small keyboard immediately after purchasing it
by using the included software to define your preferred key layout.
For this reason, I think many people love using it for PC games.
For a while, I also used the included software to play games
such as the PC version of [Genshin Impact](https://genshin.hoyoverse.com/).

In fact, it's comfortable to play the game with this keyboard,
which has been adjusted to my preferred keymap.
However, there are some problems, just like:

  - Standard Alt + number combination key definition doesn't work,
  - Three layers are required to map all functions,
    resulting in a lot of loss when switching layers, and so on.

The former could be handled by macro definitions,
but nothing could be done about the latter.

Therefore, while looking for a way to compress keymaps,
then I learned about [QMK Firmware](https://docs.qmk.fm/), and was inspired by its approach
to multifunctionality for various compact keyboards,
so I decided to try QMK.

## Keymap compression approach

To reduce keymap layers, we need to assign multiple functions
(command characters in games) to a single key.
QMK tap dance and tap hold features are good options for this.
Key combination of normal key and layer switching key is very unique.

However, it seems that the use of custom key codes is restricted
in the github user environment, so tap dance cannot be used,
and tap hold can only be used in combination with modifier keys
such as Shift and Ctrl.

Therefore, I decided to rewrite the behavior of existing key codes that are not used.
I used the following as a reference.

  - [Customizing Functionality](https://docs.qmk.fm/#/custom_quantum_functions)
  - [Macros](https://docs.qmk.fm/#/feature_macros)

The key codes used are F13, F14,..., F24 function keys.
I changed these by overriding the `process_record_user()` function.

Note: Defining a macro in a *.json file,
the `process_record_user()` function will result in a double definition error,
so I have also implemented the Alt + number function.

## Simulated Modified key

```
kc is pressed then released after the term:
          TIMER
------------+-----------------
== kc ======|=========
== kc1 =====|=========
            |== kc2 ==
------------+-----------------
            |        `- kc2 and kc1 release by real kc release
            `- kc2 press by timer

kc is pressed then released with in the term:
          TIMER
------------+-----------------
== kc ===   |
== kc1 ==   |
------------+-----------------
        `- kc1 release by real kc release
```

```
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
```

## Tap or Hold stroking

```
kc is pressed then released with in the term:
                      TIMER
------------------------+-----------------
== kc ==                |
       == kc1 ==========|
------------------------+-----------------
       |                `- kc1 release by timer
       `- kc1 press by real kc release

kc is pressed then released after the term:
                      TIMER
------------------------+-----------------
== kc ==================|=========
                        |== kc2 ==
------------------------+-----------------
                        |        `- kc2 release by real kc release
                        `- kc2 press by timer

quick retap within the term:
                                       TIMER (updated)
----------------+-------:----------------+---------
== kc ==        |== kc =:...             |
       == kc1 ==|       :                |
----------------+-------:----------------+---------
       |        `- kc1 release by real kc repress
       `- kc1 press by real kc release
```

```
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
	THDEF(KC_F13, KC_B, KC_M),
	THDEF(KC_F14, KC_J, KC_V),
	THDEF(KC_F15, KC_G, KC_U),
	THDEF(KC_F16, KC_Y, KC_O),
	THDEF(KC_F17, KC_L, KC_C),
};
```

## Latest keymap

### Layer 0

A or B means tapping key for A, or holding key for B.

```
+------------+--------+--------+--------+--------+-------------+
|    ESC     | B or M | J or V | G or U | Y or O | ENT or LALT |
+------------+--------+--------+--------+--------+-------------+
|   L or C   |   Q    |   E    |   R    |   Z    |     TAB     |
+------------+--------+--------+--------+--------+-------------+
|   MO(1)    |   A    |   W    |   D    |   T    |  P or LSFT  |
+------------+--------+--------+--------+--------+-------------+
| X or MO(2) |  SPC   |   S    |  LCTL  |          F           |
+------------+--------+--------+--------+----------------------+
```

### Layer 1

A,B means pressing A followed by B, then releasing them simultaneously.

```
+------------+--------+--------+--------+--------+-------------+
|            | LALT,1 | LALT,2 | LALT,3 | LALT,4 |   LALT,5    |
+------------+--------+--------+--------+--------+-------------+
|            |   1    |   2    |   3    |   4    |      5      |
+------------+--------+--------+--------+--------+-------------+
|    THRU    |        |        |        |        |             |
+------------+--------+--------+--------+--------+-------------+
|            |        |        |        |       LALT,ENT       |
+------------+--------+--------+--------+----------------------+
```

### Layer 2

```
+------------+--------+--------+--------+--------+-------------+
| RGB mode R |  Mod+  |  Hue+  |  Sat+  |  Brt+  |    Eff+     |
+------------+--------+--------+--------+--------+-------------+
| RGB mode K |  Mod-  |  Hue-  |  Sat-  |  Brt-  |    Eff-     |
+------------+--------+--------+--------+--------+-------------+
|     F9     |   F1   |   F2   |   F3   |   F4   |     F5      |
+------------+--------+--------+--------+--------+-------------+
|    THRU    |   F6   |   F7   |   F8   |      RGB Toggle      |
+------------+--------+--------+--------+----------------------+
```

### Keymap definition

You can use the site [QMK Configurator](https://config.qmk.fm/#/amag23/LAYOUT)
with my [kitcnya.json](https://raw.githubusercontent.com/kitcnya/amag23/master/kitcnya.json) file.
